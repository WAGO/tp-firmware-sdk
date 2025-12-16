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
///  \brief    Mock for authenticator class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_AUTHENTICATOR_HPP_
#define TEST_INC_MOCKS_MOCK_AUTHENTICATOR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticator_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::authenticator_i;
using wago::authserv::auth_result;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_authenticator: public authenticator_i
{
  public:
    MOCK_METHOD0(admin_has_no_password, bool());
    MOCK_METHOD1(has_form_auth_data, bool(wago::authserv::http::request_i const &request));
    MOCK_METHOD1(has_form_password_change_data, bool(wago::authserv::http::request_i const &request));
    MOCK_METHOD2(authenticate, auth_result(wago::authserv::http::request_i const &request,
                                           std::vector<std::string>        const &scopes));
    MOCK_METHOD1(password_change, wago::future<void>(wago::authserv::http::request_i const &request));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, admin_has_no_password())
            .Times(0);
        EXPECT_CALL(*this, has_form_auth_data(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, has_form_password_change_data(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, authenticate(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, password_change(::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_AUTHENTICATOR_HPP_
//---- End of source file ------------------------------------------------------
