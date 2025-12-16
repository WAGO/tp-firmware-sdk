//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    C++ API of WAGO Parameter Service on Linux controllers.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_SERV_PARAMETER_SERVICE_HPP_
#define INC_WAGO_WDX_LINUXOS_SERV_PARAMETER_SERVICE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/serv/api.h"
#include "wago/wdx/linuxos/serv/server_i.hpp"

#include <wago/wdx/parameter_service_frontend_i.hpp>
#include <wago/wdx/parameter_service_backend_extended_i.hpp>
#include <wago/wdx/wda/parameter_service.hpp>
#include <wago/wdx/wda/service_identity_i.hpp>
#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {

class parameter_service_frontend_i;
class parameter_service_backend_extended_i;

inline namespace file_transfer {
class parameter_service_file_api_i;
}

namespace wda {
class settings_store_i;
}
namespace linuxos {
namespace serv {
namespace auth {
    class oauth2_backend;
}

using wdx::wda::settings_store_i;


/// \brief Main class for WAGO Parameter Service on linux based systems.
///
/// An instance of this class is a complete parameter service for a linux head station.
class parameter_service
{
private:
    std::string const                                           headstation_ordernumber_m;
    std::string const                                           headstation_firmware_version_m;
    std::shared_ptr<settings_store_i>                           settings_store_m;
    std::shared_ptr<auth::oauth2_backend>                       combined_backend_m;
    //std::unique_ptr<model_provider_i>                           model_provider_m;
    //std::vector<std::unique_ptr<device_description_provider_i>> device_description_providers_m;

    wdx::wda::parameter_service parameter_service_base_m;
    std::unique_ptr<server_i>   rest_api_server_m;
    std::unique_ptr<server_i>   file_api_server_m;

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(parameter_service)

public:
    //------------------------------------------------------------------------------
    /// Construct WAGO Parameter Service.
    /// \param rest_api_base_path
    ///   The base URL to be used by the REST frontend.
    /// \param rest_api_socket_path
    ///   The FCGI socket for REST frontend (of a running web server) to connect to.
    /// \param file_api_base_path
    ///   The base URL to be used by the file frontend.
    /// \param file_api_socket_path
    ///   The FCGI socket for file frontend (of a running web server) to connect to.
    /// \param doc_base_path
    ///   The base URL where the official (redocly) online documentation is
    ///   available at.
    //------------------------------------------------------------------------------
    parameter_service(std::string headstation_ordernumber,
                      std::string headstation_firmware_version,
                      std::string rest_api_base_path,
                      std::string rest_api_socket_path,
                      std::string file_api_base_path,
                      std::string file_api_socket_path,
                      std::string doc_base_path);

    ~parameter_service() noexcept;

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
    server_i & get_rest_api_server();

    //------------------------------------------------------------------------------
    /// Create a WDA file-API server instance connecting to a FCGI socket.
    ///
    /// \param service_base_path
    ///   The base URL to be used by the file frontend.
    /// \param fcgi_socket_path
    ///   The FCGI socket for file frontend (of a running web server) to connect to.
    ///
    /// \return
    ///   Unique pointer to created server instance.
    //------------------------------------------------------------------------------
    server_i & get_file_api_server();

    //------------------------------------------------------------------------------
    /// Method to get the parameter service identity interface for identity information
    /// (e. g. service version).
    ///
    /// \return
    ///   Reference to service identity interface.
    //------------------------------------------------------------------------------
    static wdx::wda::service_identity_i const & get_service_identity();

    //------------------------------------------------------------------------------
    /// Method to get the parameter service frontend interface for service requests.
    ///
    /// \return
    ///   Wrapped service frontend interface. Unwrap by providing authorization info
    //------------------------------------------------------------------------------
    wdx::unauthorized<wdx::parameter_service_frontend_i> & get_unauthorized_frontend();

    //------------------------------------------------------------------------------
    /// Method to get the parameter service file api interface for service requests.
    ///
    /// \return
    ///   Wrapped service file api interface. Unwrap by providing authorization info
    //------------------------------------------------------------------------------
    wdx::unauthorized<wdx::parameter_service_file_api_i> & get_unauthorized_file_api();

    //------------------------------------------------------------------------------
    /// Method to get the parameter service backend interface for service data management
    /// (e. g. topology or parameter provider management).
    ///
    /// \return
    ///   Reference to service backend interface.
    //------------------------------------------------------------------------------
    wdx::parameter_service_backend_extended_i & get_backend();

    //------------------------------------------------------------------------------
    /// Cleanup should be executed before destruction and before parameter- or
    /// other providers are unregistered.
    ///
    /// \param timeout_ms
    ///   Milliseconds to wait for a response at most. Will return nonetheless.
    ///
    /// \return
    ///   Reference to service backend interface.
    //------------------------------------------------------------------------------
    void cleanup(uint32_t timeout_ms);
};


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_SERV_PARAMETER_SERVICE_HPP_
//---- End of source file ------------------------------------------------------
