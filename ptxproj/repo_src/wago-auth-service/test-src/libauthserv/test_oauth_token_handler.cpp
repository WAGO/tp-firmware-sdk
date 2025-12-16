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
///  \brief    Test OAuth token handler.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "mocks/mock_system_clock.hpp"
#include "mocks/mock_settings_store.hpp"
#include "oauth_token_handler.hpp"

#include <wc/assertion.h>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::oauth_token_handler;
using ::testing::AnyNumber;
using ::testing::WithArgs;
using ::testing::WithoutArgs;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {
constexpr uint32_t const key_lifetime_s                   = 600;
constexpr uint32_t const default_code_lifetime_s          = key_lifetime_s / 60;
constexpr uint32_t const default_access_token_lifetime_s  = key_lifetime_s / 10;
constexpr uint32_t const default_refresh_token_lifetime_s = key_lifetime_s;
std::string        const key_liftime_s_string             = std::to_string(key_lifetime_s);
WC_STATIC_ASSERT(default_code_lifetime_s          > 1);
WC_STATIC_ASSERT(default_access_token_lifetime_s  > 1);
WC_STATIC_ASSERT(default_refresh_token_lifetime_s > 1);
}

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(oauth_token_handler, construct_delete)
{
    mock_settings_store settings_store_mock;
    settings_store_mock.set_default_expectations();
    settings_store_mock.expect_global_setting_read(wago::authserv::settings_store_i::refresh_token_lifetime, key_liftime_s_string);

    oauth_token_handler token_handler(settings_store_mock);
}

class oauth_token_handler_fixture: public ::testing::Test
{
private:
    timespec cur_time;

protected:
    mock_system_clock   clock_mock;
    mock_settings_store settings_store_mock;

    oauth_token_handler_fixture() = default;
    ~oauth_token_handler_fixture() override = default;

