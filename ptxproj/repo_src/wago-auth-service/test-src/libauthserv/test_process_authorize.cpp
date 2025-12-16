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
///  \brief    Test authorize endpoint implementation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authorize.hpp"

#include "mocks/mock_oauth_token_handler.hpp"
#include "mocks/mock_authenticator.hpp"
#include "mocks/mock_user_management.hpp"
#include "mocks/mock_password_backend.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_settings_store.hpp"
#include "mocks/mock_response_helper.hpp"

#include "wago/authserv/http/http_method.hpp"
#include "wago/authserv/http/http_status_code.hpp"
#include "wago/authserv/http/response_i.hpp"
#include "http/http_exception.hpp"
#include "password_change_backend_i.hpp"

#include <wc/assertion.h>
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
using namespace wago::authserv::http;
using testing::StartsWith;
using testing::HasSubstr;
using testing::Ref;
using testing::WithArg;
using testing::WithoutArgs;
using testing::Throw;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------


class process_authorize_request_fixture: public ::testing::Test
{
public:
    enum request_type
    {
        INITIAL_REQUEST,
        LOGIN_SUBMIT,
        PASSWORD_CHANGE_SUBMIT
    };

    enum authentication_state
    {
        UNAUTHENTICATED,
        AUTHENTICATED,
        EXPIRED,
        UNAUTHORIZED
    };

protected:
    std::shared_ptr<mock_request>             request_mock;
    std::shared_ptr<mock_settings_store>      settings_store_mock;
    mock_authenticator                        authenticator_mock;
    mock_user_management                      user_management_mock;
    std::shared_ptr<mock_oauth_token_handler> oauth_token_handler_mock;
    std::shared_ptr<mock_response_helper>     response_helper_mock;

    // request parameters
    std::string test_request_client_id;
    std::string test_request_response_type;
    std::string test_request_code_challenge_method;
    std::string test_request_code_challenge;
    std::string test_request_scope;
    std::string test_request_state;
    std::string test_request_username;

    // settings
    std::string test_setting_client_id;
    std::string test_setting_client_label_value;
    std::string test_setting_redirect_uri_value;
    std::vector<std::string> test_setting_grant_type_value;
    std::vector<std::string> test_setting_scopes;
    std::string test_setting_auth_code_lifetime;

    // generated values
    std::string test_auth_code;

    // expected result
    std::string expected_username;
    std::string expected_client_label;
    std::string expected_redirect_uri;
    std::string expected_state;
    std::string expected_info_message;

public:
    process_authorize_request_fixture()
    : request_mock(std::make_shared<mock_request>())
    , settings_store_mock(std::make_shared<mock_settings_store>())
    , oauth_token_handler_mock(std::make_shared<mock_oauth_token_handler>())
    , response_helper_mock(std::make_shared<mock_response_helper>())
    , test_request_client_id("my_client")
    , test_request_response_type("code")
    , test_request_code_challenge_method("S256")
    , test_request_code_challenge("test_challenge")
    , test_request_scope("my_client:test_scope")
    , test_request_state("teststate")
    , test_request_username("testuser")

    , test_setting_client_id(test_request_client_id)
    , test_setting_client_label_value("My Test Client")
    , test_setting_redirect_uri_value("/test/redirect")
    , test_setting_grant_type_value({
        "some_type",
        "authorization_code",
        "some_other"
    })
    , test_setting_scopes({
        "other_server:some_other_scope",
        test_request_scope,
        "my_client:test_scope_2",
        "yet_another_server" // no scopes defined
    })
    , test_setting_auth_code_lifetime("123")

    , test_auth_code("testcode")

    , expected_username(test_request_username)
    , expected_client_label(test_setting_client_label_value)
    , expected_redirect_uri(test_setting_redirect_uri_value)
    , expected_state(test_request_state)
    , expected_info_message("Your password is expired. A password change is required.") // default info on password change page
    {}

    void SetUp() override
    {
        request_mock->set_default_expectations();
        settings_store_mock->set_default_expectations();
        authenticator_mock.set_default_expectations();
        user_management_mock.set_default_expectations();
        oauth_token_handler_mock->set_default_expectations();
    }

