//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    Test exception class and helper functions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/exception.hpp"

#include <wago/wdx/responses.hpp>
#include <wago/wdx/test/wda_check.hpp>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// test definitions
//------------------------------------------------------------------------------
TEST(exception, construct_delete)
{
    auto        const test_code    = wago::wdx::status_codes::file_id_mismatch;
    std::string const test_message = "Test error message";

    wago::wdx::linuxos::file::exception test_exception(test_code, test_message);
    EXPECT_WDA_STATUS(test_code,    test_exception.get_status_code());
    EXPECT_EQ(        test_message, test_exception.what());
}

TEST(exception, derived_from_runtime_error)
{
    auto        const test_code    = wago::wdx::status_codes::internal_error;
    std::string const test_message = "Test error message for runtime error";

    try
    {
        throw wago::wdx::linuxos::file::exception(test_code, test_message);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_EQ(test_message, e.what());
    }
    catch(...)
    {
        FAIL() << "Exception should be derived from std::runtime_error";
    }
}

TEST(exception, construct_from_ptr)
{
    auto        const test_code    = wago::wdx::status_codes::logic_error;
    std::string const test_message = "Another test error message";

    wago::wdx::linuxos::file::exception test_exception(test_code, test_message);
    auto const e_ptr = std::make_exception_ptr(test_exception);
    wago::wdx::linuxos::file::exception const new_exception = wago::wdx::linuxos::file::get_exception_from_ptr(e_ptr);
    EXPECT_WDA_STATUS(test_code,    new_exception.get_status_code());
    EXPECT_EQ(        test_message, new_exception.what());
}

TEST(exception, construct_from_std_exception_ptr)
{
    std::exception test_exception;
    auto const e_ptr = std::make_exception_ptr(test_exception);
    wago::wdx::linuxos::file::exception const new_exception = wago::wdx::linuxos::file::get_exception_from_ptr(e_ptr);
    EXPECT_WDA_STATUS(wago::wdx::status_codes::internal_error, new_exception.get_status_code());
    EXPECT_NE(        std::string(""), new_exception.what());
}

TEST(exception, exception_handler)
{
    auto        const test_code    = wago::wdx::status_codes::logic_error;
    std::string const test_message = "Yet another test error message";
    std::string const test_prefix  = "Message prefix: ";

    auto response_promise = std::make_shared<wago::promise<wago::wdx::value_response>>();
    auto handler          = wago::wdx::linuxos::file::create_exception_handler(response_promise, test_prefix);
    auto response_future  = response_promise->get_future();

    wago::wdx::linuxos::file::exception test_exception(test_code, test_message);
    auto const e_ptr = std::make_exception_ptr(test_exception);

    handler(e_ptr);
    ASSERT_TRUE(response_future.valid());
    ASSERT_TRUE(response_future.ready());
    ASSERT_FUTURE_VALUE(response_future);
    wago::wdx::value_response const response = response_future.get();
    EXPECT_WDA_STATUS(test_code,                  response.status);
    EXPECT_EQ(        test_prefix + test_message, response.message);
}


//---- End of source file ------------------------------------------------------
