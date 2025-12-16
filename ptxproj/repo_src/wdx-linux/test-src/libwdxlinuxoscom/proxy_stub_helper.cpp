//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of helper functions for proxy/stub interaction.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "proxy_stub_helper.hpp"

#include <gtest/gtest.h>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::managed_object;
using wago::wdx::linuxos::com::data_input_stream;

using testing::Return;
using testing::Invoke;
using testing::AnyNumber;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

proxy_stub_helper::proxy_stub_helper(mock_driver    &driver,
                                     mock_sender    &sender,
                                     managed_object &proxy_object,
                                     managed_object &stub_object)
: driver_mock(driver)
, sender_mock(sender)
, proxy(proxy_object)
, stub(stub_object)
, msg_for_proxy(std::make_shared<message_data>())
, msg_for_stub(std::make_shared<message_data>())
{ }

proxy_stub_helper::roundtrip proxy_stub_helper::expect_roundtrip(bool is_async,
                                                                 int  number)
{
    return roundtrip_internal(is_async, number, number);
}

proxy_stub_helper::roundtrip proxy_stub_helper::expect_send_and_receive(bool is_async,
                                                                        int  number)
{
    return roundtrip_internal(is_async, number, 0);
}

proxy_stub_helper::roundtrip proxy_stub_helper::roundtrip_internal(bool is_async,
                                                                   int  proxy_stub_number,
                                                                   int  stub_proxy_number)
{
    roundtrip_run_helper();

    roundtrip_send_helper(proxy, msg_for_stub, proxy_stub_number);
    roundtrip_send_helper(stub, msg_for_proxy, stub_proxy_number);

    return [this, run_count=(is_async ? static_cast<size_t>((proxy_stub_number + stub_proxy_number)*2) : 0U)] {
        for(size_t i = 0; i < run_count; ++i)
        {
            driver_mock.run_once(0);
        }
    };
}

void proxy_stub_helper::roundtrip_run_helper()
{
    auto run_helper = [&](auto) {
        if (!msg_for_proxy->empty())
        {
            data_input_stream message_stream(*msg_for_proxy);
            proxy.handle_message(message_stream);
            msg_for_proxy->clear();
            return true;
        }
        else if (!msg_for_stub->empty())
        {
            data_input_stream message_stream(*msg_for_stub);
            stub.handle_message(message_stream);
            msg_for_stub->clear();
            return true;
        }
        return false;
    };

    EXPECT_CALL(driver_mock, run_once(testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Invoke([run_helper](auto timeout) { return run_helper(timeout); }));

    EXPECT_CALL(driver_mock, run())
        .Times(AnyNumber())
        .WillRepeatedly(Invoke([run_helper]() { return run_helper(0); }));
}

void proxy_stub_helper::roundtrip_send_helper(managed_object                &object,
                                              std::shared_ptr<message_data> &message_for_target,
                                              int                            number)
{
    if (number > 0)
    {
        EXPECT_CALL(sender_mock, send(testing::Ref(object), testing::_, testing::_))
            .Times(number)
            .WillRepeatedly(Invoke([&, message_for_target](auto const &,
                                    auto const &message,
                                    auto        sent_handler) {
                sent_handler("");
                message_for_target->insert(message_for_target->begin(), message.begin(), message.end());
            }));
    }
}


//---- End of source file ------------------------------------------------------
