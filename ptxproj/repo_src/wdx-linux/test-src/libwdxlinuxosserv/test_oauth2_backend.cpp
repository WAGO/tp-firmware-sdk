//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test OAuth2 authentication backend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "auth/oauth2_backend.hpp"

#include "mocks/mock_system_clock.hpp"
#include "mocks/mock_curl.hpp"
#include "mocks/mock_settings_store.hpp"

#include <gtest/gtest.h>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::serv::auth::oauth2_backend;
using wago::wdx::wda::auth::auth_result;
using wago::wdx::linuxos::serv::sal::curl;
using ::testing::AnyNumber;
using ::testing::Exactly;
using ::testing::AtLeast;
using ::testing::WithArg;
using ::testing::WithArgs;
using ::testing::Invoke;
using ::testing::Return;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void create_wdx_token_by_default_user_auth(oauth2_backend       &backend,
                                           mock_curl            &curl_mock,
                                           std::string          &wdx_token,
                                           std::string    const &exp_url,
                                           bool           const  add_refresh_token = false);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr size_t      const broken_token_slowdown       = 0;

          std::string const default_user                = "testuser";
          std::string const default_password            = "testpass";
constexpr uint32_t    const default_wdx_expires_in      = 5 * 60;
constexpr uint32_t    const default_access_expires_in   = 1234;
          std::string const default_access_token        = "x-access-y";
          std::string const default_refresh_token       = "x-refresh-y";
          std::string const default_post_content_type   = "application/x-www-form-urlencoded";
          std::string const default_result_content_type = "application/json;charset=UTF-8";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(oauth2_backend, construct_delete)
{
    std::shared_ptr<mock_settings_store> settings_store_mock = std::make_shared<mock_settings_store>();
    settings_store_mock->set_default_expectations();
    size_t const test_broken_token_slowdown = 5;

    // Expect request building with settings store
    EXPECT_CALL(*settings_store_mock, get_setting(testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(""));

    oauth2_backend backend(settings_store_mock, test_broken_token_slowdown);
}

class oauth2_backend_fixture: public ::testing::Test
{
public:
    const std::string origin             = "http://localhost";
    const std::string token_path         = "/auth/token";
    const std::string introspection_path = "/auth/verify";
    const std::string client_id          = "paramd";

private:
    timespec cur_time;

protected:
    std::shared_ptr<mock_settings_store> settings_store_mock;
    mock_system_clock                    clock_mock;
    mock_curl                            curl_mock;

    oauth2_backend_fixture() : settings_store_mock(std::make_shared<mock_settings_store>()) {}
    ~oauth2_backend_fixture() override = default;

    void SetUp() override
    {
        cur_time = { 0, 0 };
        // Expect request building with settings store
        settings_store_mock->set_default_expectations();
        EXPECT_CALL(*settings_store_mock, get_setting(testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(WithArg<0>(Invoke([this](std::string const setting_name){
                if (setting_name == "oauth2-origin") return origin;
                if (setting_name == "oauth2-token-path") return token_path;
                if (setting_name == "oauth2-verify-access-path") return introspection_path;
                if (setting_name == "oauth2-client-id") return client_id;
                return std::string("");
            })));
        clock_mock.set_default_expectations();
        EXPECT_CALL(clock_mock, gettime(CLOCK_MONOTONIC, ::testing::NotNull()))
            .Times(AnyNumber())
            .WillRepeatedly(WithArgs<0, 1>(Invoke(this, &oauth2_backend_fixture::clock_gettime)));

        curl_mock.set_default_expectations();
    }

    int clock_gettime(clockid_t, timespec *res)
    {
        *res = cur_time;
        return 0;
    }

    void advance_system_time(int seconds, int millis = 0, int micros = 0, int nanos = 0)
    {
        cur_time.tv_sec   += seconds;
        cur_time.tv_nsec  += millis * 1'000'000;
        cur_time.tv_nsec  += micros * 1'000;
        cur_time.tv_nsec  += nanos;
    }
};

TEST_F(oauth2_backend_fixture, authenticate_password_success)
{
    std::string          const username         = default_user;
    std::string          const password         = default_password;
    std::string          const exp_url          = origin + token_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"token_type\":\""       "Bearer"                                 "\","
                                                      "\"access_token\":\""   + default_access_token                   + "\","
                                                      "\"refresh_token\":\""  + default_refresh_token                  + "\","
                                                      "\"password_expired\":"   "false"                                    ","
                                                      "\"expires_in\":"       + std::to_string(default_access_expires_in) + ""
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect password grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result, username, password](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="   "password")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("client_id="    "paramd"))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("scope="        "wda"))      << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("username="   + username))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("password="   + password))   << "Content: " + content;
            return post_result;
        })));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    auto result = backend.authenticate(username, password);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_GT(   result.token.length(), default_access_token.length() + default_refresh_token.length());
    EXPECT_LE(   result.token_expires_in, default_wdx_expires_in);
}

