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
///  \brief    Test token endpoint implementation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticator_i.hpp"
#include "token.hpp"

#include "mocks/mock_authenticator.hpp"
#include "mocks/mock_oauth_token_handler.hpp"
#include "mocks/mock_user_management.hpp"
#include "mocks/mock_password_backend.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_settings_store.hpp"

#include "wago/authserv/http/http_method.hpp"
#include "wago/authserv/http/http_status_code.hpp"
#include "wago/authserv/http/response_i.hpp"
#include "wago/authserv/oauth_token_handler_i.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
using testing::Ref;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

class process_token_request_fixture: public ::testing::Test
{
protected:
    mock_request              request_mock;
    mock_settings_store       settings_store_mock;
    mock_authenticator        authenticator_mock;
    mock_oauth_token_handler  oauth_token_handler_mock;

public:

    void SetUp() override
    {
        request_mock.set_default_expectations();
        settings_store_mock.set_default_expectations();
        authenticator_mock.set_default_expectations();
        oauth_token_handler_mock.set_default_expectations();
    }

    void expect_client_check(settings_store_i::oauth_client &client) {
        EXPECT_CALL(settings_store_mock, client_exists(client.id))
            .Times(AtLeast(1))
            .WillRepeatedly(
                Return(true)
            );
        EXPECT_CALL(settings_store_mock, get_client(client.id))
            .Times(AtLeast(1))
            .WillRepeatedly(
                testing::ReturnRef(client)
            );
    }
};

