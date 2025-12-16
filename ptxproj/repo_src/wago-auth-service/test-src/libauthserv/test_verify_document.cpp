//------------------------------------------------------------------------------
// Copyright (c) 2023 WAGO GmbH & Co. KG
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
///  \brief    Test token verification response object.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "verify_document.hpp"
#include "wago/authserv/http/http_status_code.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

TEST(verify_document, for_valid_token)
{
    std::string test_user   = "testuser";
    std::string test_scopes = "abc def";
    uint32_t    test_expire = 123456789;
    
    {
        verify_document doc(test_user, test_scopes, test_expire);
        
        EXPECT_EQ(http::http_status_code::ok,                 doc.get_status_code());
        EXPECT_EQ(http::headermap({ /* empty! */}),           doc.get_response_header());
        EXPECT_EQ("application/json;charset=UTF-8",           doc.get_content_type());
        EXPECT_EQ(std::to_string(doc.get_content().length()), doc.get_content_length());

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"active\":true"
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"username\":\"" + test_user + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"scope\":\"" + test_scopes + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"expires_in\":" + std::to_string(test_expire)
        ));
    }
}

TEST(verify_document, for_invalid_token)
{
    {
        verify_document doc;
        
        EXPECT_EQ(http::http_status_code::ok,                 doc.get_status_code());
        EXPECT_EQ(http::headermap({ /* empty! */}),           doc.get_response_header());
        EXPECT_EQ("application/json;charset=UTF-8",           doc.get_content_type());
        EXPECT_EQ(std::to_string(doc.get_content().length()), doc.get_content_length());

        EXPECT_THAT(doc.get_content(), ::testing::Eq(
            "{\"active\":false}"
        ));
    }
}