    void SetUp() override
    {
        cur_time = { 0, 0 };
        clock_mock.set_default_expectations();
        EXPECT_CALL(clock_mock, gettime(CLOCK_MONOTONIC, ::testing::NotNull()))
            .Times(AnyNumber())
            .WillRepeatedly(WithArgs<0, 1>(Invoke(this, &oauth_token_handler_fixture::clock_gettime)));
        settings_store_mock.set_default_expectations();
        settings_store_mock.expect_global_setting_read(wago::authserv::settings_store_i::refresh_token_lifetime, key_liftime_s_string);
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

TEST_F(oauth_token_handler_fixture, generate_and_validate_auth_code)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const code_verifier = "test-verifier";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    uint32_t const lifetime_1_s = 0;
    auto code_1 = token_handler.generate_auth_code(lifetime_1_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(code_1.length(), 0);

    uint32_t const lifetime_2_s = default_code_lifetime_s / 2;
    auto code_2 = token_handler.generate_auth_code(lifetime_2_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(code_2.length(), 0);

    uint32_t const lifetime_3_s = default_code_lifetime_s;
    auto code_3 = token_handler.generate_auth_code(lifetime_3_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(code_3.length(), 0);

    auto result_2 = token_handler.validate_auth_code(code_2, code_verifier);
    EXPECT_TRUE(            result_2.valid);
    EXPECT_FALSE(           result_2.expired);
    EXPECT_EQ(lifetime_2_s, result_2.remaining_time_s);
    EXPECT_EQ(client_id,    result_2.client_id);
    EXPECT_EQ(scopes,       result_2.scopes);

    auto result_3 = token_handler.validate_auth_code(code_3, code_verifier);
    EXPECT_TRUE(            result_3.valid);
    EXPECT_FALSE(           result_3.expired);
    EXPECT_EQ(lifetime_3_s, result_3.remaining_time_s);
    EXPECT_EQ(client_id,    result_3.client_id);
    EXPECT_EQ(scopes,       result_3.scopes);
}

TEST_F(oauth_token_handler_fixture, generate_and_validate_access_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id = "test_client";
    std::string const scopes    = "scope1 scope2";
    std::string const user_name     = "testuser";

    uint32_t const lifetime_1_s = 0;
    auto token_1 = token_handler.generate_access_token(lifetime_1_s, client_id, scopes, user_name);
    EXPECT_GT(token_1.length(), 0);

    uint32_t const lifetime_2_s = default_access_token_lifetime_s / 2;
    auto token_2 = token_handler.generate_access_token(lifetime_2_s, client_id, scopes, user_name);
    EXPECT_GT(token_2.length(), 0);

    uint32_t const lifetime_3_s = default_access_token_lifetime_s;
    auto token_3 = token_handler.generate_access_token(lifetime_3_s, client_id, scopes, user_name);
    EXPECT_GT(token_3.length(), 0);

    auto result_2 = token_handler.validate_access_token(token_2);
    EXPECT_TRUE(            result_2.valid);
    EXPECT_FALSE(           result_2.expired);
    EXPECT_EQ(lifetime_2_s, result_2.remaining_time_s);
    EXPECT_EQ(client_id,    result_2.client_id);
    EXPECT_EQ(scopes,       result_2.scopes);

    auto result_3 = token_handler.validate_access_token(token_3);
    EXPECT_TRUE(            result_3.valid);
    EXPECT_FALSE(           result_3.expired);
    EXPECT_EQ(lifetime_3_s, result_3.remaining_time_s);
    EXPECT_EQ(client_id,    result_3.client_id);
    EXPECT_EQ(scopes,       result_3.scopes);
}

TEST_F(oauth_token_handler_fixture, generate_and_validate_refresh_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id = "test_client";
    std::string const scopes    = "scope1 scope2";
    std::string const user_name = "testuser";

    uint32_t const lifetime_1_s = 0;
    auto token_1 = token_handler.generate_refresh_token(lifetime_1_s, client_id, scopes, user_name);
    EXPECT_GT(token_1.length(), 0);

    uint32_t const lifetime_2_s = default_refresh_token_lifetime_s / 2;
    auto token_2 = token_handler.generate_refresh_token(lifetime_2_s, client_id, scopes, user_name);
    EXPECT_GT(token_2.length(), 0);

    uint32_t const lifetime_3_s = default_refresh_token_lifetime_s;
    auto token_3 = token_handler.generate_refresh_token(lifetime_3_s, client_id, scopes, user_name);
    EXPECT_GT(token_3.length(), 0);

    auto result_2 = token_handler.validate_refresh_token(token_2);
    EXPECT_TRUE(            result_2.valid);
    EXPECT_FALSE(           result_2.expired);
    EXPECT_EQ(lifetime_2_s, result_2.remaining_time_s);
    EXPECT_EQ(client_id,    result_2.client_id);
    EXPECT_EQ(scopes,       result_2.scopes);

    auto result_3 = token_handler.validate_refresh_token(token_3);
    EXPECT_TRUE(            result_3.valid);
    EXPECT_FALSE(           result_3.expired);
    EXPECT_EQ(lifetime_3_s, result_3.remaining_time_s);
    EXPECT_EQ(client_id,    result_3.client_id);
    EXPECT_EQ(scopes,       result_3.scopes);
}

TEST_F(oauth_token_handler_fixture, auth_code_expired)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const code_verifier = "test-verifier";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    uint32_t const lifetime_s = default_code_lifetime_s;
    auto token = token_handler.generate_auth_code(lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);
    auto result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_TRUE(          result.valid);
    EXPECT_FALSE(         result.expired);
    EXPECT_EQ(lifetime_s, result.remaining_time_s);
    EXPECT_EQ(client_id,  result.client_id);
    EXPECT_EQ(scopes,     result.scopes);

    token = token_handler.generate_auth_code(lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);
    advance_system_time(static_cast<int>(lifetime_s));
    result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_TRUE(         result.valid);
    EXPECT_FALSE(        result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    token = token_handler.generate_auth_code(lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);
    advance_system_time(lifetime_s + 1);
    result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    token = token_handler.generate_auth_code(lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);
    advance_system_time(lifetime_s + 100);
    result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);
}

TEST_F(oauth_token_handler_fixture, auth_code_double_use_fails)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const code_verifier = "test-verifier";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    uint32_t const lifetime_s = default_code_lifetime_s;
    auto token = token_handler.generate_auth_code(lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);
    auto result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_TRUE(          result.valid);
    EXPECT_FALSE(         result.expired);
    EXPECT_EQ(lifetime_s, result.remaining_time_s);
    EXPECT_EQ(client_id,  result.client_id);
    EXPECT_EQ(scopes,     result.scopes);
    // second use fails
    result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_FALSE(         result.valid);
}

TEST_F(oauth_token_handler_fixture, access_token_expired)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id = "test_client";
    std::string const scopes    = "scope1 scope2";
    std::string const user_name = "testuser";

    uint32_t const lifetime_s = default_access_token_lifetime_s;
    auto token = token_handler.generate_access_token(lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token.length(), 0);
    auto result = token_handler.validate_access_token(token);
    EXPECT_TRUE(          result.valid);
    EXPECT_FALSE(         result.expired);
    EXPECT_EQ(lifetime_s, result.remaining_time_s);
    EXPECT_EQ(client_id,  result.client_id);
    EXPECT_EQ(scopes,     result.scopes);

    advance_system_time(static_cast<int>(lifetime_s));
    result = token_handler.validate_access_token(token);
    EXPECT_TRUE(         result.valid);
    EXPECT_FALSE(        result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    advance_system_time(1);
    result = token_handler.validate_access_token(token);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    advance_system_time(100);
    result = token_handler.validate_access_token(token);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);
}

TEST_F(oauth_token_handler_fixture, refresh_token_expired)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id = "test_client";
    std::string const scopes    = "scope1 scope2";
    std::string const user_name = "testuser";

