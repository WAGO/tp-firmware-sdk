//------------------------------------------------------------------------------
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
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
///  \brief    Mock HTTP response.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_RESPONSE_HPP_
#define TEST_INC_MOCKS_MOCK_RESPONSE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/request_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::http::response_i;
using wago::wdx::wda::http::http_status_code;

using ::testing::Exactly;
using ::testing::AtLeast;
using ::testing::AtMost;
using ::testing::AnyNumber;
using ::testing::WithArgs;
using ::testing::Invoke;
using ::testing::Return;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_response : public response_i
{
public:
    MOCK_CONST_METHOD0(get_status_code, http_status_code());
    MOCK_CONST_METHOD0(get_response_header, std::map<std::string, std::string> const &());
    MOCK_CONST_METHOD0(get_content_type, std::string const &());
    MOCK_CONST_METHOD0(get_content_length, std::string const &());
    MOCK_CONST_METHOD0(has_content, bool());
    MOCK_CONST_METHOD0(get_content, std::string());

    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_status_code())
            .Times(0);
        EXPECT_CALL(*this, get_response_header())
            .Times(0);
        EXPECT_CALL(*this, get_content_type())
            .Times(0);
        EXPECT_CALL(*this, get_content_length())
            .Times(0);
        EXPECT_CALL(*this, has_content())
            .Times(0);
        EXPECT_CALL(*this, get_content())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_RESPONSE_HPP_
//---- End of source file ------------------------------------------------------
