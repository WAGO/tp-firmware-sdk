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
///  \brief    Test HTTP status code related functions
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/http_status_code.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::http;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
auto const supported_http_status_codes = ::testing::Values(
    http_status_code::continue_,
    http_status_code::switching_protocols,
    http_status_code::ok,
    http_status_code::created,
    http_status_code::accepted,
    http_status_code::non_authorative_information,
    http_status_code::no_content,
    http_status_code::reset_content,
    http_status_code::partial_content,
    http_status_code::multiple_choices,
    http_status_code::moved_permanently,
    http_status_code::found,
    http_status_code::see_other,
    http_status_code::not_modified,
    http_status_code::use_proxy,
    http_status_code::unused,
    http_status_code::temporary_redirect,
    http_status_code::bad_request,
    http_status_code::unauthorized,
    http_status_code::payment_required,
    http_status_code::forbidden,
    http_status_code::not_found,
    http_status_code::method_not_allowed,
    http_status_code::not_acceptable,
    http_status_code::proxy_authentication_required,
    http_status_code::request_timeout,
    http_status_code::conflict,
    http_status_code::gone,
    http_status_code::length_required,
    http_status_code::precondition_failed,
    http_status_code::payload_too_large,
    http_status_code::uri_too_long,
    http_status_code::unsupported_media_type,
    http_status_code::range_not_satisfiable,
    http_status_code::expectattion_failed,
    http_status_code::im_a_teapot,
    http_status_code::upgrade_required,
    http_status_code::internal_server_error,
    http_status_code::not_implemented,
    http_status_code::bad_gateway,
    http_status_code::service_unavailable,
    http_status_code::gateway_timeout,
    http_status_code::http_version_not_supported
);

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class http_status_code_fixture: public ::testing::TestWithParam<http_status_code>
{
protected:
    http_status_code code = GetParam();
};

TEST_P(http_status_code_fixture, get_http_status_code_text)
{
    auto code_text = get_http_status_code_text(code);
    EXPECT_NE(code_text, nullptr);
    EXPECT_GT(strlen(code_text), 0);
}

TEST_P(http_status_code_fixture, is_http_error)
{
    if ((int)code < 400) 
    {
        EXPECT_FALSE(is_http_error(code));
    }
    else
    {
        EXPECT_TRUE(is_http_error(code));
    }
}

INSTANTIATE_TEST_CASE_P(supported_http_status_codes, http_status_code_fixture, supported_http_status_codes);

//---- End of source file ------------------------------------------------------