    uint32_t const lifetime_s = default_refresh_token_lifetime_s;
    auto token = token_handler.generate_refresh_token(lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token.length(), 0);
    auto result = token_handler.validate_refresh_token(token);
    EXPECT_TRUE(          result.valid);
    EXPECT_FALSE(         result.expired);
    EXPECT_EQ(lifetime_s, result.remaining_time_s);
    EXPECT_EQ(client_id,  result.client_id);
    EXPECT_EQ(scopes,     result.scopes);

    advance_system_time(static_cast<int>(lifetime_s));
    result = token_handler.validate_refresh_token(token);
    EXPECT_TRUE(         result.valid);
    EXPECT_FALSE(        result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    advance_system_time(1);
    result = token_handler.validate_refresh_token(token);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    advance_system_time(100);
    result = token_handler.validate_refresh_token(token);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);
}

TEST_F(oauth_token_handler_fixture, fail_on_falsy_auth_code)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const code_verifier = "test-verifier";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    std::string const random_token  = "SomeRandomValue42";
    std::string const empty_token   = "";

    // generate a token that expires immediately
    std::string const expired_token = token_handler.generate_auth_code(0, client_id, scopes, user_name, challenge);
    EXPECT_GT(expired_token.length(), 0);
    advance_system_time(1);

    auto result = token_handler.validate_auth_code(expired_token, code_verifier);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    result = token_handler.validate_auth_code(random_token, code_verifier);
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_auth_code(empty_token, code_verifier);
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_auth_code(empty_token, "wrong-verifier");
    EXPECT_FALSE(result.valid);
}

TEST_F(oauth_token_handler_fixture, fail_on_validate_auth_code_as_other_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    std::string const token = token_handler.generate_auth_code(default_code_lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);

    auto result = token_handler.validate_access_token(token);
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_refresh_token(token);
    EXPECT_FALSE(result.valid);
}

TEST_F(oauth_token_handler_fixture, fail_on_falsy_access_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const random_token  = "SomeRandomValue42";
    std::string const empty_token   = "";

    // generate a token that expires immediately
    std::string const expired_token = token_handler.generate_access_token(0, client_id, scopes, user_name);
    EXPECT_GT(expired_token.length(), 0);
    advance_system_time(1);

    auto result = token_handler.validate_access_token(expired_token);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    result = token_handler.validate_access_token(random_token);
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_access_token(empty_token);
    EXPECT_FALSE(result.valid);
}

TEST_F(oauth_token_handler_fixture, fail_on_validate_access_as_other_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";

    std::string const token = token_handler.generate_access_token(default_access_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token.length(), 0);

    auto result = token_handler.validate_auth_code(token, "test-verifier");
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_refresh_token(token);
    EXPECT_FALSE(result.valid);
}

TEST_F(oauth_token_handler_fixture, fail_on_falsy_refresh_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const random_token  = "SomeRandomValue42";
    std::string const empty_token   = "";

    // generate a token that expires immediately
    std::string const expired_token = token_handler.generate_refresh_token(0, client_id, scopes, user_name);
    EXPECT_GT(expired_token.length(), 0);
    advance_system_time(1);

    auto result = token_handler.validate_refresh_token(expired_token);
    EXPECT_TRUE(         result.valid);
    EXPECT_TRUE(         result.expired);
    EXPECT_EQ(0,         result.remaining_time_s);
    EXPECT_EQ(client_id, result.client_id);
    EXPECT_EQ(scopes,    result.scopes);

    result = token_handler.validate_refresh_token(random_token);
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_refresh_token(empty_token);
    EXPECT_FALSE(result.valid);
}

