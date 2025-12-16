//------------------------------------------------------------------------------
// Copyright (c) 2023-2024 WAGO GmbH & Co. KG
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
///  \brief    Mock HTTP request.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_REQUEST_HPP_
#define TEST_INC_MOCKS_MOCK_REQUEST_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/request_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::http::request_i;
using wago::authserv::http::response_i;
using wago::authserv::http::http_method;

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
class mock_request : public request_i
{
public:
//    MOCK_METHOD0(destructor, void());
//    virtual ~mock_request() { destructor(); } // Remove when no longer needed

    MOCK_CONST_METHOD0(is_https, bool(void));
    MOCK_CONST_METHOD0(is_localhost, bool(void));
    MOCK_CONST_METHOD0(get_method, http_method(void));
    MOCK_CONST_METHOD0(get_request_uri, wago::authserv::http::uri());
    MOCK_CONST_METHOD1(has_query_parameter, bool(std::string const & name));
    MOCK_CONST_METHOD2(get_query_parameter, std::string(std::string const & name, bool decode));
    MOCK_CONST_METHOD1(has_form_parameter, bool(std::string const & name));
    MOCK_CONST_METHOD2(get_form_parameter, std::string(std::string const & name, bool decode));
    MOCK_CONST_METHOD1(has_http_header, bool(std::string const &header_name));
    MOCK_CONST_METHOD1(get_http_header, std::string(std::string const &header_name));
    MOCK_CONST_METHOD0(get_accepted_types, std::string(void));
    MOCK_CONST_METHOD0(get_content_type, std::string(void));
    MOCK_CONST_METHOD0(get_content_length, std::string(void));
    MOCK_METHOD0(get_content, std::string(void));
    MOCK_METHOD0(get_content_stream, std::istream&(void));
    MOCK_METHOD2(add_response_header, void(std::string const &header_name,
                                           std::string const &header_value));
    // Workaround: gmock does not allow to mock 'noexcept' methods in the current version
    MOCK_METHOD1(respond_mock, void(response_i const &response));
    void respond(response_i const &response) noexcept override { respond_mock(response); }
 
    MOCK_CONST_METHOD0(is_responded, bool(void));
    MOCK_METHOD2(send_data, void(char const * data, size_t size));
    MOCK_METHOD0(finish, void(void));

    void set_default_expectations()
    {
//        EXPECT_CALL(*this, destructor())
//            .Times(AnyNumber());
        EXPECT_CALL(*this, is_https())
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*this, is_localhost())
            .Times(AnyNumber())
            .WillRepeatedly(Return(false));
        EXPECT_CALL(*this, get_method())
            .Times(AnyNumber())
            .WillRepeatedly(Return(http_method::get));
        EXPECT_CALL(*this, get_request_uri())
            .Times(0);
        EXPECT_CALL(*this, has_query_parameter(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_query_parameter(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, has_form_parameter(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_form_parameter(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, has_http_header(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_http_header(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_accepted_types())
            .Times(AnyNumber())
            .WillRepeatedly(Return("*/*"));
        EXPECT_CALL(*this, get_content_type())
            .Times(0);
        EXPECT_CALL(*this, get_content_length())
            .Times(0);
        EXPECT_CALL(*this, get_content())
            .Times(0);
        EXPECT_CALL(*this, get_content_stream())
            .Times(0);
        EXPECT_CALL(*this, add_response_header(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, respond_mock(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, is_responded())
            .Times(0);
        EXPECT_CALL(*this, send_data(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, finish())
            .Times(0);

    }

    void expect_request_query_checked(std::string key, std::string value, bool optional = false)
    {
        EXPECT_CALL(*this, has_query_parameter(key))
            .Times(AtLeast(optional ? 0 : 1))
            .WillRepeatedly(
                Return(!value.empty())
            );

        EXPECT_CALL(*this, get_query_parameter(key, ::testing::_))
            .Times(AtLeast((optional || value.empty()) ? 0 : 1))
            .WillRepeatedly(
                Return(value)
            );
    }

    void expect_request_form_checked(std::string key, std::string value, bool optional = false)
    {
        EXPECT_CALL(*this, has_form_parameter(key))
            .Times(AnyNumber())
            .WillRepeatedly(
                Return(!value.empty())
            );

        EXPECT_CALL(*this, get_form_parameter(key, ::testing::_))
            .Times(AtLeast((optional || value.empty()) ? 0 : 1))
            .WillRepeatedly(
                Return(value)
            );
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_REQUEST_HPP_
//---- End of source file ------------------------------------------------------
