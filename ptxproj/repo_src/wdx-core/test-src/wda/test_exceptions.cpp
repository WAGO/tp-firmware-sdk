//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
///  \brief    Test exception objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/exception.hpp"
#include "wago/wdx/wda/http/http_status_code.hpp"
#include "http/http_exception.hpp"
#include "rest/data_exception.hpp"
#include "rest/core_exception.hpp"
#include "status.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda;
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::rest;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char               const default_exception_text[] = "Test exception text";
static constexpr core_status_code   const default_core_status_code = core_status_code::internal_error;
static constexpr domain_status_code const default_domain_status_code = 42;
static           core_error         const default_core_error(default_core_status_code, "message", default_domain_status_code, "type", "id");
static           data_error         const default_data_error(default_core_status_code, "message", default_domain_status_code, "type", "id", 0);
//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(base_exception, construct_delete_with_message)
{
    exception e(default_exception_text);
    EXPECT_STREQ(default_exception_text, e.what());
}

TEST(base_exception, construct_delete_with_runtime_error)
{
    std::runtime_error base_e(default_exception_text);
    exception e(base_e);
    EXPECT_STREQ(default_exception_text, e.what());
}

TEST(base_exception, construct_delete_with_std_exception)
{
    std::exception base_e;
    exception e(base_e);
    EXPECT_STRNE(nullptr, e.what());
    EXPECT_LT(0, strlen(e.what()));
}

TEST(base_exception, derived_from_std_exception)
{
    try
    {
        throw exception(default_exception_text);
    }
    catch(std::exception const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than an exception";
    }
}

TEST(base_exception, derived_from_runtime_error)
{
    try
    {
        throw exception(default_exception_text);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than an runtime_error exception";
    }
}

TEST(base_exception, basic_values)
{
    char const test_message[] = "Base exception test message";

    exception testexception(test_message);
    EXPECT_STREQ(test_message, testexception.what());
}

TEST(http_exception, derived_from_runtime_error)
{
    try
    {
        throw http_exception(default_exception_text);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a runtime_error exception";
    }
}

TEST(http_exception, derived_from_base_exception)
{
    try
    {
        throw http_exception(default_exception_text);
    }
    catch(exception const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a libwda basic exception";
    }
}

TEST(http_exception, constructed_from_base_exception)
{
    char const test_message[]     = "Base exception test message";
    http_status_code const status = http_status_code::bad_request;

    http_exception testexception(exception(test_message), status);
    EXPECT_EQ(status, testexception.get_http_status_code());
    EXPECT_STRNE("", testexception.get_title().c_str());
    EXPECT_STREQ(test_message, testexception.what());
}

TEST(http_exception, web_specific_values)
{
    char const test_message[]     = "Web exception test message";
    http_status_code const status = http_status_code::bad_request;

    http_exception testexception(test_message, status);
    EXPECT_EQ(status, testexception.get_http_status_code());
    EXPECT_STRNE("", testexception.get_title().c_str());
    EXPECT_STREQ(test_message, testexception.what());
}

TEST(data_exception, derived_from_runtime_error)
{
    try
    {
        throw data_exception(default_exception_text, default_data_error);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a runtime_error exception";
    }
}

TEST(data_exception, derived_from_base_exception)
{
    try
    {
        throw data_exception(default_exception_text, default_data_error);
    }
    catch(exception const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a libwda basic exception";
    }
}

TEST(data_exception, derived_from_http_exception)
{
    try
    {
        throw data_exception(default_exception_text, default_data_error);
    }
    catch(http_exception const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a libwda http exception";
    }
}

TEST(data_exception, data_specific_values)
{
    char const test_message[]     = "Data exception test message";

    data_exception testexception(test_message, default_data_error);
    EXPECT_EQ(http_status_code::internal_server_error, testexception.get_http_status_code());
    EXPECT_STRNE("", testexception.get_title().c_str());
    EXPECT_STREQ(test_message, testexception.what());
    ASSERT_EQ(1, testexception.get_errors().size());
    EXPECT_NE(nullptr, testexception.get_errors().at(0).get());
    EXPECT_EQ(default_data_error.get_title(), testexception.get_errors().at(0)->get_title());
    EXPECT_EQ(default_data_error.get_message(), testexception.get_errors().at(0)->get_message());
    EXPECT_EQ(default_data_error.get_data_index(), testexception.get_errors().at(0)->get_data_index());
    EXPECT_EQ(default_data_error.get_domain_status_code(), testexception.get_errors().at(0)->get_domain_status_code());
}

TEST(data_exception, generate_404_http_error_code)
{
    char const test_message[]     = "Data exception test message";
    auto const error1 = std::make_shared<data_error>(core_status_code::unknown_parameter_path, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    auto const error2 = std::make_shared<data_error>(core_status_code::not_a_method, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    std::vector<std::shared_ptr<data_error>> const errors = { error1, error2 };

    data_exception testexception(test_message, errors);
    EXPECT_EQ(http_status_code::not_found, testexception.get_http_status_code());
    EXPECT_STRNE("", testexception.get_title().c_str());
    EXPECT_STREQ(test_message, testexception.what());
    ASSERT_EQ(errors.size(), testexception.get_errors().size());
}

TEST(data_exception, generate_4xx_http_error_code)
{
    char const test_message[]     = "Data exception test message";
    auto const error1 = std::make_shared<data_error>(core_status_code::unknown_monitoring_list, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    auto const error2 = std::make_shared<data_error>(core_status_code::missing_argument, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    std::vector<std::shared_ptr<data_error>> const errors = { error1, error2 };

    data_exception testexception(test_message, errors);
    EXPECT_EQ(http_status_code::bad_request, testexception.get_http_status_code());
    EXPECT_STRNE("", testexception.get_title().c_str());
    EXPECT_STREQ(test_message, testexception.what());
    ASSERT_EQ(errors.size(), testexception.get_errors().size());
}

TEST(data_exception, generate_5xx_http_error_code)
{
    char const test_message[]     = "Data exception test message";
    auto const error1 = std::make_shared<data_error>(core_status_code::success, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    auto const error2 = std::make_shared<data_error>(core_status_code::wrong_value_type, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    auto const error3 = std::make_shared<data_error>(core_status_code::no_error_yet, 
                                                     "message", default_domain_status_code, "type", "id", 0);
    std::vector<std::shared_ptr<data_error>> const errors = { error1, error2, error3 };

    data_exception testexception(test_message, errors);
    EXPECT_EQ(http_status_code::internal_server_error, testexception.get_http_status_code());
    EXPECT_STRNE("", testexception.get_title().c_str());
    EXPECT_STREQ(test_message, testexception.what());
    ASSERT_EQ(errors.size(), testexception.get_errors().size());
}

TEST(rest_core_exception, derived_from_runtime_error)
{
    try
    {
        throw core_exception(default_data_error);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_NE("", e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a runtime_error exception";
    }
}

TEST(rest_core_exception, derived_from_http_exception)
{
    try
    {
        throw core_exception(default_data_error);
    }
    catch(http_exception const &e)
    {
        EXPECT_STRNE("", e.get_title().c_str());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than a http_exception";
    }
}

TEST(rest_core_exception, specific_values)
{
    core_error test_error(default_core_error);
    core_exception testexception(test_error);
    EXPECT_EQ(default_core_status_code, testexception.get_core_status_code());
    EXPECT_EQ(test_error.get_http_status_code(), testexception.get_http_status_code());
    EXPECT_EQ(test_error.get_title(), testexception.get_title());
    EXPECT_GE(0, std::string(testexception.what()).find(test_error.get_message()));
}


//---- End of source file ------------------------------------------------------
