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
///  \brief    Test Boost ASIO adapter implementation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio/asio_adapter.hpp"

#include <gtest/gtest.h>

#include <chrono>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::asio_adapter;
using wago::wdx::linuxos::com::message_data;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(asio_adapter, construct_delete)
{
    boost::asio::io_context context;
    asio_adapter adapter(context);
}

TEST(asio_adapter, close)
{
    boost::asio::io_context context;
    asio_adapter adapter(context);

    bool close_handler_called = false;
    EXPECT_FALSE(adapter.is_closed());
    adapter.close([&close_handler_called](){ close_handler_called = true; });
    EXPECT_TRUE(adapter.is_closed());
    EXPECT_TRUE(close_handler_called);
}

TEST(asio_adapter, close_with_nonempty_queue)
{
    boost::asio::io_context context;
    asio_adapter adapter(context);

    bool close_handler_called = false;

    // Prepare message for queue
    message_data message = { 0x01, 0x02 };
    adapter.send(std::move(message), [](std::string){});

    // Close adapter connection
    EXPECT_FALSE(adapter.is_closed());
    adapter.close([&close_handler_called](){ close_handler_called = true; });
    EXPECT_TRUE(adapter.is_closed());
    EXPECT_FALSE(close_handler_called);

    // Trigger message write operation (will fail but should result in an empty queue)
    while(context.poll_one());
    // An empty queue should trigger the final close handler
    EXPECT_TRUE(close_handler_called);
}

TEST(asio_adapter, reinit)
{
    boost::asio::io_context context;
    asio_adapter adapter(context);

    adapter.close([](){});
    EXPECT_TRUE(adapter.is_closed());
    adapter.reinit();
    EXPECT_FALSE(adapter.is_closed());
}

// Send and receive is tested with an adapter pair in context of ASIO server & client unit test


//---- End of source file ------------------------------------------------------
