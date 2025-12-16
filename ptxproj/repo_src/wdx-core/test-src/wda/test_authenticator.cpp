//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
#include "wago/wdx/wda/trace_routes.hpp"
#include "auth/authenticator.hpp"
#include "http/url_utils.hpp"

#include "mocks/mock_auth_settings.hpp"
#include "mocks/mock_password_backend.hpp"
#include "mocks/mock_token_backend.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_authenticated_request_handler.hpp"
#include "mocks/mock_service_identity.hpp"

#include <wc/preprocessing.h>
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::auth;
using wago::wdx::wda::auth_settings_i;
using wago::wdx::wda::http::http_status_code;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char     const test_service_base[]            = "/service/base";

constexpr char     const token_header_name[]            = "WAGO-WDX-Auth-Token";
constexpr char     const default_test_token[]           = "loremipsumtoken";

constexpr char     const token_type_header_name[]       = "WAGO-WDX-Auth-Token-Type";
constexpr char     const token_type_header_value[]      = "Bearer";

#define                  DEFAULT_TOKEN_EXPIRATION         300
constexpr char     const token_expiration_header_name[] = "WAGO-WDX-Auth-Token-Expiration";
constexpr uint32_t const default_token_expiration_value = DEFAULT_TOKEN_EXPIRATION;
constexpr char     const default_token_expiration[]     = WC_STR(300);
#undef                   DEFAULT_TOKEN_EXPIRATION

constexpr char     const password_expired_header_name[] = "WAGO-WDX-Auth-Password-Expired";

constexpr char     const pragma_header_name[]           = "Pragma";
constexpr char     const pragma_header_value[]          = "no-cache";
constexpr char     const cache_control_header_name[]    = "Cache-Control";
constexpr char     const cache_control_header_value[]   = "no-store";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class authenticator_fixture: public ::testing::Test
{
public:
    authenticator * auth = nullptr;

    mock_authenticated_request_handler *handler_mock          = nullptr;
    mock_auth_settings                 *auth_settings_mock    = nullptr;
    mock_password_backend              *password_backend_mock = nullptr;
    mock_token_backend                 *token_backend_mock    = nullptr;
    mock_service_identity               service_identity_mock;

    authenticator_fixture()
    { }

    void SetUp() override
    {
        std::shared_ptr<mock_authenticated_request_handler> test_handler = std::make_shared<mock_authenticated_request_handler>();
        std::shared_ptr<mock_auth_settings>                 test_auth_settings         = std::make_shared<mock_auth_settings>();
        std::unique_ptr<mock_password_backend>              test_password_backend      = std::make_unique<mock_password_backend>();
        std::unique_ptr<mock_token_backend>                 test_token_backend         = std::make_unique<mock_token_backend>();

        handler_mock          = test_handler.get();
        auth_settings_mock    = test_auth_settings.get();
        password_backend_mock = test_password_backend.get();
        token_backend_mock    = test_token_backend.get();

        auth = new authenticator(test_auth_settings,
                                 std::move(test_password_backend),
                                 std::move(test_token_backend),
                                 test_service_base,
                                 wago::wdx::wda::trace_route::none,
                                 test_handler);

        handler_mock->set_default_expectations();
        auth_settings_mock->set_default_expectations();
        password_backend_mock->set_default_expectations();
        token_backend_mock->set_default_expectations();
        service_identity_mock.set_default_expectations();
    }
    void TearDown() override
    {
        delete auth;
    }
};

TEST_F(authenticator_fixture, construct_delete)
{
    // Nothing to do, everything is done in SetUp/TearDown
}

