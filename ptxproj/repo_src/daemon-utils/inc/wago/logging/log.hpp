//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Definition for common log functionality.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_LOGGING_LOG_HPP_
#define INC_WAGO_LOGGING_LOG_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/log.h>
#include <iostream>

// export symbols for functions declared by wc/log
extern "C"
{
WAGO_LOGGING_API void wc_log_output(log_level_t  const log_level,
                                    char const * const message) noexcept;
WAGO_LOGGING_API log_level_t wc_get_log_level(void) noexcept;
}

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace logging {

//------------------------------------------------------------------------------
/// Type definition for a log function pointer to a function used for log output
/// with differentiation based on a log level. This function conforming to this 
/// type can be used with \link set_log_function() \endlink.
///
/// \param log_level
///   Log level for current message.
/// \param message
///   Current log message to process.
///
/// \see For undifferentiated log output see \link log_function_simple_t() \endlink.
///
/// There exist pre-defined functions which can be used directly:
/// \see \link log_stdout() \endlink for log processing by std::out
/// \see \link log_stderr() \endlink for log processing by std::err
/// \see \link log_syslog() \endlink for log processing by syslog
/// \see \link log_journal() \endlink for log processing by systemd journal
//------------------------------------------------------------------------------
using log_function_t = void(*)(log_level_t  const log_level, 
                               char const * const message);

//------------------------------------------------------------------------------
/// Type definition for a simple log function pointer to a function used for
/// log output without differentiation based on a log level.
///
/// \param message
///   Current log message to process.
///
/// \see For differentiated log output see \link log_function_t() \endlink.
//------------------------------------------------------------------------------
using log_function_simple_t = void(*)(char const * const message);

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// Pre-defined function for log output to stdout.
/// This function may be used with \link set_log_function() \endlink.
///
/// \param log_level
///   Log level for current message.
/// \param message
///   Current log message to process.
///
/// \see \link log_outstream() \endlink
/// \see \link log_stderr() \endlink
/// \see \link log_syslog() \endlink
/// \see \link log_journal() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void log_stdout(log_level_t  const log_level,
                                 char const * const message);

//------------------------------------------------------------------------------
/// Pre-defined function for log output to stderr.
/// This function may be used with \link set_log_function() \endlink.
///
/// \param log_level
///   Log level for current message.
/// \param message
///   Current log message to process.
///
/// \see \link log_outstream() \endlink
/// \see \link log_stdout() \endlink
/// \see \link log_syslog() \endlink
/// \see \link log_journal() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void log_stderr(log_level_t  const log_level,
                                 char const * const message);

//------------------------------------------------------------------------------
/// Pre-defined function for log output to an output stream.
///
/// \param log_level
///   Log level for current message.
/// \param message
///   Current log message to process.
/// \param outstream
///   Output stream to use for output (e. g. stdout, stderr, ...)
///
/// \see \link log_stdout() \endlink
/// \see \link log_stderr() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void log_outstream(log_level_t  const  log_level,
                                    char const * const  message,
                                    std::ostream       &outstream);

//------------------------------------------------------------------------------
/// Initialize function to initialize & open selected log for this process.
///
/// \param service_name
///   Service name to use for log messages.
///
/// \see \link close() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void init(char const * const service_name);

//------------------------------------------------------------------------------
/// Function to close log for this process.
///
/// \see \link init() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void close();

//------------------------------------------------------------------------------
/// Pre-defined function for log output to system log (syslog).
/// This function may be used with \link set_log_function() \endlink.
///
/// \param log_level
///   Log level for current message.
/// \param message
///   Current log message to process.
///
/// \see \link log_stdout() \endlink
/// \see \link log_stderr() \endlink
/// \see \link log_journal() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void log_syslog(log_level_t  const log_level,
                                 char const * const message);

//------------------------------------------------------------------------------
/// Pre-defined function for log output to systemd journal.
/// This function may be used with \link set_log_function() \endlink.
///
/// \param log_level
///   Log level for current message.
/// \param message
///   Current log message to process.
///
/// \see \link log_stdout() \endlink
/// \see \link log_stderr() \endlink
/// \see \link log_syslog() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void log_journal(log_level_t  const log_level,
                                  char const * const message);

//------------------------------------------------------------------------------
/// Get currently configured log level.
///
/// \return
///   Currently configured log level.
///
/// \see \link set_log_level() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API log_level_t get_log_level();

//------------------------------------------------------------------------------
/// Configure log level (global scope for whole executable).
///
/// \param level
///   Log level to configure.
///
/// \see \link get_log_level() \endlink
//------------------------------------------------------------------------------
WAGO_LOGGING_API void set_log_level(log_level_t const level);

//------------------------------------------------------------------------------
/// Set log function to use for log output (global scope for whole executable).
///
/// \param log_function
///   Log function to set.
//------------------------------------------------------------------------------
WAGO_LOGGING_API void set_log_function(log_function_t const log_function);

//------------------------------------------------------------------------------
/// Set simple log function to use for log output (global scope for whole executable).
/// The simple log function (log_function_simple_t) does only receive a message.
/// To also receive a log category/level use function overload with a function of type log_function_t.
///
/// \param log_function_simple
///   Simple log function to set.
//------------------------------------------------------------------------------
WAGO_LOGGING_API void set_log_function(log_function_simple_t const log_function_simple);

} // Namespace logging
} // Namespace wago


#endif // INC_WAGO_LOGGING_LOG_HPP_
//---- End of source file ------------------------------------------------------
