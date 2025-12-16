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
///  \brief    C++ API of WAGO Auth Service on Linux controllers.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_AUTH_SERVICE_HPP_
#define INC_WAGO_AUTHSERV_AUTH_SERVICE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"
#include "wago/authserv/server_i.hpp"
#include "wago/authserv/settings_store_i.hpp"
#include "wago/authserv/oauth_token_handler_i.hpp"
#include "wago/authserv/http/request_handler_i.hpp"
#include "wago/wdx/parameter_service_frontend_i.hpp"

#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class web_frontend;

/// \brief Main class for WAGO Auth Service on linux based systems.
///
/// An instance of this class is a complete auth service for a linux head station.
class auth_service
{
private:
    std::shared_ptr<settings_store_i>        settings_store_m;
    std::shared_ptr<oauth_token_handler_i>   oauth_token_handler_m;
    wdx::parameter_service_frontend_i       &parameter_service_frontend_m;

    std::unique_ptr<http::request_handler_i> request_handler_m;
    std::unique_ptr<server_i>                web_api_server_m;

private:
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(auth_service)

public:
    //------------------------------------------------------------------------------
    /// Construct WAGO Auth Service.
    /// \param service_file
    ///   The name of the auth service executable.
    /// \param service_name
    ///   The human readable name of the auth service.
    /// \param web_api_base_path
    ///   The base URL to be used by the web frontend.
    /// \param web_api_socket_path
    ///   The FCGI socket for web frontend (of a running web server) to connect to.
    /// \param parameter_service_frontend
    ///   Parameter Service frontend API to connect to.
    //------------------------------------------------------------------------------
    auth_service(std::string                       const &service_file,
                 std::string                       const &service_name,
                 std::string                       const &web_api_base_path,
                 std::string                       const &web_api_socket_path,
                 wdx::parameter_service_frontend_i       &parameter_service_frontend);

    ~auth_service() noexcept;

    //------------------------------------------------------------------------------
    /// Create a WDA REST-API server instance connecting to a FCGI socket.
    ///
    /// \param service_base_path
    ///   The base URL to be used by the REST frontend.
    /// \param fcgi_socket_path
    ///   The FCGI socket for REST frontend (of a running web server) to connect to.
    ///
    /// \return
    ///   Unique pointer to created server instance.
    //------------------------------------------------------------------------------
    server_i & get_web_api_server();

    //------------------------------------------------------------------------------
    /// Get used settings store.
    ///
    /// \return
    ///   Reference to settings store.
    //------------------------------------------------------------------------------
    std::shared_ptr<settings_store_i> get_settings_store();

    //------------------------------------------------------------------------------
    /// Get used token handler.
    ///
    /// \return
    ///   Reference to token handler.
    //------------------------------------------------------------------------------
    std::shared_ptr<oauth_token_handler_i> get_oauth_token_handler();

    //------------------------------------------------------------------------------
    /// Reload the settings store
    //------------------------------------------------------------------------------
    void reload_config() noexcept;
};


} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_AUTH_SERVICE_HPP_
//---- End of source file ------------------------------------------------------

