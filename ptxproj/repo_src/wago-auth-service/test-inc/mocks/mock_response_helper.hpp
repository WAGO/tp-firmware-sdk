//------------------------------------------------------------------------------
// Copyright (c) 2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for oauth html templater.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_response_helper_HPP_
#define TEST_INC_MOCKS_MOCK_response_helper_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "response_helper_i.hpp"

#include <wc/compiler.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_response_helper: public response_helper_i
{
  public:
    MOCK_CONST_METHOD5(send_password_setup_page, void(http::request_i                     &request,
                                                      std::string                   const &cancel_uri,
                                                      http::http_status_code               code,
                                                      response_helper_i::error_type const &error_type,
                                                      std::string                   const &additional_error_message));
    MOCK_CONST_METHOD7(send_login_page, void(http::request_i                     &request,
                                             std::string                   const &cancel_uri,
                                             http::http_status_code               code,
                                             std::string                   const &accessing_client_label,
                                             response_helper_i::error_type const &error_type,
                                             std::string                   const &additional_error_message,
                                             std::string                   const &success_message));
    MOCK_CONST_METHOD7(send_password_change_page, void(http::request_i                     &request,
                                                       std::string                   const &cancel_uri,
                                                       http::http_status_code               code,
                                                       std::string                   const &user_name,
                                                       response_helper_i::error_type const &error_type,
                                                       std::string                   const &additional_error_message,
                                                       std::string                   const &info_message));
    MOCK_CONST_METHOD4(send_success_confirmation, void(http::request_i              &request,
                                                       std::string            const &continue_uri,
                                                       http::http_status_code        code,
                                                       std::string            const &success_message));
    MOCK_CONST_METHOD5(send_error_confirmation, void(http::request_i                     &request,
                                                     std::string                   const &continue_uri,
                                                     http::http_status_code               code,
                                                     response_helper_i::error_type const &error_type,
                                                     std::string                   const &additional_error_message));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, send_password_setup_page(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, send_login_page(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, send_password_change_page(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, send_success_confirmation(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, send_error_confirmation(::testing::_, ::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_response_helper_HPP_
//---- End of source file ------------------------------------------------------
