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
///  \brief    Test managed object store implementation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/manager.hpp"

#include "mocks/mock_managed_object_store.hpp"
#include "mocks/mock_adapter.hpp"
#include "mocks/mock_managed_object.hpp"
#include "mocks/mock_driver.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::manager;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::message_data;
using wago::wdx::linuxos::com::managed_object_store_i;
using wago::wdx::linuxos::com::data_istream;
using wago::wdx::linuxos::com::data_input_stream;
using wago::wdx::linuxos::com::data_stream;
using wago::wdx::linuxos::com::backend_object_id;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class manager_fixture : public testing::Test
{
public:
    mock_adapter              &adapter_mock;
    mock_driver                driver_mock;
    mock_managed_object_store &store_mock;
    std::string                test_connection_name;
    
    manager the_manager;

    manager_fixture(std::unique_ptr<mock_adapter>              adapter,
                    std::unique_ptr<mock_managed_object_store> store)
    : adapter_mock(*adapter)
    , store_mock(*store)
    , test_connection_name("Test Connection")
    , the_manager(test_connection_name, std::move(adapter), driver_mock, std::move(store))
    { }

    manager_fixture()
    : manager_fixture(std::make_unique<mock_adapter>(), 
                      std::make_unique<mock_managed_object_store>())
    { }

    ~manager_fixture() override = default;

    void SetUp() override
    {
        adapter_mock.set_default_expectations();
        driver_mock.set_default_expectations();
        store_mock.set_default_expectations();
    }

    std::unique_ptr<mock_managed_object> create_mocked_object(managed_object_id object_id = 0)
    {
        return std::make_unique<mock_managed_object>(object_id, test_connection_name, the_manager, driver_mock);
    }
};

TEST_F(manager_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(manager_fixture, get_store)
{
    EXPECT_EQ(&store_mock, &the_manager.get_object_store());
}

TEST_F(manager_fixture, send)
{
    auto               test_sender  = create_mocked_object();
    message_data const test_message = { 0x01, 0x02, 0x03, 0x04};

    EXPECT_CALL(adapter_mock, send(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Invoke([test_sender_id = test_sender->get_id(), test_message](auto message, auto handler) {
            // must send the object id first, which is coded
            managed_object_id encoded_object_id;
            data_input_stream stream(message);
            wago::wdx::linuxos::com::coder::decode(stream, encoded_object_id);
            EXPECT_EQ(test_sender_id, encoded_object_id);
            
            // original message is available just after the id
            auto message_without_sender = message_data(message.begin() + sizeof(managed_object_id), message.end());
            EXPECT_EQ(message_without_sender, test_message);
            handler("");
        }));

    bool handler_called = false;
    the_manager.send(*test_sender, std::move(test_message), [&handler_called](std::string error) {
        EXPECT_TRUE(error.empty());
        handler_called = true;
    });
    EXPECT_TRUE(handler_called);
}

TEST_F(manager_fixture, receive)
{
    auto test_receiver    = create_mocked_object();
    managed_object &test_receiver_ref = *test_receiver.get();
    auto test_object_meta = std::make_shared<wago::wdx::linuxos::com::managed_object_store_i::object_meta>();

    message_data test_message  = { 0x01, 0x02, 0x03, 0x04};
    message_data whole_message;
    data_stream stream(whole_message);
    wago::wdx::linuxos::com::coder::encode(stream, test_receiver->get_id());
    wago::wdx::linuxos::com::coder::encode(stream, test_message);

    EXPECT_CALL(store_mock, get_object_meta(::testing::Eq(test_receiver->get_id())))
        .Times(1)
        .WillRepeatedly(Return(test_object_meta));

    EXPECT_CALL(store_mock, get_object(::testing::Eq(test_receiver->get_id())))
        .Times(1)
        .WillRepeatedly(Invoke([&test_receiver_ref](auto) {
            return ::testing::ByRef(test_receiver_ref);
        }));

    EXPECT_CALL(*test_receiver, handle_message(::testing::_))
        .Times(1)
        .WillRepeatedly(Invoke([test_message](auto &message_stream) {
            // must be called with the message body only. ID must have been removed
            // from stream by the manager already
            message_data received_message;
            wago::wdx::linuxos::com::coder::decode(message_stream, received_message);
            EXPECT_EQ(test_message, received_message);
        }));

    the_manager.receive(whole_message);
}

//---- End of source file ------------------------------------------------------
