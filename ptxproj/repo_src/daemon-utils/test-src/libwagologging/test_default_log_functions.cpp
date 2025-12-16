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
///  \brief    Test WAGO logging default log functions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"
#include "wago/logging/log.hpp"
#include "mocks/mock_syslog.hpp"

#ifdef SYSTEMD_INTEGRATION
#include "mocks/mock_journal.hpp"
#endif // SYSTEMD_INTEGRATION

#include <gtest/gtest.h>
#include <syslog.h>
#include <cstring>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::logging::log_function_t;
using wago::logging::sal::syslog;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char const default_log_message[] = "Default test log message";

namespace wago {
namespace logging {
namespace sal {

syslog       *syslog::instance     = nullptr;
int    const  syslog::log_debug    = 1;
int    const  syslog::log_info     = 2;
int    const  syslog::log_notice   = 3;
int    const  syslog::log_warning  = 4;
int    const  syslog::log_error    = 5;
int    const  syslog::log_critical = 6;

#ifdef SYSTEMD_INTEGRATION
journal       *journal::instance     = nullptr;
int     const  journal::log_debug    = 7;
int     const  journal::log_info     = 6;
int     const  journal::log_notice   = 5;
int     const  journal::log_warning  = 4;
int     const  journal::log_error    = 3;
int     const  journal::log_critical = 2;
int     const  journal::log_alert    = 1;
#endif // SYSTEMD_INTEGRATION

} // Namespace sal
} // Namespace logging
} // Namespace wago

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(default_log_functions, stdout_is_log_function_type)
{
    volatile log_function_t log_function = wago::logging::log_stdout;
    (void)log_function;
}

TEST(default_log_functions, stderr_is_log_function_type)
{
    volatile log_function_t log_function = wago::logging::log_stderr;
    (void)log_function;
}

TEST(default_log_functions, syslog_is_log_function_type)
{
    volatile log_function_t log_function = wago::logging::log_syslog;
    (void)log_function;
}

TEST(default_log_functions, journal_is_log_function_type)
{
    volatile log_function_t log_function = wago::logging::log_journal;
    (void)log_function;
}