TEST_F(authenticator_fixture, handle_successful_basic_auth)
{
    // Request setup
    std::string const test_user              = "user";
    std::string const test_password          = "password";
    std::string const test_auth_header_value = "Basic dXNlcjpwYXNzd29yZA=="; // Base64(user:password)

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(test_user), ::testing::StrEq(test_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { test_user, true, false, default_test_token, default_token_expiration_value }));

    // May ask for the request's path
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(Return("/some/path"));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    // Should redirect handle
    EXPECT_CALL(*handler_mock, handle_mock(::testing::Ref(*test_request.get()), authentication_info { test_user }))
        .Times(Exactly(1));

    // Should add the token type header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_type_header_name), ::testing::StrEq(token_type_header_value)))
        .Times(Exactly(1));

    // Should add the token expiration header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_expiration_header_name), ::testing::StrEq(default_token_expiration)))
        .Times(Exactly(1));

    // Should add the token header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_header_name), ::testing::StrEq(default_test_token)))
        .Times(Exactly(1));

    // Should add the pragma header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(pragma_header_name), ::testing::StrEq(pragma_header_value)))
        .Times(Exactly(1));

    // Should add the cache-control header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(cache_control_header_name), ::testing::StrEq(cache_control_header_value)))
        .Times(Exactly(1));

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_successful_expired_basic_auth)
{
    // Request setup
    std::string const test_user              = "user";
    std::string const test_password          = "password";
    std::string const test_auth_header_value = "Basic dXNlcjpwYXNzd29yZA=="; // Base64(user:password)

    // Response setup
    std::string const test_password_expired        = "true";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(test_user), ::testing::StrEq(test_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { test_user, true, true, default_test_token, default_token_expiration_value }));

    // May ask for the request's path
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(Return("/some/path"));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    // Should redirect handle
    EXPECT_CALL(*handler_mock, handle_mock(::testing::Ref(*test_request.get()), authentication_info { test_user }))
        .Times(Exactly(1));

    // Should add the token type header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_type_header_name), ::testing::StrEq(token_type_header_value)))
        .Times(Exactly(1));

    // Should add the token expiration header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_expiration_header_name), ::testing::StrEq(default_token_expiration)))
        .Times(Exactly(1));

    // Should add the token header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_header_name), ::testing::StrEq(default_test_token)))
        .Times(Exactly(1));

    // Should add the pragma header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(pragma_header_name), ::testing::StrEq(pragma_header_value)))
        .Times(Exactly(1));

    // Should add the cache-control header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(cache_control_header_name), ::testing::StrEq(cache_control_header_value)))
        .Times(Exactly(1));

    // Should set password expired header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(password_expired_header_name), ::testing::StrEq(test_password_expired)))
        .Times(Exactly(1));

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_successful_bearer_auth)
{
    // Request setup
    std::string const test_user              = "test_user";
    std::string const test_bearer            = "testbearertokenvalue";
    std::string const test_auth_header_value = "Bearer testbearertokenvalue";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    
    // Token backend should be used to authenticate
    EXPECT_CALL(*token_backend_mock, authenticate(::testing::StrEq(test_bearer), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { test_user, true, false, default_test_token, default_token_expiration_value }));

    // May ask for the request's path
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(Return("/some/path"));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    // Should redirect handle
    EXPECT_CALL(*handler_mock, handle_mock(::testing::Ref(*test_request.get()), authentication_info { test_user }))
        .Times(Exactly(1));

    // Should add the token type header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_type_header_name), ::testing::StrEq(token_type_header_value)))
        .Times(Exactly(1));

    // Should add the token expiration header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_expiration_header_name), ::testing::StrEq(default_token_expiration)))
        .Times(Exactly(1));

    // Should add the token header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_header_name), ::testing::StrEq(default_test_token)))
        .Times(Exactly(1));

    // Should add the pragma header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(pragma_header_name), ::testing::StrEq(pragma_header_value)))
        .Times(Exactly(1));

    // Should add the cache-control header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(cache_control_header_name), ::testing::StrEq(cache_control_header_value)))
        .Times(Exactly(1));

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_successful_expired_bearer_auth)
{
    // Request setup
    std::string const test_user              = "test_user";
    std::string const test_bearer            = "testbearertokenvalue";
    std::string const test_auth_header_value = "Bearer testbearertokenvalue";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    
    // Token backend should be used to authenticate
    EXPECT_CALL(*token_backend_mock, authenticate(::testing::StrEq(test_bearer), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { test_user, true, true, default_test_token, default_token_expiration_value }));

    // May ask for the request's path
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(::testing::AnyNumber())
        .WillRepeatedly(Return("/some/path"));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    // Should redirect handle
    EXPECT_CALL(*handler_mock, handle_mock(::testing::Ref(*test_request.get()), authentication_info { test_user }))
        .Times(Exactly(1));

    // Should add the token type header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_type_header_name), ::testing::StrEq(token_type_header_value)))
        .Times(Exactly(1));

    // Should add the token expiration header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_expiration_header_name), ::testing::StrEq(default_token_expiration)))
        .Times(Exactly(1));

    // Should add the token header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(token_header_name), ::testing::StrEq(default_test_token)))
        .Times(Exactly(1));

    // Should add the pragma header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(pragma_header_name), ::testing::StrEq(pragma_header_value)))
        .Times(Exactly(1));

    // Should add the cache-control header
    EXPECT_CALL(*test_request, add_response_header(::testing::StrCaseEq(cache_control_header_name), ::testing::StrEq(cache_control_header_value)))
        .Times(Exactly(1));

    auth->handle(std::move(test_request));
}

