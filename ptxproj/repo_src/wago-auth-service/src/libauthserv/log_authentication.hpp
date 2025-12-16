//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Log text kit for authentication protocol messages
///            related to requirement YBP-5163.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_LOG_AUTHENTICATION_HPP_
#define SRC_LIBAUTHSERV_LOG_AUTHENTICATION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/log.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

static inline void log_authentication_event(log_level_t const &log_level,
                                            std::string const &event,
                                            std::string const &user_name,
                                            std::string const &client_id,
                                            std::string const &scopes,
                                            std::string const &grant_type) noexcept
{
    std::string message = event;
    message += user_name.empty() ? ", " : (" "
               "user "      "\"" + user_name  + "\", ");
    message += "client ID " "\"" + client_id  + "\", "
               "scopes: "   "["  + scopes     + "], "
               "grant: "    "\"" + grant_type + "\"";

    wc_log(log_level, message);
}

static inline void log_authentication_info(std::string const &event,
                                           std::string const &user_name,
                                           std::string const &client_id,
                                           std::string const &scopes,
                                           std::string const &grant_type) noexcept
{
    log_authentication_event(log_level_t::notice, event, user_name, client_id, scopes, grant_type);
}

static inline void log_authentication_warning(std::string const &event,
                                              std::string const &user_name,
                                              std::string const &client_id,
                                              std::string const &scopes,
                                              std::string const &grant_type) noexcept
{
    log_authentication_event(log_level_t::warning, event, user_name, client_id, scopes, grant_type);
}

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_LOG_AUTHENTICATION_HPP_
//---- End of source file ------------------------------------------------------