TEST(default_log_functions, tag_debug)
{
    ::testing::internal::CaptureStdout();

    // Test settings
    log_level_t const log_level          = log_level_t::debug;
    char        const expected_log_tag[] = "DEBUG";

    // Test
    {
        wago::logging::log_stdout(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(expected_log_tag) != std::string::npos);
}

TEST(default_log_functions, tag_info)
{
    ::testing::internal::CaptureStdout();

    // Test settings
    log_level_t const log_level          = log_level_t::info;
    char        const expected_log_tag[] = "INFO";

    // Test
    {
        wago::logging::log_stdout(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(expected_log_tag) != std::string::npos);
}

TEST(default_log_functions, tag_notice)
{
    ::testing::internal::CaptureStdout();

    // Test settings
    log_level_t const log_level          = log_level_t::notice;
    char        const expected_log_tag[] = "NOTE";

    // Test
    {
        wago::logging::log_stdout(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(expected_log_tag) != std::string::npos);
}

TEST(default_log_functions, tag_warning)
{
    ::testing::internal::CaptureStdout();

    // Test settings
    log_level_t const log_level          = log_level_t::warning;
    char        const expected_log_tag[] = "WARN";

    // Test
    {
        wago::logging::log_stdout(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(expected_log_tag) != std::string::npos);
}

TEST(default_log_functions, tag_error)
{
    ::testing::internal::CaptureStdout();

    // Test settings
    log_level_t const log_level          = log_level_t::error;
    char        const expected_log_tag[] = "ERROR";

    // Test
    {
        wago::logging::log_stdout(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(expected_log_tag) != std::string::npos);
}

TEST(default_log_functions, tag_fatal)
{
    ::testing::internal::CaptureStdout();

    // Test settings
    log_level_t const log_level          = log_level_t::fatal;
    char        const expected_log_tag[] = "FATAL";

    // Test
    {
        wago::logging::log_stdout(log_level, default_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(expected_log_tag) != std::string::npos);
}

TEST(default_log_functions, outstream_print)
{
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    // Test settings
    char        const test_log_message[] = "Test log message";
    log_level_t const log_level          = log_level_t::debug;

    // Test
    {
        wago::logging::log_outstream(log_level, test_log_message, std::cout);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(test_log_message) != std::string::npos);
    EXPECT_TRUE(::testing::internal::GetCapturedStderr().empty());
}

TEST(default_log_functions, stdout_print)
{
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    // Test settings
    char        const test_log_message[] = "Test log message";
    log_level_t const log_level          = log_level_t::debug;

    // Test
    {
        wago::logging::log_stdout(log_level, test_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().find(test_log_message) != std::string::npos);
    EXPECT_TRUE(::testing::internal::GetCapturedStderr().empty());
}

TEST(default_log_functions, stderr_print)
{
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    // Test settings
    char        const test_log_message[] = "Test log message";
    log_level_t const log_level          = log_level_t::debug;

    // Test
    {
        wago::logging::log_stderr(log_level, test_log_message);
    }

    // Check output
    EXPECT_TRUE(::testing::internal::GetCapturedStderr().find(test_log_message) != std::string::npos);
    EXPECT_TRUE(::testing::internal::GetCapturedStdout().empty());
}

TEST(default_log_functions, syslog_print_debug)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_log_message[]     = "Test log message";
    log_level_t const log_level              = log_level_t::debug;

    // Set expectations
    syslog_mock.set_default_expectations();
    int const log_priority = LOG_DAEMON | wago::logging::sal::syslog::log_debug;
    EXPECT_CALL(syslog_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_syslog(log_level, test_log_message);
    }
}

TEST(default_log_functions, syslog_print_info)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_log_message[]     = "Test log message";
    log_level_t const log_level              = log_level_t::info;

    // Set expectations
    syslog_mock.set_default_expectations();
    int const log_priority = LOG_DAEMON | wago::logging::sal::syslog::log_info;
    EXPECT_CALL(syslog_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_syslog(log_level, test_log_message);
    }
}

TEST(default_log_functions, syslog_print_notice)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_log_message[]     = "Test log message";
    log_level_t const log_level              = log_level_t::notice;

    // Set expectations
    syslog_mock.set_default_expectations();
    int const log_priority = LOG_DAEMON | wago::logging::sal::syslog::log_notice;
    EXPECT_CALL(syslog_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_syslog(log_level, test_log_message);
    }
}

TEST(default_log_functions, syslog_print_warning)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_log_message[]     = "Test log message";
    log_level_t const log_level              = log_level_t::warning;

    // Set expectations
    syslog_mock.set_default_expectations();
    int const log_priority = LOG_DAEMON | wago::logging::sal::syslog::log_warning;
    EXPECT_CALL(syslog_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_syslog(log_level, test_log_message);
    }
}

TEST(default_log_functions, syslog_print_error)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_log_message[]     = "Test log message";
    log_level_t const log_level              = log_level_t::error;

    // Set expectations
    syslog_mock.set_default_expectations();
    int const log_priority = LOG_DAEMON | wago::logging::sal::syslog::log_error;
    EXPECT_CALL(syslog_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_syslog(log_level, test_log_message);
    }
}

TEST(default_log_functions, syslog_print_fatal)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_log_message[]     = "Test log message";
    log_level_t const log_level              = log_level_t::fatal;

    // Set expectations
    syslog_mock.set_default_expectations();
    int const log_priority = LOG_DAEMON | wago::logging::sal::syslog::log_critical;
    EXPECT_CALL(syslog_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_syslog(log_level, test_log_message);
    }
}

#ifdef SYSTEMD_INTEGRATION
TEST(default_log_functions, journal_print_debug)
{
    // Test settings
    mock_journal       journal_mock;
    char         const test_log_message[] = "Test log message";
    log_level_t  const log_level          = log_level_t::debug;

    // Set expectations
    journal_mock.set_default_expectations();
    int const log_priority = wago::logging::sal::journal::log_debug;
    EXPECT_CALL(journal_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_journal(log_level, test_log_message);
    }
}

TEST(default_log_functions, journal_print_info)
{
    // Test settings
    mock_journal       journal_mock;
    char         const test_log_message[] = "Test log message";
    log_level_t  const log_level          = log_level_t::info;

    // Set expectations
    journal_mock.set_default_expectations();
    int const log_priority = wago::logging::sal::journal::log_info;
    EXPECT_CALL(journal_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_journal(log_level, test_log_message);
    }
}

TEST(default_log_functions, journal_print_notice)
{
    // Test settings
    mock_journal       journal_mock;
    char         const test_log_message[] = "Test log message";
    log_level_t  const log_level          = log_level_t::notice;

    // Set expectations
    journal_mock.set_default_expectations();
    int const log_priority = wago::logging::sal::journal::log_notice;
    EXPECT_CALL(journal_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_journal(log_level, test_log_message);
    }
}

TEST(default_log_functions, journal_print_warning)
{
    // Test settings
    mock_journal       journal_mock;
    char         const test_log_message[] = "Test log message";
    log_level_t  const log_level          = log_level_t::warning;

    // Set expectations
    journal_mock.set_default_expectations();
    int const log_priority = wago::logging::sal::journal::log_warning;
    EXPECT_CALL(journal_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_journal(log_level, test_log_message);
    }
}

TEST(default_log_functions, journal_print_error)
{
    // Test settings
    mock_journal       journal_mock;
    char         const test_log_message[] = "Test log message";
    log_level_t  const log_level          = log_level_t::error;

    // Set expectations
    journal_mock.set_default_expectations();
    int const log_priority = wago::logging::sal::journal::log_error;
    EXPECT_CALL(journal_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_journal(log_level, test_log_message);
    }
}

TEST(default_log_functions, journal_print_fatal)
{
    // Test settings
    mock_journal       journal_mock;
    char         const test_log_message[]     = "Test log message";
    log_level_t  const log_level              = log_level_t::fatal;

    // Set expectations
    journal_mock.set_default_expectations();
    int const log_priority = wago::logging::sal::journal::log_critical;
    EXPECT_CALL(journal_mock, log(log_priority, test_log_message))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::log_journal(log_level, test_log_message);
    }
}
#endif // SYSTEMD_INTEGRATION

TEST(management_functions, init)
{
    // Test settings
    mock_syslog syslog_mock;
    char        const test_service_name[]      = "Test log message";
    int         const expected_syslog_option   = LOG_PID;
    int         const expected_syslog_facility = LOG_DAEMON;

    // Set expectations
    syslog_mock.set_default_expectations();
    EXPECT_CALL(syslog_mock, openlog(test_service_name, expected_syslog_option, expected_syslog_facility))
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::init(test_service_name);
    }
}

TEST(management_functions, close)
{
    // Test settings
    mock_syslog syslog_mock;

    // Set expectations
    syslog_mock.set_default_expectations();
    EXPECT_CALL(syslog_mock, closelog())
        .Times(::testing::Exactly(1));

    // Test
    {
        wago::logging::close();
    }
}


//---- End of source file ------------------------------------------------------
