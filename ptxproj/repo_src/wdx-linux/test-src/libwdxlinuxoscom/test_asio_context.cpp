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
///  \brief    Test Boost ASIO context implementation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio/asio_context.hpp"
#include "asio/asio_adapter.hpp"

#include <gtest/gtest.h>

#include <chrono>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::asio_context;
using wago::wdx::linuxos::com::driver_i;
using wago::wdx::linuxos::com::receiver_i;
using wago::wdx::linuxos::com::asio_adapter;
using wago::wdx::linuxos::com::message_data;

//------------------------------------------------------------------------------
// concrete test derivation
//------------------------------------------------------------------------------
class test_context : public asio_context, public receiver_i
{
private:
    class test_receiver : public receiver_i
    {
    private:
        receiver_i &context_receiver_m;

    public:
        test_receiver(receiver_i &context_receiver) noexcept : context_receiver_m(context_receiver) {}
        void receive(message_data message) override
        {
            context_receiver_m.receive(message);
        }
    };

public:
    std::shared_ptr<test_receiver> receiver;
    bool                           disconnected = false;
    message_data                   received_data;

public:
    test_context() : asio_context(std::make_shared<boost::asio::io_context>()), receiver(std::make_shared<test_receiver>(*this)) {}
    void receive_data(message_data data)
    {
        on_receive(data, "", std::make_shared<asio_adapter>(*context_m));
    }
    void receive_error()
    {
        on_receive(message_data(), "Test error", std::make_shared<asio_adapter>(*context_m));
    }

private:
    std::shared_ptr<receiver_i> get_receiver(std::shared_ptr<asio_adapter>) override
    {
        return receiver;
    }

    void on_disconnect(std::shared_ptr<asio_adapter>) override
    {
        disconnected = true;
    }

    void receive(message_data message) override
    {
        received_data = message;
    }
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(asio_context, construct_delete)
{
    test_context context;
}

TEST(asio_context, driver_run)
{
    test_context context;
    driver_i &driven_context = context;

    driven_context.stop();
    EXPECT_FALSE(driven_context.run());
}

TEST(asio_context, driver_run_once)
{
    test_context context;
    driver_i &driven_context = context;

    driven_context.stop();
    EXPECT_FALSE(driven_context.run_once());
}

TEST(asio_context, driver_run_once_with_timeout)
{
    uint32_t const test_timeout_ms = 100;
    test_context context;
    driver_i &driven_context = context;

    driven_context.stop();
    auto const before       = std::chrono::system_clock::now();
    EXPECT_FALSE(driven_context.run_once(test_timeout_ms));
    auto const after        = std::chrono::system_clock::now();
    auto const run_duration = after - before;
    EXPECT_LE(test_timeout_ms, std::chrono::duration_cast<std::chrono::milliseconds>(run_duration).count());
}

TEST(asio_context, receive_data)
{
    message_data test_data = { 0x11, 0x38, 0x25 };

    test_context context;
    context.receive_data(test_data);
    EXPECT_FALSE(context.disconnected);
    ASSERT_EQ(test_data.size(), context.received_data.size());
    for(size_t i = 0; i < test_data.size(); ++i)
    {
        EXPECT_EQ(test_data.at(i), context.received_data.at(i));
    }
}

TEST(asio_context, receive_error)
{
    test_context context;
    context.receive_error();
    EXPECT_TRUE(context.disconnected);
    EXPECT_EQ(0, context.received_data.size());
}


//---- End of source file ------------------------------------------------------
