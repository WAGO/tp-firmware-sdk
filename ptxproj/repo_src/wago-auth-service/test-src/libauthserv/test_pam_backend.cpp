//------------------------------------------------------------------------------
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
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
///  \brief    Test PAM authentication backend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "pam_backend.hpp"
#include "wago/authserv/exception.hpp"

#include "mocks/mock_pam.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::pam_backend;
using wago::authserv::exception;
using testing::Return;
using testing::AtLeast;
using testing::WithoutArgs;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class pam_backend_fixture : public ::testing::Test
{
protected:
    mock_pam pam_mock;

    pam_backend_fixture() = default;
    ~pam_backend_fixture() override = default;

    void SetUp() override
    {
        pam_mock.set_default_expectations();
    }
};

TEST_F(pam_backend_fixture, construct_delete)
{
    pam_backend backend("test_service_name");
}

TEST_F(pam_backend_fixture, authenticate_succeeds)
{
    char const test_service[]  = "testservice";
    char const test_user[]     = "testuser";
    char const test_password[] = "testpassword";

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));;
    EXPECT_CALL(pam_mock, acct_mgmt(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}

TEST_F(pam_backend_fixture, authenticate_succeeds_with_expired_password)
{
    char const test_service[]  = "testservice";
    char const test_user[]     = "testuser";
    char const test_password[] = "testpassword";

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));;
    EXPECT_CALL(pam_mock, acct_mgmt(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_NEW_AUTHTOK_REQD));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}

TEST_F(pam_backend_fixture, authenticate_throws_when_pam_start_fails)
{
    char const test_service[]      = "testservice";
    char const test_user[]         = "testuser";
    char const test_password[]     = "testpassword";
    char const test_error[]        = "testerror";
    int        test_pam_error_code = 999;

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, start(::testing::StrEq(test_service), ::testing::StrEq(test_user), ::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(test_pam_error_code)); // any != PAM_SUCCESS should do
    EXPECT_CALL(pam_mock, strerror(::testing::_, test_pam_error_code))
        .Times(AnyNumber())
        .WillRepeatedly(Return(test_error));

    EXPECT_THROW({
        backend.authenticate(test_user, test_password);
    }, exception);
}

TEST_F(pam_backend_fixture, authenticate_succeeds_when_pam_end_fails)
{
    char const test_service[]      = "testservice";
    char const test_user[]         = "testuser";
    char const test_password[]     = "testpassword";
    char const test_error[]        = "testerror";
    int        test_pam_error_code = 999;

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, start(::testing::StrEq(test_service), ::testing::StrEq(test_user), ::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));
    EXPECT_CALL(pam_mock, end(::testing::_, ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_pam_error_code));
    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));;
    EXPECT_CALL(pam_mock, acct_mgmt(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));
    EXPECT_CALL(pam_mock, strerror(::testing::_, test_pam_error_code))
        .Times(AnyNumber())
        .WillRepeatedly(Return(test_error));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}

TEST_F(pam_backend_fixture, authenticate_fails_when_pam_authenticate_fails)
{
    char const test_service[]      = "testservice";
    char const test_user[]         = "testuser";
    char const test_password[]     = "testpassword";
    char const test_error[]        = "testerror";
    int        test_pam_error_code = 999;

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(test_pam_error_code));
    EXPECT_CALL(pam_mock, strerror(::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(test_error));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}

TEST_F(pam_backend_fixture, authenticate_fails_when_pam_acct_mgmt_fails)
{
    char const test_service[]      = "testservice";
    char const test_user[]         = "testuser";
    char const test_password[]     = "testpassword";
    char const test_error[]        = "testerror";
    int        test_pam_error_code = 999;

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));;
    EXPECT_CALL(pam_mock, acct_mgmt(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(test_pam_error_code));
    EXPECT_CALL(pam_mock, strerror(::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(test_error));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}

TEST_F(pam_backend_fixture, pam_conversation)
{
    char const test_service[]  = "testservice";
    char const test_user[]     = "testuser";
    char const test_password[] = "testpassword";

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(WithoutArgs(Invoke(&pam_mock, &mock_pam::pam_authenticate_conversation)));
    EXPECT_CALL(pam_mock, acct_mgmt(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(PAM_SUCCESS));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}

TEST_F(pam_backend_fixture, pam_conversation_error)
{
    char const test_service[]  = "testservice";
    char const test_user[]     = "testuser";
    char const test_password[] = "testpassword";
    char const test_error[]    = "testerror";

    pam_backend backend(test_service);

    EXPECT_CALL(pam_mock, authenticate(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(WithoutArgs(Invoke(&pam_mock, &mock_pam::pam_authenticate_conversation_error)));
    EXPECT_CALL(pam_mock, strerror(::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(test_error));

    auto const result = backend.authenticate(test_user, test_password);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_EQ(result.user_name, test_user);
}


//---- End of source file ------------------------------------------------------
