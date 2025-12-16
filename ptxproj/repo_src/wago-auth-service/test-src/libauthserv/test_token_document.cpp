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
///  \brief    Test token response object.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "token_document.hpp"
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

TEST(token_document, with_access_token)
{
    std::string test_access_token          = "abc_my_test_token";
    uint32_t    test_access_token_lifetime = 123456789;
    
    {
        token_document doc(test_access_token, test_access_token_lifetime);

        EXPECT_EQ(http::http_status_code::ok,                 doc.get_status_code());
        EXPECT_EQ(std::string("no-store"),                    doc.get_response_header().at("Cache-Control"));
        EXPECT_EQ(std::string("no-cache"),                    doc.get_response_header().at("Pragma"));
        EXPECT_EQ("application/json;charset=UTF-8",           doc.get_content_type());
        EXPECT_EQ(std::to_string(doc.get_content().length()), doc.get_content_length());

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"access_token\":\"" + test_access_token + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"expires_in\":" + std::to_string(test_access_token_lifetime)
        ));
        EXPECT_THAT(doc.get_content(), ::testing::Not(::testing::HasSubstr(
            "\"refresh_token\""
        )));
        EXPECT_THAT(doc.get_content(), ::testing::Not(::testing::HasSubstr(
            "\"password_expired\""
        )));
    }
}

TEST(token_document, with_access_token_and_refresh_token)
{
    std::string test_access_token          = "abc_my_test_token";
    std::string test_refresh_token         = "xyz_my_refresh_token";
    uint32_t    test_access_token_lifetime = 123456789;
    
    {
        token_document doc(test_access_token, test_refresh_token, test_access_token_lifetime);
        
        EXPECT_EQ(http::http_status_code::ok,                 doc.get_status_code());
        EXPECT_EQ(std::string("no-store"),                    doc.get_response_header().at("Cache-Control"));
        EXPECT_EQ(std::string("no-cache"),                    doc.get_response_header().at("Pragma"));
        EXPECT_EQ("application/json;charset=UTF-8",           doc.get_content_type());
        EXPECT_EQ(std::to_string(doc.get_content().length()), doc.get_content_length());

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"access_token\":\"" + test_access_token + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"expires_in\":" + std::to_string(test_access_token_lifetime)
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"refresh_token\":\"" + test_refresh_token + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::Not(::testing::HasSubstr(
            "\"password_expired\""
        )));
    }
}


TEST(token_document, with_access_token_and_refresh_token_and_password_expired)
{
    std::string test_access_token          = "abc_my_test_token";
    std::string test_refresh_token         = "xyz_my_refresh_token";
    uint32_t    test_access_token_lifetime = 123456789;
    bool        test_password_expired      = true;
    
    {
        token_document doc(test_access_token, test_refresh_token, test_access_token_lifetime, test_password_expired);
        
        EXPECT_EQ(http::http_status_code::ok,                 doc.get_status_code());
        EXPECT_EQ(std::string("no-store"),                    doc.get_response_header().at("Cache-Control"));
        EXPECT_EQ(std::string("no-cache"),                    doc.get_response_header().at("Pragma"));
        EXPECT_EQ("application/json;charset=UTF-8",           doc.get_content_type());
        EXPECT_EQ(std::to_string(doc.get_content().length()), doc.get_content_length());

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"access_token\":\"" + test_access_token + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"expires_in\":" + std::to_string(test_access_token_lifetime)
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"refresh_token\":\"" + test_refresh_token + "\""
        ));
        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"password_expired\":" + std::string(test_password_expired ? "true" : "false")
        ));
    }
}
