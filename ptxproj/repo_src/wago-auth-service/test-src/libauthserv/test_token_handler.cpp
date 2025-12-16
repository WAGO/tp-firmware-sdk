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
///  \brief    Test common token handler.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "mocks/mock_system_clock.hpp"
#include "mocks/mock_settings_store.hpp"
#include "token_handler.hpp"

#include <wc/assertion.h>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::token_handler;
using ::testing::AnyNumber;
using ::testing::WithArgs;
using ::testing::WithoutArgs;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {
constexpr uint32_t const key_lifetime_s       = 100;
std::string        const key_liftime_s_string = std::to_string(key_lifetime_s);
constexpr char     const default_payload[] = "test payload";
}

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(token_handler, construct_delete)
{
    mock_settings_store settings_store_mock;
    settings_store_mock.set_default_expectations();
    settings_store_mock.expect_global_setting_read(wago::authserv::settings_store_i::refresh_token_lifetime, key_liftime_s_string);

    token_handler token_handler(settings_store_mock);
}

class token_handler_fixture: public ::testing::Test
{
private:
    timespec cur_time;

protected:
    mock_system_clock   clock_mock;
    mock_settings_store settings_store_mock;

    token_handler_fixture() = default;
    ~token_handler_fixture() override = default;

    void SetUp() override
    {
        cur_time = { 0, 0 };
        clock_mock.set_default_expectations();
        EXPECT_CALL(clock_mock, gettime(CLOCK_MONOTONIC, ::testing::NotNull()))
            .Times(AnyNumber())
            .WillRepeatedly(WithArgs<0, 1>(Invoke(this, &token_handler_fixture::clock_gettime)));
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

TEST_F(token_handler_fixture, generate_and_validate_token)
{
    token_handler token_handler(settings_store_mock);

    auto token = token_handler.build_token(default_payload);
    EXPECT_GT(token.length(), 0);

    auto result = token_handler.get_token_payload(token);
    EXPECT_STREQ(default_payload, result.c_str());
}

TEST_F(token_handler_fixture, generate_and_validate_token_with_key_rotation)
{
    token_handler token_handler(settings_store_mock);

    // Without rotation token with same payload should be equal
    auto token_1 = token_handler.build_token(default_payload);
    ASSERT_GT(token_1.length(), 0);

    auto result_1 = token_handler.get_token_payload(token_1);
    ASSERT_STREQ(default_payload, result_1.c_str());

    // Key should be rotated but code still valid
    advance_system_time(static_cast<int>(key_lifetime_s + 1));
    auto result_2 = token_handler.get_token_payload(token_1);
    EXPECT_STREQ(default_payload, result_2.c_str());

    // Key should be rotated again, code can't be verified (only one previous key kept for verification)
    advance_system_time(static_cast<int>(key_lifetime_s + 1));
    EXPECT_THROW(token_handler.get_token_payload(token_1), std::runtime_error);
}

TEST_F(token_handler_fixture, regenerate_keys)
{
    token_handler token_handler(settings_store_mock);

    // Without rotation token with same payload should be equal
    auto token_1 = token_handler.build_token(default_payload);
    ASSERT_GT(token_1.length(), 0);

    auto result_1 = token_handler.get_token_payload(token_1);
    ASSERT_STREQ(default_payload, result_1.c_str());

    // invalidate all tokens by regenerating keys
    token_handler.regenerate_key();
    EXPECT_THROW(token_handler.get_token_payload(token_1), std::runtime_error);
}


//---- End of source file ------------------------------------------------------