TEST_F(oauth2_backend_fixture, authenticate_password_expired)
{
    std::string          const username         = default_user;
    std::string          const password         = default_password;
    std::string          const exp_url          = origin + token_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"token_type\":\""       "Bearer"                                 "\","
                                                      "\"access_token\":\""   + default_access_token                   + "\","
                                                      "\"refresh_token\":\""  + default_refresh_token                  + "\","
                                                      "\"password_expired\":"   "true"                                     ","
                                                      "\"expires_in\":"       + std::to_string(default_access_expires_in) + ""
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect password grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result, username, password](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="   "password")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("client_id="    "paramd"))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("scope="        "wda"))      << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("username="   + username))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("password="   + password))   << "Content: " + content;
            return post_result;
        })));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    auto result = backend.authenticate(username, password);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(  result.user_name, username);
    EXPECT_TRUE(result.expired);
    EXPECT_GT(  result.token.length(), default_access_token.length() + default_refresh_token.length());
    EXPECT_LE(  result.token_expires_in, default_wdx_expires_in);
}

TEST_F(oauth2_backend_fixture, authenticate_password_fail_on_server_400)
{
    std::string          const username         = default_user;
    std::string          const password         = default_password;
    std::string          const exp_url          = origin + token_path;
    std::string          const exp_query        = "";
    std::string          const err_type         = "invalid_scope";
    std::string          const result_content   = "{"
                                                      "\"error\":\""             + err_type        + "\""
                                                  "}";
    curl::request_result const post_result      = { 400, default_result_content_type, result_content };

    // Expect password grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result, username, password](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="   "password")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("client_id="    "paramd"))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("scope="        "wda"))      << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("username="   + username))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("password="   + password))   << "Content: " + content;
            return post_result;
        })));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    auto result = backend.authenticate(username, password);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
}

TEST_F(oauth2_backend_fixture, authenticate_password_fail_on_server_500)
{
    std::string          const username         = default_user;
    std::string          const password         = default_password;
    std::string          const exp_url          = origin + token_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "";
    curl::request_result const post_result      = { 500, default_result_content_type, result_content };

    // Expect password grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result, username, password](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="   "password")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("client_id="    "paramd"))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("scope="        "wda"))      << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("username="   + username))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("password="   + password))   << "Content: " + content;
            return post_result;
        })));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    auto result = backend.authenticate(username, password);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
}

TEST_F(oauth2_backend_fixture, authenticate_password_fail_on_malformed_doc)
{
    std::string          const username         = default_user;
    std::string          const password         = default_password;
    std::string          const exp_url          = origin + token_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "something is=\"no\" valid JSON";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect password grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result, username, password](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="   "password")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("client_id="    "paramd"))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("scope="        "wda"))      << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("username="   + username))   << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("password="   + password))   << "Content: " + content;
            return post_result;
        })));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    auto result = backend.authenticate(username, password);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
}

TEST_F(oauth2_backend_fixture, authenticate_oauth_token_success)
{
    std::string          const token            = default_access_token;
    uint32_t             const expire           = default_access_expires_in;
    std::string          const username         = "test-user";
    std::string          const scopes           = "wda";
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "true"                     ","
                                                      "\"username\":\"" + username               + "\","
                                                      "\"scope\":\""    + scopes                 + "\","
                                                      "\"expires_in\":" + std::to_string(expire) +    ""
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, expire);
}

