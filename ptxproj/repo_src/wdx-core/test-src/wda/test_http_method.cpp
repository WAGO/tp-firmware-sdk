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
///  \brief    Test HTTP method related functions
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/http_method.hpp"

#include <gtest/gtest.h>

#include <tuple>
#include <string>

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
auto const supported_http_methods = ::testing::Values(
    std::make_tuple(http_method::get,     std::string("GET")),
    std::make_tuple(http_method::head,    std::string("HEAD")),
    std::make_tuple(http_method::post,    std::string("POST")),
    std::make_tuple(http_method::put,     std::string("PUT")),
    std::make_tuple(http_method::delete_, std::string("DELETE")),
    std::make_tuple(http_method::connect, std::string("CONNECT")),
    std::make_tuple(http_method::options, std::string("OPTIONS")),
    std::make_tuple(http_method::trace,   std::string("TRACE")),
    std::make_tuple(http_method::patch,   std::string("PATCH"))
);

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class http_method_fixture: public ::testing::TestWithParam<std::tuple<http_method,std::string>> 
{
protected:
    http_method method = std::get<0>(GetParam());
    std::string method_name = std::get<1>(GetParam());
};

TEST_P(http_method_fixture, get_method_name)
{
    EXPECT_EQ(get_method_name(method), method_name);
}

TEST_P(http_method_fixture, get_method_from_name)
{
    EXPECT_EQ(get_method_from_name(method_name.c_str()), method);
}

INSTANTIATE_TEST_CASE_P(supported_http_methods, http_method_fixture, supported_http_methods);

//---- End of source file ------------------------------------------------------