    void expect_request_analysis(request_type type, bool optional = false)
    {

        EXPECT_CALL(authenticator_mock, admin_has_no_password())
            .Times(AnyNumber())
            .WillRepeatedly(
                Return(false)
            );
        EXPECT_CALL(authenticator_mock, has_form_auth_data(Ref(*request_mock)))
            .Times(AnyNumber())
            .WillRepeatedly(
                // password change also contains login data
                Return(type == LOGIN_SUBMIT || type == PASSWORD_CHANGE_SUBMIT)
            );
        EXPECT_CALL(authenticator_mock, has_form_password_change_data(Ref(*request_mock)))
            .Times(AnyNumber())
            .WillRepeatedly(
                Return(type == PASSWORD_CHANGE_SUBMIT)
            );

        std::string test_request_query = "?client_id=" + test_request_client_id
                                              + "&response_type=" + test_request_response_type
                                              + "&code_challenge_method=" + test_request_code_challenge_method
                                              + "&code_challenge=" + test_request_code_challenge
                                              + "&test_request_scope=" + test_request_scope
           + (test_request_state.empty() ? "" : "&state=" + test_request_state);
        EXPECT_CALL(*request_mock, get_request_uri())
            .Times(AnyNumber())
            .WillRepeatedly(
                Return("https://only.query.matters.for.test" + test_request_query)
            );
        request_mock->expect_request_query_checked("client_id",             test_request_client_id);
        request_mock->expect_request_query_checked("response_type",         test_request_response_type        , optional);
        request_mock->expect_request_query_checked("state",                 test_request_state                , optional);
        request_mock->expect_request_query_checked("code_challenge_method", test_request_code_challenge_method, optional);
        request_mock->expect_request_query_checked("code_challenge",        test_request_code_challenge       , optional);
        request_mock->expect_request_query_checked("scope",                 test_request_scope                , optional);

        EXPECT_CALL(*request_mock, get_method())
            .Times(AtLeast(optional ? 0 : 1))
            .WillRepeatedly(
                Return(type == INITIAL_REQUEST ? http_method::get : http_method::post)
            );
    }

    void expect_settings_read_for_validation()
    {
        static settings_store_i::oauth_client test_setting_client({
            .id=test_request_client_id,
            .label=test_setting_client_label_value,
            .redirect_uri=test_setting_redirect_uri_value,
            .grant_types=test_setting_grant_type_value
        });
        expect_settings_read_for_validation(test_setting_client);
    }

    void expect_settings_read_for_validation(settings_store_i::oauth_client &settings_store_client)
    {
        EXPECT_CALL(*settings_store_mock, client_exists(test_setting_client_id))
            .Times(AtLeast(1))
            .WillRepeatedly(
                Return(true)
            );

        EXPECT_CALL(*settings_store_mock, get_client(test_request_client_id))
            .Times(AtLeast(1))
            .WillRepeatedly(
                testing::ReturnRef(settings_store_client)
            );

        EXPECT_CALL(*settings_store_mock, get_all_scopes())
            .Times(AtLeast(1))
            .WillRepeatedly(
                testing::ReturnRef(test_setting_scopes)
            );
    }

    void expect_authentication(authentication_state state = AUTHENTICATED)
    {
        auth_result result;
        switch (state)
        {
            case UNAUTHENTICATED:
                result = auth_result{false, false, false, test_request_username};
                break;
            case AUTHENTICATED:
                result = auth_result{true, false, false, test_request_username};
                break;
            case EXPIRED:
                result = auth_result{true, true, false, test_request_username};
                break;
            case UNAUTHORIZED:
                result = auth_result{true, false, true, test_request_username};
                break;
        }
        EXPECT_CALL(authenticator_mock, authenticate(Ref(*request_mock), std::vector<std::string> { test_request_scope }))
            .Times(Exactly(1))
            .WillRepeatedly(
                Return(result)
            );
    }

    void expect_redirect(std::string expected_error_message = "")
    {
        bool             is_error             = !expected_error_message.empty();
        http_status_code expected_http_status = http_status_code::found;

        // expect token generation
        if(!is_error)
        {
            EXPECT_CALL(*oauth_token_handler_mock, generate_auth_code(
                    ::testing::_, test_request_client_id,
                    test_request_scope, test_request_username, test_request_code_challenge)
                )
                .Times(Exactly(1))
                .WillOnce(Return(test_auth_code));

            settings_store_mock->expect_global_setting_read(settings_store_i::auth_code_lifetime, test_setting_auth_code_lifetime);
        }

        // expect redirect header
        EXPECT_CALL(*request_mock, add_response_header("Location", ::testing::StartsWith(expected_redirect_uri)))
            .Times(Exactly(1))
            .WillRepeatedly(Invoke([this, is_error, expected_error_message](std::string const &, std::string const &header_value) {
                EXPECT_THAT(header_value, HasSubstr("state=" + test_request_state));
                if (is_error)
                {
                    EXPECT_THAT(header_value, HasSubstr("error=" + expected_error_message));
                    EXPECT_THAT(header_value, Not(HasSubstr("code=" + test_auth_code)));
                }
                else
                {
                    EXPECT_THAT(header_value, HasSubstr("code=" + test_auth_code));
                    EXPECT_THAT(header_value, Not(HasSubstr("error=" + expected_error_message)));
                }
            }));

        EXPECT_CALL(*request_mock, respond_mock(::testing::_))
            .Times(Exactly(1))
            .WillOnce(testing::Invoke([expected_http_status](response_i const &response) {
                EXPECT_EQ(expected_http_status, response.get_status_code());
            }));

        EXPECT_CALL(*request_mock, finish())
            .Times(AtMost(1));
    }