TEST_F(oauth2_backend_fixture, authenticate_oauth_token_cached)
{
    std::string          const token            = default_access_token;
    uint32_t             const expire           = default_access_expires_in;
    std::string          const username         = "test-user";
    std::string          const scopes           = "wda";
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "true"                     ","
                                                      "\"username\":\"" + username               + "\","
                                                      "\"scope\":\""    + scopes                 + "\","
                                                      "\"expires_in\":" + std::to_string(expire) +    ""
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, expire);

    advance_system_time(1);
    remaining_time = 0;
    result = backend.authenticate(token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, expire - 1);

    // Expect new token verification via cURL
    testing::Mock::VerifyAndClearExpectations(&curl_mock);
    std::string          const result_content_new   = "{"
                                                          "\"active\":"       "false"
                                                      "}";
    curl::request_result const post_result_new      = { 200, default_result_content_type, result_content_new };
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result_new, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result_new;
            }
        )));

    advance_system_time(wago::wdx::linuxos::serv::auth::oauth_token_cache_time);
    remaining_time = 0;
    result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, 0);
}

TEST_F(oauth2_backend_fixture, authenticate_oauth_token_expired)
{
    std::string          const token            = default_access_token;
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "false"
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = default_access_expires_in;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   0, remaining_time);
}

TEST_F(oauth2_backend_fixture, authenticate_oauth_fail_on_server_400)
{
    std::string          const token            = default_access_token;
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const err_description  = "Some Bad Request";
    std::string          const err_type         = "unauthorized_client";
    std::string          const result_content   = "{"
                                                      "\"error_description\":\"" + err_description + "\","
                                                      "\"error\":\""             + err_type        + "\""
                                                  "}";
    curl::request_result const post_result      = { 400, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = default_access_expires_in;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   0, remaining_time);
}

TEST_F(oauth2_backend_fixture, authenticate_oauth_fail_on_server_500)
{
    std::string          const token            = default_access_token;
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "";
    curl::request_result const post_result      = { 500, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = default_access_expires_in;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   0, remaining_time);
}

TEST_F(oauth2_backend_fixture, authenticate_oauth_fail_on_malformed_doc)
{
    std::string          const token            = default_access_token;
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "something is=\"no\" valid JSON";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = default_access_expires_in;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   0, remaining_time);
}

TEST_F(oauth2_backend_fixture, authenticate_fail_on_random_oauth_token)
{
    std::string          const token            = "SomeRandomValue42";
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "false"
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = default_access_expires_in;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   0, remaining_time);
}

TEST_F(oauth2_backend_fixture, authenticate_fail_on_empty_oauth_token)
{
    std::string          const token            = "";
    std::string          const exp_url          = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "false"
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = default_access_expires_in;
    auto result = backend.authenticate(token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   0, remaining_time);
}

void create_wdx_token_by_default_user_auth(oauth2_backend       &backend,
                                           mock_curl            &curl_mock,
                                           std::string          &wdx_token,
                                           std::string    const &exp_url,
                                           bool           const  add_refresh_token)
{
    std::string          const exp_query      = "";
    std::string          const result_content = "{"
                                                    "\"token_type\":\""       "Bearer"                                    "\","
                                                    "\"access_token\":\""   + default_access_token                      + "\","
                            + (add_refresh_token ? ("\"refresh_token\":\""  + default_refresh_token                     + "\",") : "") +
                                                    "\"password_expired\":"   "false"                                       ","
                                                    "\"expires_in\":"       + std::to_string(default_access_expires_in) +    ""
                                                "}";
    curl::request_result const post_result    = { 200, default_result_content_type, result_content };

    // Expect password grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result](std::string){
            return post_result;
        })));

    auto token_result = backend.authenticate(default_user, default_password);
    ASSERT_TRUE( token_result.success);
    ASSERT_FALSE(token_result.token.empty());
    ASSERT_GT(   token_result.token_expires_in, 0);
    ASSERT_GE(   token_result.token_expires_in, default_wdx_expires_in);

    wdx_token = token_result.token;
}

TEST_F(oauth2_backend_fixture, authenticate_wdx_token_success_cached)
{
    std::string          const exp_url          = origin + token_path;

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    // Generate WDx token by authenticate with username/password first
    std::string wdx_token;
    create_wdx_token_by_default_user_auth(backend, curl_mock, wdx_token, exp_url);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, default_user);
    EXPECT_FALSE(result.expired);
    EXPECT_FALSE(result.token.empty());
    EXPECT_EQ(   result.token, wdx_token);
    EXPECT_EQ(   remaining_time, default_wdx_expires_in);
}