ACTION(check_response_is_unauthenticated) {
    response_i const &res = arg0;
    EXPECT_EQ(res.get_status_code(), http_status_code::unauthorized);
    EXPECT_EQ(res.get_content_type(), "");
    EXPECT_EQ(res.get_content(), "");
    EXPECT_THAT(res.get_response_header().at("WWW-Authenticate"), ::testing::AllOf(
        ::testing::HasSubstr("Basic"),
        ::testing::HasSubstr("Bearer")
    ));
    EXPECT_THAT(res.get_response_header().at("WAGO-WDX-Auth-Methods"), ::testing::AllOf(
        ::testing::HasSubstr("Password"),
        ::testing::HasSubstr("WDXToken"),
        ::testing::HasSubstr("OAuth2")
    ));
}

TEST_F(authenticator_fixture, handle_failed_basic_auth)
{
    // Request setup
    std::string const test_user              = "user";
    std::string const test_password          = "password";
    std::string const test_auth_header_value = "Basic dXNlcjpwYXNzd29yZA=="; // Base64(user:password)

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    
    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    // Password backend should be used to authenticate
    EXPECT_CALL(*password_backend_mock, authenticate(::testing::StrEq(test_user), ::testing::StrEq(test_password)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result()));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(check_response_is_unauthenticated());

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_failed_bearer_auth)
{
    // Request setup
    std::string const test_bearer            = "testbearertokenvalue";
    std::string const test_auth_header_value = "Bearer testbearertokenvalue";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));

    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    // Token backend should be used to authenticate
    EXPECT_CALL(*token_backend_mock, authenticate(::testing::StrEq(test_bearer), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result()));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(check_response_is_unauthenticated());

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_failed_expired_bearer_auth)
{
    // Request setup
    std::string const test_user              = "test_user";
    std::string const test_bearer            = "testbearertokenvalue";
    std::string const test_auth_header_value = "Bearer testbearertokenvalue";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));

    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    // Token backend should be used to authenticate
    EXPECT_CALL(*token_backend_mock, authenticate(::testing::StrEq(test_bearer), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(auth_result { test_user, false, true, "", 0 }));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(check_response_is_unauthenticated());

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_allowed_unauthenticated_url)
{
    std::string const allowed_urls = "/some/path/first/test;/some/second/path;/some/third/path/for/test";

    // Request setup
    std::string const test_auth_header_value = "";
    std::string const test_path              = std::string(test_service_base) + "/some/second/path/";
    std::string const test_uri               = std::string(test_service_base) + "/some/second/path/?with-an-query-string=ask";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_uri));

    // Should ask for allowed unauthenticated urls
    EXPECT_CALL(*auth_settings_mock, get_unauthenticated_urls_mock())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(allowed_urls));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    // Should redirect handle
    EXPECT_CALL(*handler_mock, handle_mock(::testing::Ref(*test_request.get()), authentication_info { "unknown" }))
        .Times(Exactly(1));

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_allowed_unauthenticated_url_with_variable_part)
{
    std::string const allowed_urls = "/some/path/:for_variable:/testing";

    // Request setup
    std::string const test_auth_header_value = "";
    std::string const test_path              = std::string(test_service_base) + "/some/path/with-an-variable-for/testing";
    std::string const test_uri               = std::string(test_service_base) + "/some/path/with-an-variable-for/testing";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_uri));

    // Should ask for allowed unauthenticated urls
    EXPECT_CALL(*auth_settings_mock, get_unauthenticated_urls_mock())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(allowed_urls));

    // Should not respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(0);

    // Should redirect handle
    EXPECT_CALL(*handler_mock, handle_mock(::testing::Ref(*test_request.get()), authentication_info { "unknown" }))
        .Times(Exactly(1));

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_failed_unauthenticated_url)
{
    std::string const allowed_urls = "/some/path/first/test;/some/second/path;/some/third/path/for/test";

    // Request setup
    std::string const test_auth_header_value = "";
    std::string const test_path              = std::string(test_service_base) + "/some/unknown/path";
    std::string const test_uri               = std::string(test_service_base) + "/some/unknown/path";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_uri));

    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    // Should ask for allowed unauthenticated urls
    EXPECT_CALL(*auth_settings_mock, get_unauthenticated_urls_mock())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(allowed_urls));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(check_response_is_unauthenticated());

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_failed_unauthenticated_url_contained)
{
    std::string const allowed_urls = "/some/path/first/test";

    // Request setup
    std::string const test_auth_header_value = "";
    std::string const test_path              = std::string(test_service_base) + "/path/first";
    std::string const test_uri               = std::string(test_service_base) + "/path/first";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_uri));

    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));
    
    // Should ask for allowed unauthenticated urls
    EXPECT_CALL(*auth_settings_mock, get_unauthenticated_urls_mock())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(allowed_urls));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(check_response_is_unauthenticated());

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}

