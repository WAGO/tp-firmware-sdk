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
#ifndef SRC_LIBAUTHSERV_response_helper_HPP_
#define SRC_LIBAUTHSERV_response_helper_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "response_helper_i.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class response_helper : public response_helper_i
{
private:
    std::string                       const escaped_service_name_m;
    std::shared_ptr<settings_store_i>       settings_store_m;
    std::vector<std::string>                password_setup_template_m;
    std::vector<std::string>                login_template_m;
    std::vector<std::string>                password_change_template_m;
    std::vector<std::string>                confirmation_template_m;

public:
    response_helper(std::string                       const &service_name,
                    std::shared_ptr<settings_store_i>        settings_store);
    
    void send_password_setup_page(http::request_i                     &request,
                                  std::string                   const &cancel_uri,
                                  http::http_status_code               code,
                                  response_helper_i::error_type const &error_type = no_error,
                                  std::string                   const &additional_error_message = "") const override;

    void send_login_page(http::request_i                     &request,
                         std::string                   const &cancel_uri,
                         http::http_status_code               code,
                         std::string                   const &accessing_client_label,
                         response_helper_i::error_type const &error_type = no_error,
                         std::string                   const &additional_error_message = "",
                         std::string                   const &success_message = "") const override;

    void send_password_change_page(http::request_i                     &request,
                                   std::string                   const &cancel_uri,
                                   http::http_status_code               code,
                                   std::string                   const &user_name,
                                   response_helper_i::error_type const &error_type = no_error,
                                   std::string                   const &additional_error_message = "",
                                   std::string                   const &info_message = "") const override;
    
    void send_success_confirmation(http::request_i              &request,
                                   std::string            const &continue_uri,
                                   http::http_status_code        code,
                                   std::string            const &success_message = "") const override;
    
    void send_error_confirmation(http::request_i                     &request,
                                 std::string                   const &continue_uri,
                                 http::http_status_code              code,
                                 response_helper_i::error_type const &error_type = internal_error,
                                 std::string                   const &additional_error_message = "") const override;
};

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_response_helper_HPP_
//---- End of source file ------------------------------------------------------