    void expect_login_response(http_status_code              expected_http_status     = http_status_code::ok,
                               response_helper_i::error_type expected_error_type      = response_helper_i::no_error,
                               std::string                   expected_error_message   = "",
                               std::string                   expected_success_message = "")
    {
        std::string expected_cancel_uri(expected_redirect_uri +
                                        "?error=access_denied" +
                                        "&error_description=Cancel%20requested" +
                                        (expected_state.empty() ? "" : "&state=" + expected_state));
        EXPECT_CALL(*response_helper_mock, send_login_page(Ref(*request_mock),
                                                           expected_cancel_uri,
                                                           expected_http_status,
                                                           expected_client_label,
                                                           expected_error_type,
                                                           expected_error_message,
                                                           expected_success_message))
            .Times(Exactly(1));
    }

    void expect_password_change_response(http_status_code              expected_http_status   = http_status_code::ok,
                                         response_helper_i::error_type expected_error_type    = response_helper_i::no_error,
                                         std::string                   expected_error_message = "")
    {
        std::string expected_cancel_uri(expected_redirect_uri +
                                        "?error=access_denied" +
                                        "&error_description=Cancel%20requested" +
                                        (expected_state.empty() ? "" : "&state=" + expected_state));
        EXPECT_CALL(*response_helper_mock, send_password_change_page(Ref(*request_mock),
                                                                     expected_cancel_uri,
                                                                     expected_http_status,
                                                                     expected_username,
                                                                     expected_error_type,
                                                                     expected_error_message,
                                                                     expected_info_message))
            .Times(Exactly(1));
    }
};

TEST_F(process_authorize_request_fixture, get_login_page)
{
    expect_request_analysis(INITIAL_REQUEST);
    expect_settings_read_for_validation();
    expect_login_response();

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, wrong_client_id)
{
    //change default client id in request (without changing in settings)
    test_request_client_id = "wrong_client";

    expect_request_analysis(INITIAL_REQUEST, true);

    // override default behavior for wrong client
    EXPECT_CALL(*settings_store_mock, client_exists(test_request_client_id))
        .Times(AtLeast(1))
        .WillRepeatedly(
            Return(false)
        );

    EXPECT_THROW({
        {
            wago::authserv::process_authorize_request(
                response_helper_mock,
                settings_store_mock,
                request_mock,
                oauth_token_handler_mock,
                authenticator_mock
            );
        }
    }, http::http_exception);
}

TEST_F(process_authorize_request_fixture, get_login_page_after_failed_login)
{
    expect_request_analysis(LOGIN_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication(UNAUTHENTICATED);
    expect_login_response(http_status_code::bad_request, response_helper_i::invalid_username_or_password);

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, redirect_after_successful_login)
{
    expect_request_analysis(LOGIN_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication();
    expect_redirect();
    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, redirect_after_successful_login_with_id_as_scope)
{
    // change scope to id only in request and settings
    test_request_scope = test_request_client_id;
    test_setting_scopes = {test_request_scope, "another_scope"};

    expect_request_analysis(LOGIN_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication();
    expect_redirect();
    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, redirect_on_access_denied)
{
    expect_request_analysis(LOGIN_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication(UNAUTHORIZED);
    expect_redirect("access_denied");
    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, redirect_on_wrong_scope)
{
    // change scope in request (without changing setting
    test_request_scope = "wrong_scope";

    expect_request_analysis(INITIAL_REQUEST, true);
    expect_settings_read_for_validation();
    expect_redirect("invalid_scope");

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, get_password_change_page_on_expired_password)
{
    expect_request_analysis(LOGIN_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication(EXPIRED);
    expect_password_change_response();

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}


TEST_F(process_authorize_request_fixture, get_password_change_page_after_failed_authentication)
{
    // no info about about expired password
    // without a valid authentication it is unknown if the user has to change the password
    expected_info_message = "";

    expect_request_analysis(PASSWORD_CHANGE_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication(UNAUTHENTICATED);
    expect_password_change_response(http_status_code::bad_request, response_helper_i::invalid_username_or_password);

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}


TEST_F(process_authorize_request_fixture, get_password_change_page_after_failed_change)
{
    expect_request_analysis(PASSWORD_CHANGE_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication(EXPIRED);

    std::string error_msg = "error message for password change";
    EXPECT_CALL(authenticator_mock, password_change(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Throw(wago::authserv::password_change_error(error_msg)));

        expect_password_change_response(http_status_code::bad_request, response_helper_i::internal_error, error_msg);

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_authorize_request_fixture, redirect_after_successful_password_change)
{
    expect_request_analysis(PASSWORD_CHANGE_SUBMIT);
    expect_settings_read_for_validation();
    expect_authentication(EXPIRED);

    EXPECT_CALL(authenticator_mock, password_change(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(testing::ByMove(wago::resolved_future())));

    expect_login_response(http_status_code::ok, response_helper_i::no_error, "", "Successfully updated password.");

    {
        wago::authserv::process_authorize_request(
            response_helper_mock,
            settings_store_mock,
            request_mock,
            oauth_token_handler_mock,
            authenticator_mock
        );
    }
}
