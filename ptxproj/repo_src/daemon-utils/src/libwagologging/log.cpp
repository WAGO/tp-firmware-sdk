//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO implementation of common log function.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/logging/log.hpp"
#include "system_abstraction.hpp"

#include <wc/assertion.h>
#include <wc/compiler.h>
#include <wc/structuring.h>

#include <syslog.h>
#include <sstream>
#include <atomic>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::logging::log_function_t;
using wago::logging::log_function_simple_t;
using std::atomic;

namespace wago {
namespace logging {

using std::stringstream;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static void log_without_level(log_level_t  const log_level,
                              char const * const message);

static char const * get_log_level_tag(log_level_t const  log_level);

} // Namespace logging
} // Namespace wago

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static atomic<log_level_t>           current_log_level(off);
static atomic<log_function_t>        current_log_function(nullptr);
static atomic<log_function_simple_t> current_log_function_simple(nullptr);
static auto const                    default_syslog_facility(LOG_DAEMON);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
GNUC_ATTRIBUTE(weak) void wc_log_output(log_level_t  const log_level,
                                        char const * const message) noexcept
{
    log_function_t const log_function = current_log_function;
    if(log_function != nullptr)
    {
        try
        {
            log_function(log_level, message);
        }
        catch(const std::exception& e)
        {
            // Avoid error output for not important messages
            if(log_level <= log_level_t::warning)
            {
                std::cerr << "ERROR:   Failed to write/send log message: " << e.what() << '\n';
                std::cerr << "         Failed log message: " << message << '\n';
            }
        }
    }
}

GNUC_ATTRIBUTE(weak) log_level_t wc_get_log_level() noexcept
{
    return wago::logging::get_log_level();
}

namespace wago {
namespace logging {

log_level_t get_log_level()
{
    return current_log_level;
}

void set_log_level(log_level_t const level)
{
    current_log_level = level;
}

void set_log_function(log_function_t const log_function)
{
    current_log_function = log_function;
    current_log_function_simple = nullptr;
}

void set_log_function(log_function_simple_t const log_function_simple)
{
    current_log_function_simple = log_function_simple;
    if(log_function_simple != nullptr)
    {
        current_log_function = log_without_level;
    }
    else
    {
        current_log_function = nullptr;
    }
}

void log_stdout(log_level_t  const log_level,
                char const * const message)
{
    log_outstream(log_level, message, std::cout);
}

void log_stderr(log_level_t  const log_level,
                char const * const message)
{
    log_outstream(log_level, message, std::cerr);
}

void log_outstream(log_level_t  const  log_level,
                   char const * const  message,
                   std::ostream       &outstream)
{
    // Using buffer to avoid mixed output by different logging threads
    stringstream buffer;
    buffer << get_log_level_tag(log_level) << " " << message << std::endl;
    // Flush needed for output stream to avoid further buffering
    outstream << buffer.str() << std::flush;
}

void init(char const * const service_name)
{
    sal::syslog::get_instance().openlog(service_name, LOG_PID, default_syslog_facility);
}

void close()
{
    sal::syslog::get_instance().closelog();
}

void log_syslog(log_level_t  const log_level,
                char const * const message)
{
    int syslog_priority = 0;
    switch(log_level)
    {
        case debug:
            syslog_priority = sal::syslog::log_debug;
            break;

        case info:
            syslog_priority = sal::syslog::log_info;
            break;

        case notice:
            syslog_priority = sal::syslog::log_notice;
            break;

        case warning:
            syslog_priority = sal::syslog::log_warning;
            break;

        case error:
            syslog_priority = sal::syslog::log_error;
            break;

        case fatal:
            syslog_priority = sal::syslog::log_critical;
            break;

        default:
            WC_FAIL("Tried to get syslog priority for unsupported log level");
        case off:
            return;

    }
    sal::syslog::get_instance().log(default_syslog_facility|syslog_priority, message);
}

void log_journal(log_level_t  const log_level,
                 char const * const message)
{
#ifdef SYSTEMD_INTEGRATION

    int journal_priority = sal::journal::log_notice;
    switch(log_level)
    {
        case debug:
            journal_priority = sal::journal::log_debug;
            break;

        case info:
            journal_priority = sal::journal::log_info;
            break;

        case notice:
            journal_priority = sal::journal::log_notice;
            break;

        case warning:
            journal_priority = sal::journal::log_warning;
            break;

        case error:
            journal_priority = sal::journal::log_error;
            break;

        case fatal:
            journal_priority = sal::journal::log_critical;
            break;

        default:
            WC_FAIL("Tried to get journal priority for unsupported log level");
        case off:
            return;

    }
    sal::journal::get_instance().log(journal_priority, message);

#else

    log_stderr(log_level_t::warning, "Log via journal requested but not supported");
    log_stderr(log_level, message);

#endif // SYSTEMD_INTEGRATION
}

void log_without_level(log_level_t  const WC_UNUSED_PARAM(log_level),
                       char const * const message)
{
    log_function_simple_t const log_function_simple = current_log_function_simple;
    WC_ASSERT(log_function_simple != nullptr);
    log_function_simple(message);
}

char const * get_log_level_tag(log_level_t const log_level)
{
    char const * log_level_tag = "";

    switch(log_level)
    {
        case debug:
            log_level_tag = "[ DEBUG ]";
            break;

        case info:
            log_level_tag = "[ INFO  ]";
            break;

        case notice:
            log_level_tag = "[ NOTE  ]";
            break;

        case warning:
            log_level_tag = "[ WARN  ]";
            break;

        case error:
            log_level_tag = "[ ERROR ]";
            break;

        case fatal:
            log_level_tag = "[ FATAL ]";
            break;

        default:
            WC_FAIL("Tried to get log level tag for unsupported log level");
        case off:
            break;
    }

    return log_level_tag;
}


} // Namespace logging
} // Namespace wago


//---- End of source file ------------------------------------------------------
