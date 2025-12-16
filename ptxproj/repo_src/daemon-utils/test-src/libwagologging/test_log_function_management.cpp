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
///  \brief    Test WAGO logging log function management.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/logging/log.hpp"
#include "mocks/mock_syslog.hpp"

#include <gtest/gtest.h>
#include <sys/syslog.h>

#include <cstring>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::logging::log_function_t;
using wago::logging::log_function_simple_t;
using std::string;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char const default_log_message[] = "Default test log message";

log_level_t         last_log_level;
char        const * last_message;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
static void test_log_function(log_level_t const         log_level,
                              char        const * const message)
{
    last_log_level = log_level;
    last_message   = message;
}

static void test_log_function_simple(char const * const message)
{
    last_message   = message;
}

TEST(log_function_management, set_log_function)
{
    log_function_t log_function = test_log_function;
    wago::logging::set_log_function(log_function);
}

TEST(log_function_management, set_log_function_simple)
{
    log_function_simple_t log_function_simple = test_log_function_simple;
    wago::logging::set_log_function(log_function_simple);
}

TEST(log_function_management, log_call_without_function)
{
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    // Test settings
    log_level_t const log_level = log_level_t::debug;

    // Test
    {
        wago::logging::set_log_function(static_cast<log_function_t>(nullptr));
        wc_log_output(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().empty());
    EXPECT_TRUE(::testing::internal::GetCapturedStderr().empty());
}

TEST(log_function_management, log_call_with_function)
{
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    // Test settings
    log_level_t const log_level   = log_level_t::debug;
    string      const log_message = default_log_message + std::to_string(__LINE__);

    // Test
    {
        wago::logging::set_log_function(test_log_function);
        wc_log_output(log_level, log_message.c_str());
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().empty());
    EXPECT_TRUE(::testing::internal::GetCapturedStderr().empty());
    EXPECT_EQ(log_level, last_log_level);
    EXPECT_STREQ(log_message.c_str(), last_message);
}

TEST(log_function_management, log_call_with_simple_function)
{
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    // Test settings
    log_level_t const log_level   = log_level_t::debug;
    string      const log_message = default_log_message + std::to_string(__LINE__);

    // Test
    {
        wago::logging::set_log_function(test_log_function_simple);
        wc_log_output(log_level, log_message.c_str());
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().empty());
    EXPECT_TRUE(::testing::internal::GetCapturedStderr().empty());
    EXPECT_STREQ(log_message.c_str(), last_message);
}

TEST(log_function_management, set_log_function_reset)
{
    // Test settings
    log_level_t const log_level   = log_level_t::debug;
    string      const log_message = default_log_message + std::to_string(__LINE__);
    char        const dummy[]     = "Dummy Value";

    // Test
    {
        last_message = dummy;
        wago::logging::set_log_function(test_log_function);
        wago::logging::set_log_function(static_cast<log_function_simple_t>(nullptr));
        wc_log_output(log_level, default_log_message);
    }

    // Check output
    EXPECT_STREQ(dummy, last_message);
}

TEST(log_function_management, set_log_function_simple_reset)
{
    // Test settings
    log_level_t const log_level   = log_level_t::debug;
    string      const log_message = default_log_message + std::to_string(__LINE__);
    char        const dummy[]     = "Dummy Value";

    // Test
    {
        last_message = dummy;
        wago::logging::set_log_function(test_log_function_simple);
        wago::logging::set_log_function(static_cast<log_function_t>(nullptr));
        wc_log_output(log_level, default_log_message);
    }

    // Check output
    EXPECT_STREQ(dummy, last_message);
}

TEST(log_function_management, set_get_log_level)
{
    // Test settings
    log_level_t const log_level = log_level_t::fatal;

    // Test
    {
        wago::logging::set_log_level(log_level);
    }

    // Check log level
    EXPECT_EQ(log_level, wago::logging::get_log_level());
    EXPECT_EQ(log_level, wc_get_log_level());
}

TEST(log_function_management, syslog_integration)
{
    // Test settings
    char        const service_name[]   = "Unit Test";
    log_level_t const log_level        = log_level_t::fatal;
    mock_syslog syslog_mock;
    EXPECT_CALL(syslog_mock, openlog(service_name, ::testing::_, ::testing::_))
        .Times(::testing::Exactly(1));
    EXPECT_CALL(syslog_mock, closelog())
        .Times(::testing::Exactly(1));
    EXPECT_CALL(syslog_mock, log(LOG_DAEMON | wago::logging::sal::syslog::log_critical, default_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::init(service_name);
        wago::logging::log_syslog(log_level, default_log_message);
        wago::logging::close();
    }
}


//---- End of source file ------------------------------------------------------