TEST_F(oauth2_backend_fixture, authenticate_wdx_token_success_checked)
{
    std::string          const token            = default_access_token;
    uint32_t             const expire           = default_access_expires_in;
    std::string          const username         = default_user;
    std::string          const scopes           = "wda";
    std::string          const exp_url_1        = origin + token_path;
    std::string          const exp_url_2        = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "true"                     ","
                                                      "\"username\":\"" + username               + "\","
                                                      "\"scope\":\""    + scopes                 + "\","
                                                      "\"expires_in\":" + std::to_string(expire) +    ""
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    // Generate WDx token by authenticate with username/password first
    std::string wdx_token;
    create_wdx_token_by_default_user_auth(backend, curl_mock, wdx_token, exp_url_1);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(wdx_token, &remaining_time);
    ASSERT_TRUE( result.success);
    ASSERT_FALSE(result.expired);
    ASSERT_GE(   remaining_time, default_wdx_expires_in);

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url_2, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    advance_system_time(static_cast<int>(default_wdx_expires_in + 1));
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_FALSE(result.token.empty());
    EXPECT_NE(   result.token, wdx_token);
    EXPECT_GE(   remaining_time, default_wdx_expires_in);
}

TEST_F(oauth2_backend_fixture, authenticate_wdx_token_expired)
{
    std::string          const username         = default_user;
    std::string          const token            = default_access_token;
    std::string          const scopes           = "wda";
    std::string          const exp_url_1        = origin + token_path;
    std::string          const exp_url_2        = origin + introspection_path;
    std::string          const exp_query        = "";
    std::string          const result_content   = "{"
                                                      "\"active\":"       "false"
                                                  "}";
    curl::request_result const post_result      = { 200, default_result_content_type, result_content };

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    // Generate WDx token by authenticate with username/password first
    std::string wdx_token;
    create_wdx_token_by_default_user_auth(backend, curl_mock, wdx_token, exp_url_1);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(wdx_token, &remaining_time);
    ASSERT_TRUE( result.success);
    ASSERT_FALSE(result.expired);
    ASSERT_GE(   remaining_time, default_wdx_expires_in);

    advance_system_time(static_cast<int>(default_wdx_expires_in - 1));
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_GE(   remaining_time, 1);

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url_2, exp_query, default_post_content_type, testing::_, testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result;
            }
        )));

    advance_system_time(2);
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE (result.expired);
    EXPECT_EQ(   remaining_time, 0);

    advance_system_time(100);
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE (result.expired);
    EXPECT_EQ(   remaining_time, 0);
}

TEST_F(oauth2_backend_fixture, authenticate_wdx_token_expired_refresh_success)
{
    std::string          const username         = default_user;
    std::string          const token            = default_access_token;
    std::string          const scopes           = "wda";
    std::string          const exp_url_1        = origin + token_path;
    std::string          const exp_url_2        = origin + introspection_path;
    std::string          const exp_query_1      = "";
    std::string          const result_content_1 = "{"
                                                      "\"active\":"       "false"
                                                  "}";
    curl::request_result const post_result_1    = { 200, default_result_content_type, result_content_1 };
    std::string          const result_content_2 = "{"
                                                      "\"active\":"       "true"                     ","
                                                      "\"username\":\"" + username               + "\","
                                                      "\"scope\":\""    + scopes                 + "\","
                                                      "\"expires_in\":" + std::to_string(default_access_expires_in) +    ""
                                                  "}";
    curl::request_result const post_result_2    = { 200, default_result_content_type, result_content_2 };


    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    // Generate WDx token by authenticate with username/password first
    std::string wdx_token;
    create_wdx_token_by_default_user_auth(backend, curl_mock, wdx_token, exp_url_1, true);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(wdx_token, &remaining_time);
    ASSERT_TRUE( result.success);
    ASSERT_FALSE(result.expired);
    ASSERT_EQ(   remaining_time, default_wdx_expires_in);

    advance_system_time(static_cast<int>(default_wdx_expires_in - 1));
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_GE(   remaining_time, 1);

    // Expect one token verification via cURL which will fail and trigger a refresh
    EXPECT_CALL(curl_mock, post_data(exp_url_2, exp_query_1, default_post_content_type, testing::HasSubstr("token=" + token), testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(post_result_1));

    std::string          const refresh_token    = default_refresh_token;
    std::string          const new_token        = "someNewAccessToken123";
    std::string          const exp_query_3      = "";
    std::string          const result_content_3 = "{"
                                                      "\"token_type\":\""       "Bearer"                                    "\","
                                                      "\"access_token\":\""   + new_token                                 + "\","
                                                      "\"password_expired\":"   "false"                                       ","
                                                      "\"expires_in\":"       + std::to_string(default_access_expires_in) +    ""
                                                  "}";
    curl::request_result const post_result_3    = { 200, default_result_content_type, result_content_3 };

    // Expect refresh grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url_1, exp_query_3, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result_3, refresh_token](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="      "refresh_token")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("refresh_token=" + refresh_token))   << "Content: " + content;
            return post_result_3;
        })));

    // Expect another token verification via cURL which will succeed
    EXPECT_CALL(curl_mock, post_data(exp_url_2, exp_query_1, default_post_content_type, testing::HasSubstr("token=" + new_token), testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(post_result_2));

    advance_system_time(2);
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_FALSE(result.token.empty());
    EXPECT_NE(   result.token, wdx_token);
    EXPECT_EQ(   remaining_time, default_wdx_expires_in);
}

