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
///  \brief    Test verify endpoint implementation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "verify.hpp"
#include "mocks/mock_oauth_token_handler.hpp"
#include "mocks/mock_request.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class process_verify_request_fixture: public ::testing::Test
{
protected:
    mock_request              request_mock;
    mock_oauth_token_handler  oauth_token_handler_mock;

public:

    void SetUp() override
    {
        request_mock.set_default_expectations();
        oauth_token_handler_mock.set_default_expectations();
    }
};

TEST_F(process_verify_request_fixture, for_valid_token)
{
    std::string                              const token  = "testToken123";
    oauth_token_handler_i::validation_result const result = { true, false, 42, "testclient", "testscope", "testname" };

    request_mock.expect_request_form_checked("token", token);
    EXPECT_CALL(oauth_token_handler_mock, validate_access_token(token))
                .Times(Exactly(1))
                .WillRepeatedly(Return(result));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::ok, response.get_status_code());
        }));

    process_verify_request(request_mock, oauth_token_handler_mock);
}

TEST_F(process_verify_request_fixture, for_invalid_token)
{
    std::string                              const token  = "testTokenABC";
    oauth_token_handler_i::validation_result const result;

    request_mock.expect_request_form_checked("token", token);
    EXPECT_CALL(oauth_token_handler_mock, validate_access_token(token))
                .Times(Exactly(1))
                .WillRepeatedly(Return(result));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::ok, response.get_status_code());
        }));

    process_verify_request(request_mock, oauth_token_handler_mock);
}
