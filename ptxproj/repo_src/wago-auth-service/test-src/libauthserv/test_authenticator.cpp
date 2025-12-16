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
///  \brief    Test authenticator.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticator.hpp"
#include "oauth_error.hpp"
#include "http/http_exception.hpp"

#include "mocks/mock_password_backend.hpp"
#include "mocks/mock_password_change_backend.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_user_management.hpp"

#include <memory>

#include <wc/structuring.h>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
using wago::authserv::http::response_i;
using wago::authserv::http::http_status_code;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define TEST_SERVICE_BASE "/service/base"

using scopevector = std::vector<std::string>;
using groupvector = std::vector<std::string>;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class authenticator_fixture: public ::testing::Test
{
public:
    std::unique_ptr<authenticator>                auth;
    std::shared_ptr<mock_password_backend>        password_backend_mock;
    std::shared_ptr<mock_password_change_backend> password_change_backend_mock;
    std::unique_ptr<mock_user_management>         user_management_mock;

    std::string const default_user              = "username";
    std::string const default_password          = "secretpassword";
    std::string const default_auth_header_value = "Basic dXNlcm5hbWU6c2VjcmV0cGFzc3dvcmQ="; // Base64(username:secretpassword)
    scopevector const default_scopes            = { "some", "scopes", "with:colon" };
    groupvector const default_groups            = { "username", "scopes", "othergroup", "some", "with_colon" };
    groupvector const incomplete_groups         = { default_groups[0], default_groups[1], default_groups[2], default_groups[4] };
    size_t      const default_group_count       = WC_ARRAY_LENGTH(default_groups);
    group       const default_group_11          = { const_cast<char*>(default_groups[0].c_str()), nullptr, 11, nullptr };
    group       const default_group_12          = { const_cast<char*>(default_groups[1].c_str()), nullptr, 12, nullptr };
    group       const default_group_13          = { const_cast<char*>(default_groups[2].c_str()), nullptr, 13, nullptr };
    group       const default_group_14          = { const_cast<char*>(default_groups[3].c_str()), nullptr, 14, nullptr };
    group       const default_group_15          = { const_cast<char*>(default_groups[4].c_str()), nullptr, 15, nullptr };

    authenticator_fixture()
    { }

    void expect_form_auth(mock_request       &test_request,
                          std::string  const  user,
                          std::string  const  password)
    {
        EXPECT_CALL(test_request, has_http_header(::testing::StrCaseEq("Authorization")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(false));

        EXPECT_CALL(test_request, get_http_header(::testing::StrCaseEq("Authorization")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(""));

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("username")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("username"), true))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(user));

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("password")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("password"), true))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(password));
    }

    void expect_basic_auth(mock_request       &test_request,
                           std::string  const  auth_header_value)
    {
        EXPECT_CALL(test_request, has_http_header(::testing::StrCaseEq("Authorization")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(test_request, get_http_header(::testing::StrCaseEq("Authorization")))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(auth_header_value));

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("username")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(false));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("username"), true))
            .Times(AnyNumber())
            .WillRepeatedly(Return(""));

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("password")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(false));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("password"), true))
            .Times(AnyNumber())
            .WillRepeatedly(Return(""));
    }

    void expect_password_change(mock_request       &test_request,
                                std::string  const  user,
                                std::string  const  password,
                                std::string  const  newPassword)
    {

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("username")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("username"), true))
            .Times(AnyNumber())
            .WillRepeatedly(Return(user));

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("password")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("password"), true))
            .Times(AnyNumber())
            .WillRepeatedly(Return(password));

        EXPECT_CALL(test_request, has_form_parameter(::testing::StrCaseEq("new_password")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));

        EXPECT_CALL(test_request, get_form_parameter(::testing::StrCaseEq("new_password"), true))
            .Times(AnyNumber())
            .WillRepeatedly(Return(newPassword));

    }

    void SetUp() override
    {
        password_backend_mock        = std::make_shared<mock_password_backend>();
        password_change_backend_mock = std::make_shared<mock_password_change_backend>();
        user_management_mock         = std::make_unique<mock_user_management>();
        auth                         = std::make_unique<authenticator>(password_backend_mock, password_change_backend_mock, TEST_SERVICE_BASE);

        password_backend_mock->set_default_expectations();
        user_management_mock->set_default_expectations();

        EXPECT_CALL(*user_management_mock, getgroupname(::testing::Ge(11)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(default_group_11.gr_name));

        EXPECT_CALL(*user_management_mock, getgroupname(::testing::Ge(12)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(default_group_12.gr_name));

        EXPECT_CALL(*user_management_mock, getgroupname(::testing::Ge(13)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(default_group_13.gr_name));

        EXPECT_CALL(*user_management_mock, getgroupname(::testing::Ge(14)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(default_group_14.gr_name));

        EXPECT_CALL(*user_management_mock, getgroupname(::testing::Ge(15)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(default_group_15.gr_name));
    }
};

TEST_F(authenticator_fixture, construct_delete)
{
    // Nothing to do, everything is done in SetUp/TearDown
}

TEST_F(authenticator_fixture, no_auth_data)
{
    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("username")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("username"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("password")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("password"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_FALSE( auth->has_form_auth_data(*test_request));
    EXPECT_THROW({
        try {
            auth->authenticate( *test_request, default_scopes);
        }
        catch( const http::http_exception &e)
        {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("Invalid authentication data"));
            throw;
        }
    }, http::http_exception);
}

TEST_F(authenticator_fixture, legacy_new_password_param)
{
    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("username")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("username"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return("admin"));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("password")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("password"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return("wago"));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("new_password")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("new-password")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("new-password"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return("wago"));

    EXPECT_TRUE( auth->has_form_password_change_data(*test_request));
    auth->password_change(*test_request);
}

// Form authentication
//#####################

TEST_F(authenticator_fixture, successful_form_auth)
{
    // Test setup
    std::string const user = "testuser";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_form_auth(*test_request, default_user, default_password);

    EXPECT_CALL(*user_management_mock, get_groups(::testing::StrEq(default_user)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(default_groups));

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(default_user), ::testing::StrEq(default_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { true, false, false, user }));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(  auth->has_form_auth_data(*test_request));
    auto result = auth->authenticate( *test_request, default_scopes);
    EXPECT_TRUE( result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_FALSE(result.unauthorized);
}

TEST_F(authenticator_fixture, successful_expired_form_auth)
{
    // Test setup
    std::string const user = "testuser";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_form_auth(*test_request, default_user, default_password);

    EXPECT_CALL(*user_management_mock, get_groups(::testing::StrEq(default_user)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(default_groups));

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(default_user), ::testing::StrEq(default_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { true, true, false, user }));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(  auth->has_form_auth_data(*test_request));
    auto result = auth->authenticate( *test_request, default_scopes);
    EXPECT_TRUE( result.success);
    EXPECT_TRUE( result.expired);
    EXPECT_FALSE(result.unauthorized);
}

TEST_F(authenticator_fixture, successful_unauthorized_form_auth)
{
    // Test setup
    std::string const user = "testuser";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_form_auth(*test_request, default_user, default_password);

    EXPECT_CALL(*user_management_mock, get_groups(::testing::StrEq(default_user)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(incomplete_groups));

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(default_user), ::testing::StrEq(default_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { true, false, false, user }));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(  auth->has_form_auth_data(*test_request));
    auto result = auth->authenticate( *test_request, default_scopes);
    EXPECT_TRUE( result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.unauthorized);
}

TEST_F(authenticator_fixture, failed_form_auth)
{
    // Test setup
    std::string const user           = "testuser";
    std::string const wrong_password = "wrongpassword";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_form_auth(*test_request, default_user, wrong_password);

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(default_user), ::testing::StrEq(wrong_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { false, false, false, user }));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(  auth->has_form_auth_data(*test_request));
    auto result = auth->authenticate( *test_request, default_scopes);
    EXPECT_FALSE(result.success);
}

TEST_F(authenticator_fixture, no_password_change_data)
{
    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("username")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("username"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("password")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("password"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    EXPECT_CALL(*test_request, has_form_parameter(::testing::StrCaseEq("new_password")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(*test_request, get_form_parameter(::testing::StrCaseEq("new_password"), true))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_FALSE(auth->has_form_password_change_data(*test_request));

    EXPECT_THROW({ auth->password_change(*test_request); }, std::runtime_error);
}

TEST_F(authenticator_fixture, successful_password_change)
{
    // Test setup
    std::string const user          = "testuser";
    std::string const new_password  = "changedPassword";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_password_change(*test_request, default_user, default_password, new_password);

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_change_backend_mock, change_password(
            ::testing::StrEq(default_user),
            ::testing::StrEq(default_password),
            ::testing::StrEq(new_password)
        ))
        .Times(Exactly(1));

    // Should not respond the request -> done in other class
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(auth->has_form_auth_data(*test_request));

    auth->password_change(*test_request);
}

TEST_F(authenticator_fixture, successful_password_change_with_special_characters)
{
    // Test setup
    std::string const user          = "testuser";
    std::string const new_password  = "!\"_%&/()'";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_password_change(*test_request, default_user, default_password, new_password);

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_change_backend_mock, change_password(
            ::testing::StrEq(default_user),
            ::testing::StrEq(default_password),
            ::testing::StrEq(new_password)
        ))
        .Times(Exactly(1));

    // Should not respond the request -> done in other class
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(auth->has_form_auth_data(*test_request));

    auth->password_change(*test_request);
}

TEST_F(authenticator_fixture, error_on_password_change)
{
    // Test setup
    std::string const user         = "testuser";
    std::string const new_password = "changedPassword";
    std::string const custom_error = "an error occured";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    expect_password_change(*test_request, default_user, default_password, new_password);

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_change_backend_mock, change_password(
            ::testing::StrEq(default_user),
            ::testing::StrEq(default_password),
            ::testing::StrEq(new_password)
        ))
        .Times(Exactly(1))
        .WillRepeatedly(testing::Throw(std::runtime_error(custom_error)));

    // Should not respond the request -> done in other class
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    EXPECT_TRUE(auth->has_form_auth_data(*test_request));

    try
    {
        auth->password_change(*test_request);
        FAIL();
    }
    catch (std::runtime_error &e)
    {
        EXPECT_THAT(e.what(), ::testing::HasSubstr(custom_error));
    }
    catch (...)
    {
        FAIL();
    }
}

#ifdef ENFORCE_SET_ADMIN_PASSWORD
TEST_F(authenticator_fixture, admin_has_no_password)
{
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq("admin"), ::testing::StrEq("")))
        .Times(2) // true has to be checked again each time
        .WillRepeatedly(
            Return(auth_result{true, false, false, "admin"})
        );

    EXPECT_TRUE(auth->admin_has_no_password());
    EXPECT_TRUE(auth->admin_has_no_password());
}

TEST_F(authenticator_fixture, admin_has_a_password)
{
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq("admin"), ::testing::StrEq("")))
        .Times(1) // only one call as false can be cached
        .WillRepeatedly(
            Return(auth_result{false, false, false, ""})
        );

    EXPECT_FALSE(auth->admin_has_no_password());
    EXPECT_FALSE(auth->admin_has_no_password());
}
#endif

//---- End of source file ------------------------------------------------------
