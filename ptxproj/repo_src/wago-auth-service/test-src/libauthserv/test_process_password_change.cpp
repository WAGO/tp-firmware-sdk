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
///  \brief    Test password change endpoint implementation.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "password_change.hpp"

#include "mocks/mock_authenticator.hpp"
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


class process_password_change_request_fixture: public ::testing::Test
{
public:
    enum request_type
    {
        INITIAL_REQUEST,
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
    std::shared_ptr<mock_request>         request_mock;
    std::shared_ptr<mock_settings_store>  settings_store_mock;
    mock_authenticator                    authenticator_mock;
    std::shared_ptr<mock_response_helper> response_helper_mock;

    // request parameters
    std::string test_request_username;
    std::string test_request_password;
    std::string test_setting_redirect_uri_value;

    // expected result
    std::string                   expected_username;
    http_status_code              expected_http_status;
    std::string                   expected_redirect_uri;
    response_helper_i::error_type expected_error_type;
    std::string                   expected_error_message;
    std::string                   expected_info_message;
    std::string                   expected_success_message;

public:
    process_password_change_request_fixture()
    : request_mock(std::make_shared<mock_request>())
    , settings_store_mock(std::make_shared<mock_settings_store>())
    , response_helper_mock(std::make_shared<mock_response_helper>())
    , test_request_username("testuser")
    , test_request_password("testpass")
    , test_setting_redirect_uri_value(response_helper_i::CLOSE_TAB)

    , expected_username(test_request_username)
    , expected_http_status(http_status_code::ok)
    , expected_redirect_uri(test_setting_redirect_uri_value)
    , expected_error_type(response_helper_i::no_error)
    , expected_error_message()
    , expected_info_message() 
    , expected_success_message()
    {}

    void SetUp() override
    {
        request_mock->set_default_expectations();
        settings_store_mock->set_default_expectations();
        authenticator_mock.set_default_expectations();
    }

    void expect_request_analysis(request_type type, bool optional = false)
    {
        EXPECT_CALL(authenticator_mock, has_form_password_change_data(Ref(*request_mock)))
            .Times(AnyNumber())
            .WillRepeatedly(
                Return(type == PASSWORD_CHANGE_SUBMIT)
            );

        if(type == INITIAL_REQUEST) {
            request_mock->expect_request_query_checked("username", test_request_username, true);
        }

        EXPECT_CALL(*request_mock, get_method())
            .Times(AtLeast(optional ? 0 : 1))
            .WillRepeatedly(
                Return(type == INITIAL_REQUEST ? http_method::get : http_method::post)
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
        EXPECT_CALL(authenticator_mock, authenticate(Ref(*request_mock), std::vector<std::string> {}))
            .Times(Exactly(1))
            .WillRepeatedly(
                Return(result)
            );
    }

    void expect_password_change()
    {
        EXPECT_CALL(authenticator_mock, password_change(Ref(*request_mock)))
            .Times(Exactly(1))
            .WillRepeatedly(WithoutArgs(
                Invoke([]() -> wago::future<void> { return wago::resolved_future(); })
            ));
    }

    void expect_password_change_page()
    {
        request_mock->expect_request_form_checked("username", test_request_username, true);
        EXPECT_CALL(*response_helper_mock, send_password_change_page(Ref(*request_mock),
                                                                     expected_redirect_uri,
                                                                     expected_http_status,
                                                                     expected_username,
                                                                     expected_error_type,
                                                                     expected_error_message,
                                                                     expected_info_message));
    }

    void expect_success_page() {
        request_mock->expect_request_form_checked("username", test_request_username);
        EXPECT_CALL(*response_helper_mock, send_success_confirmation(Ref(*request_mock),
                                                                     expected_redirect_uri,
                                                                     expected_http_status,
                                                                     expected_success_message));
    }

    void expect_error_page() {
        EXPECT_CALL(*response_helper_mock, send_error_confirmation(Ref(*request_mock),
                                                                   expected_redirect_uri,
                                                                   expected_http_status,
                                                                   expected_error_type,
                                                                   expected_error_message));
    }
};

TEST_F(process_password_change_request_fixture, get_password_change_page)
{
    expect_request_analysis(INITIAL_REQUEST);
    expect_password_change_page();
    {
        wago::authserv::process_password_change_request(
            response_helper_mock,
            request_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_password_change_request_fixture, successfully_change_password)
{
    expect_request_analysis(PASSWORD_CHANGE_SUBMIT);
    expect_authentication();
    expect_password_change();
    expected_success_message = "Successfully updated password for user testuser.";
    expect_success_page();
    {
        wago::authserv::process_password_change_request(
            response_helper_mock,
            request_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_password_change_request_fixture, wrong_password_response)
{
    expect_request_analysis(PASSWORD_CHANGE_SUBMIT);
    expect_authentication(UNAUTHENTICATED);
    expected_http_status = http_status_code::bad_request;
    expected_error_type = response_helper_i::invalid_username_or_password;
    expect_password_change_page();
    {
        wago::authserv::process_password_change_request(
            response_helper_mock,
            request_mock,
            authenticator_mock
        );
    }
}

TEST_F(process_password_change_request_fixture, wrong_parameters_error)
{
    test_request_username = ""; // no username query argument in request
    expect_request_analysis(INITIAL_REQUEST);
    expected_http_status = http_status_code::bad_request;
    expected_error_type = response_helper_i::invalid_request;
    expected_error_message = "Missing query parameter 'username'";
    expect_error_page();
    {
        wago::authserv::process_password_change_request(
            response_helper_mock,
            request_mock,
            authenticator_mock
        );
    }
}