TEST_F(oauth2_backend_fixture, authenticate_wdx_token_expired_refresh_expired_or_invalid)
{
    std::string          const username         = default_user;
    std::string          const token            = default_access_token;
    std::string          const scopes           = "wda";
    std::string          const exp_url_1        = origin + token_path;
    std::string          const exp_url_2        = origin + introspection_path;
    std::string          const exp_query_1      = "";
    std::string          const result_content_1 = "{"
                                                      "\"active\":"       "false"
                                                  "}";
    curl::request_result const post_result_1    = { 200, default_result_content_type, result_content_1 };

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    // Generate WDx token by authenticate with username/password first
    std::string wdx_token;
    create_wdx_token_by_default_user_auth(backend, curl_mock, wdx_token, exp_url_1, true);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(wdx_token, &remaining_time);
    ASSERT_TRUE( result.success);
    ASSERT_FALSE(result.expired);
    ASSERT_EQ(   remaining_time, default_wdx_expires_in);

    advance_system_time(static_cast<int>(default_wdx_expires_in - 1));
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_TRUE( result.success);
    EXPECT_EQ(   result.user_name, username);
    EXPECT_FALSE(result.expired);
    EXPECT_GE(   remaining_time, 1);

    // Expect token verification via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url_2, exp_query_1, default_post_content_type, testing::_, testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArg<3>(Invoke(
            [post_result_1, token](std::string const content){
                EXPECT_NE(std::string::npos, content.find("token=" + token)) << "Content: " + content;
                return post_result_1;
            }
        )));

    std::string          const refresh_token    = default_refresh_token;
    std::string          const err_description  = "Some error description";
    std::string          const err_uri          = "http://some-error-page.de/description.html";
    std::string          const err_type         = "invalid_grant";
    std::string          const exp_query_2      = "";
    std::string          const result_content_2 = "{"
                                                      "\"error_description\":\"" + err_description + "\","
                                                      "\"error_uri\":\""         + err_uri         + "\","
                                                      "\"error\":\""             + err_type        + "\""
                                                  "}";
    curl::request_result const post_result_2    = { 400, default_result_content_type, result_content_2 };

    // Expect refresh grant via cURL
    EXPECT_CALL(curl_mock, post_data(exp_url_1, exp_query_2, default_post_content_type, testing::_, testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<3>(Invoke([post_result_2, refresh_token](std::string const content){
            EXPECT_NE(std::string::npos, content.find("grant_type="      "refresh_token")) << "Content: " + content;
            EXPECT_NE(std::string::npos, content.find("refresh_token=" + refresh_token))   << "Content: " + content;
            return post_result_2;
        })));

    advance_system_time(2);
    result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, 0);
}

TEST_F(oauth2_backend_fixture, authenticate_fail_on_random_wdx_token)
{
    std::string          const wdx_token        = "wdx$SomeRandomValue42";

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(wdx_token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, 0);
}

TEST_F(oauth2_backend_fixture, authenticate_fail_on_empty_wdx_token)
{
    std::string          const empty_token      = "wdx$";

    oauth2_backend backend(settings_store_mock, broken_token_slowdown);

    uint32_t remaining_time = 0;
    auto result = backend.authenticate(empty_token, &remaining_time);
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.expired);
    EXPECT_TRUE( result.token.empty());
    EXPECT_EQ(   remaining_time, 0);
}


//---- End of source file ------------------------------------------------------
