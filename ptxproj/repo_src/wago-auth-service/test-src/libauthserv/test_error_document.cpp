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
///  \brief    Test error response object.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "error_document.hpp"
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


TEST(error_document, with_http_status_type_description_and_uri)
{
    http::http_status_code test_http_status = http::http_status_code::im_a_teapot;
    std::string            test_type        = "a_type";
    std::string            test_description = "abc description 456";
    std::string            test_uri         = "/error/uri";
    
    {
        error_document doc(test_http_status, test_type, test_description, test_uri);
        
        EXPECT_EQ(test_http_status,                           doc.get_status_code());
        EXPECT_EQ(http::headermap({ /* empty! */}),           doc.get_response_header());
        EXPECT_EQ("application/json;charset=UTF-8",           doc.get_content_type());
        EXPECT_EQ(std::to_string(doc.get_content().length()), doc.get_content_length());

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"error\":\"" + test_type + "\""
        ));

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"error_description\":\"" + test_description + "\""
        ));

        EXPECT_THAT(doc.get_content(), ::testing::HasSubstr(
            "\"error_uri\":\"" + test_uri + "\""
        ));
    }
}