TEST_F(authenticator_fixture, handle_failed_unauthenticated_url_empty_allowed)
{
    std::string const allowed_urls = "";

    // Request setup
    std::string const test_auth_header_value = "";
    std::string const test_path              = std::string(test_service_base) + "/some/path";
    std::string const test_uri               = std::string(test_service_base) + "/some/path";

    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_auth_header_value));
    EXPECT_CALL(*test_request, get_request_uri())
        .Times(AnyNumber())
        .WillRepeatedly(Return(test_uri));

    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(false));

    // Should ask for allowed unauthenticated urls
    EXPECT_CALL(*auth_settings_mock, get_unauthenticated_urls_mock())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(allowed_urls));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(check_response_is_unauthenticated());

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}


TEST_F(authenticator_fixture, no_auth_popup_header)
{
    // Create the request
    auto test_request = std::make_unique<mock_request>();
    test_request->set_default_expectations();

    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("Authorization")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return("Inv4lid!"));
    
    // check for "no popup" header
    EXPECT_CALL(*test_request, has_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*test_request, get_http_header(::testing::StrCaseEq("WAGO-WDX-No-Auth-Popup")))
        .Times(AtLeast(1))
        .WillRepeatedly(Return("true"));

    // Should respond the request
    EXPECT_CALL(*test_request, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Invoke([](response_i const &res) {
            EXPECT_EQ(res.get_status_code(), http_status_code::unauthorized);
            EXPECT_EQ(res.get_content_type(), "");
            EXPECT_EQ(res.get_content(), "");
            EXPECT_EQ(res.get_response_header().count("WWW-Authenticate"), 0);
            EXPECT_THAT(res.get_response_header().at("WAGO-WDX-Auth-Methods"), ::testing::AllOf(
                ::testing::HasSubstr("Password"),
                ::testing::HasSubstr("WDXToken"),
                ::testing::HasSubstr("OAuth2")
            ));
        }));

    // React on CORS requests
    EXPECT_CALL(*test_request, get_http_header(::testing::StrEq("Origin")))
        .Times(1);

    // May add additional response headers
    EXPECT_CALL(*test_request, add_response_header(::testing::_, ::testing::_))
        .Times(AnyNumber());

    auth->handle(std::move(test_request));
}


//---- End of source file ------------------------------------------------------
