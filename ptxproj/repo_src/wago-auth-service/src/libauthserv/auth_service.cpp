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
///  \brief    Implementation of WAGO Auth Service on Linux controllers.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/auth_service_provider.hpp"
#include "wago/authserv/auth_service.hpp"
#include "settings_store.hpp"
#include "oauth_token_handler.hpp"
#include "fcgi/server.hpp"
#include "http/secure_handler.hpp"
#include "web_frontend.hpp"

#include <memory>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

using fcgi::server;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
auth_service::auth_service(std::string                       const &service_file,
                           std::string                       const &service_name,
                           std::string                       const &web_api_base_path,
                           std::string                       const &web_api_socket_path,
                           wdx::parameter_service_frontend_i       &parameter_service_frontend)
: settings_store_m(std::make_shared<settings_store>())
, oauth_token_handler_m(std::make_shared<oauth_token_handler>(*settings_store_m))
, parameter_service_frontend_m(parameter_service_frontend)

// HTTPS (TLS) required by RFC 6749:
// https://www.rfc-editor.org/rfc/rfc6749#section-3.1
, request_handler_m(std::make_unique<http::secure_handler>(std::make_unique<web_frontend>(service_file,
                                                                                          service_name,
                                                                                          web_api_base_path,
                                                                                          settings_store_m,
                                                                                          parameter_service_frontend_m,
                                                                                          oauth_token_handler_m),
                                                           true))
, web_api_server_m(std::make_unique<server>(web_api_socket_path, *request_handler_m))
{ }

auth_service::~auth_service() noexcept = default;

server_i & auth_service::get_web_api_server()
{
    return *web_api_server_m;
}

std::shared_ptr<settings_store_i> auth_service::get_settings_store()
{
    return settings_store_m;
}

std::shared_ptr<oauth_token_handler_i> auth_service::get_oauth_token_handler()
{
    return oauth_token_handler_m;
}

void auth_service::reload_config() noexcept
{
    settings_store_m->reload_config();
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