TEST_F(process_token_request_fixture, get_token_by_authorization_code)
{
    std::string test_client_id =                              "testclient";
    request_mock.expect_request_form_checked("grant_type",    "authorization_code");
    request_mock.expect_request_form_checked("client_id",     test_client_id);
    request_mock.expect_request_form_checked("code",          "testcode");
    request_mock.expect_request_form_checked("code_verifier", "testcode_verifier");

    std::string test_empty;
    std::vector<std::string> test_setting_grant_type_value {
        "some_type",
        "authorization_code",
        "some_other"
    };
    settings_store_i::oauth_client test_setting_client({
        .id=test_client_id,
        .label=test_empty,
        .redirect_uri=test_empty,
        .grant_types=test_setting_grant_type_value
    });
    expect_client_check(test_setting_client);
    std::string test_access_token_lifetime  = "123";
    std::string test_refresh_token_lifetime = "1234";
    settings_store_mock.expect_global_setting_read(settings_store_i::access_token_lifetime, test_access_token_lifetime);
    settings_store_mock.expect_global_setting_read(settings_store_i::refresh_token_lifetime, test_refresh_token_lifetime);

    EXPECT_CALL(oauth_token_handler_mock, validate_auth_code("testcode", "testcode_verifier"))
        .Times(Exactly(1))
        .WillOnce(Return(oauth_token_handler_i::validation_result {
                true,
                false,
                1,
                "testclient",
                "testscope",
                "testuser"
            }));
    
    EXPECT_CALL(oauth_token_handler_mock, generate_access_token(::testing::_, "testclient", "testscope", "testuser"))
        .Times(Exactly(1))
        .WillOnce(Return("testaccesstoken"));

    EXPECT_CALL(request_mock, is_localhost())
        .Times(Exactly(1))
        .WillOnce(Return(true));

    EXPECT_CALL(oauth_token_handler_mock, generate_refresh_token(::testing::_, "testclient", "testscope", "testuser"))
        .Times(Exactly(1))
        .WillOnce(Return("testrefreshtoken"));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::ok, response.get_status_code());
        }));

    // test execution
    {
        wago::authserv::process_token_request(
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_token_request_fixture, get_token_by_password)
{
    std::string test_client_id =                           "testclient";
    request_mock.expect_request_form_checked("grant_type", "password");
    request_mock.expect_request_form_checked("client_id",  test_client_id);
    request_mock.expect_request_form_checked("scope",      "testserver:testscope");
    request_mock.expect_request_form_checked("username",   "testuser", true);
    request_mock.expect_request_form_checked("password",   "testpassword", true);
    

    std::vector<std::string> test_setting_grant_type_value {
        "some_type",
        "password",
        "some_other"
    };
    std::vector<std::string> test_setting_scopes {
        "some_other_scope",
        "testserver:testscope",
        "yet_another_scope"
    };
    std::string test_empty;
    settings_store_i::oauth_client test_setting_client({
        .id=test_client_id,
        .label=test_empty,
        .redirect_uri=test_empty,
        .grant_types=test_setting_grant_type_value
    });
    expect_client_check(test_setting_client);
    EXPECT_CALL(settings_store_mock, get_all_scopes())
        .Times(AtLeast(1))
        .WillRepeatedly(
            testing::ReturnRef(test_setting_scopes)
        );
    std::string test_access_token_lifetime        = "123";
    std::string test_refresh_token_lifetime       = "1234";
    settings_store_mock.expect_global_setting_read(settings_store_i::access_token_lifetime, test_access_token_lifetime);
    settings_store_mock.expect_global_setting_read(settings_store_i::refresh_token_lifetime, test_refresh_token_lifetime);

    EXPECT_CALL(authenticator_mock, has_form_auth_data(Ref(request_mock)))
        .Times(AnyNumber())
        .WillRepeatedly(
            Return(true)
        );

    EXPECT_CALL(authenticator_mock, authenticate(Ref(request_mock), std::vector<std::string> { "testserver:testscope" }))
        .Times(Exactly(1))
        .WillRepeatedly(
            Return( auth_result {true, false, false, "testuser"})
        );

    EXPECT_CALL(authenticator_mock, admin_has_no_password())
        .Times(AtLeast(1))
        .WillRepeatedly(
            Return(false)
        );

    EXPECT_CALL(oauth_token_handler_mock, generate_access_token(::testing::_, "testclient", "testserver:testscope", "testuser"))
        .Times(Exactly(1))
        .WillOnce(Return("testaccesstoken"));

    EXPECT_CALL(request_mock, is_localhost())
        .Times(Exactly(1))
        .WillOnce(Return(true));

    EXPECT_CALL(oauth_token_handler_mock, generate_refresh_token(::testing::_, "testclient", "testserver:testscope", "testuser"))
        .Times(Exactly(1))
        .WillOnce(Return("testrefreshtoken"));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::ok, response.get_status_code());
        }));

    // test execution
    {
        wago::authserv::process_token_request(
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_token_request_fixture, deny_token_by_password_with_wrong_password)
{
    std::string test_client_id =                           "testclient";
    request_mock.expect_request_form_checked("grant_type", "password");
    request_mock.expect_request_form_checked("client_id",  test_client_id);
    request_mock.expect_request_form_checked("scope",      "testserver:testscope");
    request_mock.expect_request_form_checked("username",   "testuser", true);
    request_mock.expect_request_form_checked("password",   "testpassword", true);
    

    std::vector<std::string> test_setting_grant_type_value {
        "some_type",
        "password",
        "some_other"
    };
    std::vector<std::string> test_setting_scopes {
        "some_other_scope",
        "testserver:testscope",
        "yet_another_scope"
    };
    std::string test_empty;
    settings_store_i::oauth_client test_setting_client({
        .id=test_client_id,
        .label=test_empty,
        .redirect_uri=test_empty,
        .grant_types=test_setting_grant_type_value
    });

    EXPECT_CALL(request_mock, is_localhost())
        .Times(AtLeast(1))
        .WillOnce(Return(true));

    expect_client_check(test_setting_client);
    EXPECT_CALL(settings_store_mock, get_all_scopes())
        .Times(AtLeast(1))
        .WillRepeatedly(
            testing::ReturnRef(test_setting_scopes)
        );

    EXPECT_CALL(authenticator_mock, has_form_auth_data(Ref(request_mock)))
        .Times(AnyNumber())
        .WillRepeatedly(
            Return(true)
        );

    EXPECT_CALL(authenticator_mock, authenticate(Ref(request_mock), std::vector<std::string> { "testserver:testscope" }))
        .Times(Exactly(1))
        .WillRepeatedly(
            Return( auth_result {false, false, false, "testuser"})
        );

    EXPECT_CALL(authenticator_mock, admin_has_no_password())
        .Times(AnyNumber())
        .WillRepeatedly(
            Return(false)
        );

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::bad_request, response.get_status_code());
        }));

    // test execution
    {
        wago::authserv::process_token_request(
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_token_request_fixture, deny_token_by_password_no_admin_password)
{
    std::string test_client_id =                           "testclient";
    request_mock.expect_request_form_checked("grant_type", "password");
    request_mock.expect_request_form_checked("client_id",  test_client_id);
    request_mock.expect_request_form_checked("scope",      "testserver:testscope");
    request_mock.expect_request_form_checked("username",   "testuser", true);
    request_mock.expect_request_form_checked("password",   "testpassword", true);
    

    std::vector<std::string> test_setting_grant_type_value {
        "some_type",
        "password",
        "some_other"
    };
    std::vector<std::string> test_setting_scopes {
        "some_other_scope",
        "testserver:testscope",
        "yet_another_scope"
    };
    std::string test_empty;
    settings_store_i::oauth_client test_setting_client({
        .id=test_client_id,
        .label=test_empty,
        .redirect_uri=test_empty,
        .grant_types=test_setting_grant_type_value
    });

    EXPECT_CALL(request_mock, is_localhost())
        .Times(AtLeast(1))
        .WillOnce(Return(true));

    expect_client_check(test_setting_client);
    EXPECT_CALL(settings_store_mock, get_all_scopes())
        .Times(AtLeast(1))
        .WillRepeatedly(
            testing::ReturnRef(test_setting_scopes)
        );

    EXPECT_CALL(authenticator_mock, has_form_auth_data(Ref(request_mock)))
        .Times(AnyNumber())
        .WillRepeatedly(
            Return(true)
        );

    EXPECT_CALL(authenticator_mock, authenticate(Ref(request_mock), std::vector<std::string> { "testserver:testscope" }))
        .Times(Exactly(1))
        .WillRepeatedly(
            Return( auth_result {true, false, false, "testuser"})
        );

    EXPECT_CALL(authenticator_mock, admin_has_no_password())
        .Times(AtLeast(1))
        .WillRepeatedly(
            Return(true)
        );

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::bad_request, response.get_status_code());
        }));

    // test execution
    {
        wago::authserv::process_token_request(
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}


TEST_F(process_token_request_fixture, deny_token_by_password_expired)
{
    std::string test_client_id =                           "testclient";
    request_mock.expect_request_form_checked("grant_type", "password");
    request_mock.expect_request_form_checked("client_id",  test_client_id);
    request_mock.expect_request_form_checked("scope",      "testserver:testscope");
    request_mock.expect_request_form_checked("username",   "testuser", true);
    request_mock.expect_request_form_checked("password",   "testpassword", true);
    

    std::vector<std::string> test_setting_grant_type_value {
        "some_type",
        "password",
        "some_other"
    };
    std::vector<std::string> test_setting_scopes {
        "some_other_scope",
        "testserver:testscope",
        "yet_another_scope"
    };
    std::string test_empty;
    settings_store_i::oauth_client test_setting_client({
        .id=test_client_id,
        .label=test_empty,
        .redirect_uri=test_empty,
        .grant_types=test_setting_grant_type_value
    });
    expect_client_check(test_setting_client);
    EXPECT_CALL(settings_store_mock, get_all_scopes())
        .Times(AtLeast(1))
        .WillRepeatedly(
            testing::ReturnRef(test_setting_scopes)
        );

    EXPECT_CALL(authenticator_mock, has_form_auth_data(Ref(request_mock)))
        .Times(AnyNumber())
        .WillRepeatedly(
            Return(true)
        );

    EXPECT_CALL(request_mock, is_localhost())
        .Times(Exactly(1))
        .WillOnce(Return(true));

    EXPECT_CALL(authenticator_mock, authenticate(Ref(request_mock), std::vector<std::string> { "testserver:testscope" }))
        .Times(Exactly(1))
        .WillRepeatedly(
            Return( auth_result {true, true, false, "testuser"})
        );

    EXPECT_CALL(authenticator_mock, admin_has_no_password())
        .Times(AtLeast(1))
        .WillRepeatedly(
            Return(false)
        );

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::bad_request, response.get_status_code());
            EXPECT_THAT(response.get_content(), ::testing::HasSubstr("Password expired"));
        }));

    // test execution
    {
        wago::authserv::process_token_request(
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_token_request_fixture, get_token_by_refresh_token)
{
    request_mock.expect_request_form_checked("grant_type",    "refresh_token");
    request_mock.expect_request_form_checked("client_id",     "testclient", true);
    request_mock.expect_request_form_checked("scope",         "testscope");
    request_mock.expect_request_form_checked("refresh_token", "testtoken");

    std::string test_access_token_lifetime        = "123";
    settings_store_mock.expect_global_setting_read(settings_store_i::access_token_lifetime, test_access_token_lifetime);

    EXPECT_CALL(oauth_token_handler_mock, validate_refresh_token("testtoken"))
        .Times(Exactly(1))
        .WillOnce(Return(oauth_token_handler_i::validation_result {
                true,
                false,
                1,
                "testclient",
                "testscope",
                "testuser"
            }));
    
    EXPECT_CALL(oauth_token_handler_mock, generate_access_token(::testing::_, "testclient", "testscope", "testuser"))
        .Times(Exactly(1))
        .WillOnce(Return("testaccesstoken"));

    EXPECT_CALL(request_mock, is_localhost())
        .Times(Exactly(1))
        .WillOnce(Return(true));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](http::response_i const &response) {
            EXPECT_EQ(http::http_status_code::ok, response.get_status_code());
        }));

    // test execution
    {
        wago::authserv::process_token_request(
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}
