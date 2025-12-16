//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Response creater for requests to the WAGO Auth Service.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_response_helper_I_HPP_
#define SRC_LIBAUTHSERV_response_helper_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>

#include <string>
#include <vector>
#include <map>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class authenticator_i;
class oauth_token_handler_i;
class settings_store_i;
namespace http {
class request_i;
enum class http_status_code: uint16_t;
}

class response_helper_i
{
    WC_INTERFACE_CLASS(response_helper_i)
public:
    // constant used as link of javascript if the tab should be closed instead of redirected 
    constexpr static char CLOSE_TAB[] = "";
    enum error_type {
        no_error,
        internal_error,
        invalid_request,
        invalid_username_or_password,
        invalid_new_password
    };
    
public:
    virtual void send_password_setup_page(http::request_i                     &request,
                                          std::string                   const &cancel_uri,
                                          http::http_status_code               code,
                                          response_helper_i::error_type const &error_type = no_error,
                                          std::string                   const &additional_error_message = "") const = 0;

    virtual void send_login_page(http::request_i                     &request,
                                 std::string                   const &cancel_uri,
                                 http::http_status_code               code,
                                 std::string                   const &accessing_client_label,
                                 response_helper_i::error_type const &error_type = no_error,
                                 std::string                   const &additional_error_message = "",
                                 std::string                   const &success_message = "") const = 0;

    virtual void send_password_change_page(http::request_i                     &request,
                                           std::string                   const &cancel_uri,
                                           http::http_status_code               code,
                                           std::string                   const &user_name,
                                           response_helper_i::error_type const &error_type = no_error,
                                           std::string                   const &additional_error_message = "",
                                           std::string                   const &info_message = "") const = 0;
    
    virtual void send_success_confirmation(http::request_i              &request,
                                           std::string            const &continue_uri,
                                           http::http_status_code        code,
                                           std::string            const &success_message = "") const = 0;
    
    virtual void send_error_confirmation(http::request_i                     &request,
                                         std::string                   const &continue_uri,
                                         http::http_status_code               code,
                                         response_helper_i::error_type const &error_type = internal_error,
                                         std::string                   const &additional_error_message = "") const = 0;
};

//------------------------------------------------------------------------------
// static value definitions
//------------------------------------------------------------------------------
constexpr char   const system_use_notification_file[] = "/etc/system-use-notification";
constexpr size_t const system_use_notification_max    = 1024 * 1024; // Limit memory usage to 1 MiB

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_response_helper_I_HPP_
//---- End of source file ------------------------------------------------------
