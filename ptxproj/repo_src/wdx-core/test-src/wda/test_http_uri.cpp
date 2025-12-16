//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test HTTP URI related functions
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/uri.hpp"

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

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------


TEST(http_uri, construct_default_empty_uri)
{
    uri test_uri;

    EXPECT_EQ("", test_uri.get_path());
    EXPECT_EQ("", test_uri.get_query());
}

TEST(http_uri, construct_uri_complete)
{
    uri test_uri = "/test/path?test=query&parameters";

    EXPECT_EQ("/test/path",             test_uri.get_path());
    EXPECT_EQ("?test=query&parameters", test_uri.get_query());
}

TEST(http_uri, construct_uri_no_query)
{
    uri test_uri = "/test/path";

    EXPECT_EQ("/test/path",             test_uri.get_path());
    EXPECT_EQ("",                       test_uri.get_query());
}

TEST(http_uri, construct_uri_empty_query)
{
    uri test_uri = "/test/path?";

    EXPECT_EQ("/test/path",             test_uri.get_path());
    EXPECT_EQ("?",                      test_uri.get_query());
}

TEST(http_uri, construct_uri_no_path)
{
    uri test_uri = "?test=query&parameters";

    EXPECT_EQ("",                       test_uri.get_path());
    EXPECT_EQ("?test=query&parameters", test_uri.get_query());
}

TEST(http_uri, construct_uri_empty_path)
{
    uri test_uri = "/?test=query&parameters";

    EXPECT_EQ("/",                      test_uri.get_path());
    EXPECT_EQ("?test=query&parameters", test_uri.get_query());
}

TEST(http_uri, parse_query)
{
    auto parsed_query = uri::parse_query("test=query&parameters");
    EXPECT_EQ(2,       parsed_query.size());
    EXPECT_EQ("query", parsed_query.at("test"));
    EXPECT_EQ("",      parsed_query.at("parameters"));
}

TEST(http_uri, parse_query_with_questionmark)
{
    auto parsed_query = uri::parse_query("?test=query&parameters");
    EXPECT_EQ(2,       parsed_query.size());
    EXPECT_EQ("query", parsed_query.at("test"));
    EXPECT_EQ("",      parsed_query.at("parameters"));
}

TEST(http_uri, parse_query_empty)
{
    auto parsed_query = uri::parse_query("");
    EXPECT_EQ(0,       parsed_query.size());
}

TEST(http_uri, parse_query_keeps_encoding)
{
    auto parsed_query = uri::parse_query("test=%20query%3F%26&para%20meters");
    EXPECT_EQ(2,                parsed_query.size());
    EXPECT_EQ("%20query%3F%26", parsed_query.at("test"));
    EXPECT_EQ("",               parsed_query.at("para%20meters"));
}

TEST(http_uri, construct_uri_complete_with_fragment)
{
    uri test_uri = "/test/path?test=query&parameters#fragment=value";

    EXPECT_EQ("/test/path",             test_uri.get_path());
    EXPECT_EQ("?test=query&parameters", test_uri.get_query());
    EXPECT_EQ("/test/path?test=query&parameters", test_uri.as_string()); //expect fragment is removed
}

TEST(http_uri, construct_uri_no_query_with_fragment)
{
    uri test_uri = "/test/path#fragment=value";

    EXPECT_EQ("/test/path", test_uri.get_path());
    EXPECT_EQ("",           test_uri.get_query());
    EXPECT_EQ("/test/path", test_uri.as_string()); //expect fragment is removed
}

//---- End of source file ------------------------------------------------------