TEST_F(oauth_token_handler_fixture, fail_on_validate_refresh_as_other_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";

    std::string const token = token_handler.generate_refresh_token(default_refresh_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token.length(), 0);

    auto result = token_handler.validate_auth_code(token, "test-verifier");
    EXPECT_FALSE(result.valid);

    result = token_handler.validate_access_token(token);
    EXPECT_FALSE(result.valid);
}

TEST_F(oauth_token_handler_fixture, invalidate_auth_code)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const code_verifier = "test-verifier";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    std::string const token = token_handler.generate_auth_code(default_code_lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token.length(), 0);
    std::string const token2 = token_handler.generate_auth_code(default_code_lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(token2.length(), 0);

    // code validity can't be checked here as this would invalidate the code

    token_handler.revoke_token(token);

    // only second token stays valid
    auto result = token_handler.validate_auth_code(token, code_verifier);
    EXPECT_FALSE(result.valid);
    result = token_handler.validate_auth_code(token2, code_verifier);
    EXPECT_TRUE(result.valid);
}

TEST_F(oauth_token_handler_fixture, invalidate_access_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";

    std::string const token = token_handler.generate_access_token(default_refresh_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token.length(), 0);
    std::string const token2 = token_handler.generate_access_token(default_refresh_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token2.length(), 0);

    // all tokens are valid
    auto result = token_handler.validate_access_token(token);
    EXPECT_TRUE(result.valid);
    result = token_handler.validate_access_token(token2);
    EXPECT_TRUE(result.valid);

    token_handler.revoke_token(token);

    // only second token stays valid
    result = token_handler.validate_access_token(token);
    EXPECT_FALSE(result.valid);
    result = token_handler.validate_access_token(token2);
    EXPECT_TRUE(result.valid);
}

TEST_F(oauth_token_handler_fixture, invalidate_refresh_token)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";

    std::string const token = token_handler.generate_refresh_token(default_refresh_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token.length(), 0);
    std::string const token2 = token_handler.generate_refresh_token(default_refresh_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(token2.length(), 0);

    // all tokens are valid
    auto result = token_handler.validate_refresh_token(token);
    EXPECT_TRUE(result.valid);
    result = token_handler.validate_refresh_token(token2);
    EXPECT_TRUE(result.valid);

    token_handler.revoke_token(token);

    // only second token stays valid
    result = token_handler.validate_refresh_token(token);
    EXPECT_FALSE(result.valid);
    result = token_handler.validate_refresh_token(token2);
    EXPECT_TRUE(result.valid);
}

TEST_F(oauth_token_handler_fixture, invalidate_all_tokens)
{
    oauth_token_handler token_handler(settings_store_mock);
    std::string const client_id     = "test_client";
    std::string const scopes        = "scope1 scope2";
    std::string const user_name     = "testuser";
    std::string const code_verifier = "test-verifier";
    std::string const challenge     = "JBbiqONGWPaAmwXk_8bT6UnlPfrn65D32eZlJS-zGG0";

    std::string const auth_code = token_handler.generate_auth_code(default_code_lifetime_s, client_id, scopes, user_name, challenge);
    EXPECT_GT(auth_code.length(), 0);
    std::string const access_token = token_handler.generate_access_token(default_access_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(access_token.length(), 0);
    std::string const refresh_token = token_handler.generate_refresh_token(default_refresh_token_lifetime_s, client_id, scopes, user_name);
    EXPECT_GT(refresh_token.length(), 0);

    // all tokens are valid (auth_code isn't tested as this would invalidate the code)
    auto result = token_handler.validate_access_token(access_token);
    EXPECT_TRUE(result.valid);
    result = token_handler.validate_refresh_token(refresh_token);
    EXPECT_TRUE(result.valid);

    token_handler.revoke_all_tokens();

    // all tokens are invalid
    result = token_handler.validate_auth_code(auth_code, code_verifier);
    EXPECT_FALSE(result.valid);
    result = token_handler.validate_access_token(access_token);
    EXPECT_FALSE(result.valid);
    result = token_handler.validate_refresh_token(refresh_token);
    EXPECT_FALSE(result.valid);
}

//---- End of source file ------------------------------------------------------
