//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
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
#ifndef INC_WAGO_WDX_WDA_PARAMETER_SERVICE_HPP_
#define INC_WAGO_WDX_WDA_PARAMETER_SERVICE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/api.h"
#include "wago/wdx/wda/service_identity_i.hpp"
#include "wago/wdx/wda/auth/password_backend_i.hpp"
#include "wago/wdx/wda/auth/token_backend_i.hpp"
#include "wago/wdx/wda/http/request_handler_i.hpp"
#include "wago/wdx/unauthorized.hpp"

#include <wago/wdx/parameter_service_frontend_i.hpp>
#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>
#include <wago/wdx/parameter_service_backend_extended_i.hpp>
#include <wago/wdx/permissions_i.hpp>
#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
class parameter_service_i;

// FIXME: Force indexing of undocumented WDMM files
/// \file wago/wdx/wda/wdmm/class_definition.hpp
/// \file wago/wdx/wda/wdmm/device_model.hpp
/// \file wago/wdx/wda/wdmm/feature_definition.hpp
/// \file wago/wdx/wda/wdmm/method_argument_definition.hpp
/// \file wago/wdx/wda/wdmm/method_definition.hpp
/// \file wago/wdx/wda/wdmm/parameter_definition_owner.hpp

// FIXME: Ignore missing documentation for WDMM types
/// \class wdmm::method_argument_definition
///
/// No documentation available

/// \var   wdmm::method_argument_definition::name
///
/// No documentation available

/// \var   wdmm::method_argument_definition::value_type
///
/// No documentation available

/// \var   wdmm::method_argument_definition::value_rank
///
/// No documentation available

/// \class wdmm::method_definition
///
/// No documentation available

/// \var   wdmm::method_definition::in_args
///
/// No documentation available

/// \var   wdmm::method_definition::out_args
///
/// No documentation available

/// \class wdmm::parameter_definition
///
/// No documentation available

/// \var   wdmm::parameter_definition::id
///
/// No documentation available

/// \var   wdmm::parameter_definition::path
///
/// No documentation available

/// \var   wdmm::parameter_definition::value_type
///
/// No documentation available

/// \var   wdmm::parameter_definition::value_rank
///
/// No documentation available

/// \var   wdmm::parameter_definition::only_online
///
/// No documentation available

/// \class wdmm::device_model
///
/// No documentation available

/// \var   wdmm::device_model::classes
///
/// No documentation available

/// \var   wdmm::device_model::definitions
///
/// No documentation available

/// \var   wdmm::device_model::name
///
/// No documentation available

/// \var   wdmm::device_model::features
///
/// No documentation available

/// \fn    wdmm::device_model::get_definition(parameter_id_t definition_id)
///
/// No documentation available

/// \fn    wdmm::device_model::find_enum_definition(name_t enum_name)
///
/// No documentation available

/// \fn    wdmm::device_model::find_class_definition(name_t name)
///
/// No documentation available

/// \fn    wdmm::device_model::find_feature_definition(name_t name)
///
/// No documentation available

/// \var   wdmm::device_collections
///
/// No documentation available

/// \class wdmm::parameter_definition_owner
///
/// No documentation available

/// \var   wdmm::parameter_definition_owner::name
///
/// No documentation available

/// \var   wdmm::parameter_definition_owner::includes
///
/// No documentation available

/// \var   wdmm::parameter_definition_owner::parameter_definitions
///
/// No documentation available

/// \var   wdmm::parameter_definition_owner::is_resolved
///
/// No documentation available

/// \var   wdmm::parameter_definition_owner::resolved_includes
///
/// No documentation available

/// \var   wdmm::parameter_definition_owner::resolved_parameter_definitions
///
/// No documentation available

/// \class wdmm::class_definition
///
/// No documentation available

/// \var   wdmm::class_definition::base_path
///
/// No documentation available

/// \var   wdmm::parameter_attributes
///
/// No documentation available

/// \var   wdmm::parameter_value_types
///
/// No documentation available

/// \var   wdmm::parameter_value_rank
///
/// No documentation available

/// \fn    wdmm::to_string(parameter_value_types t)
///
/// No documentation available

/// \fn    wdmm::to_string(parameter_value_rank rank)
///
/// No documentation available


}
}

namespace wago {
namespace wdx {
class model_provider_i;
class device_description_provider_i;

namespace wda {
class settings_store_i;

/// \brief Main class for WAGO Parameter Service.
///
/// An instance of this class is a complete parameter service for a head station.
class parameter_service
{
private:
    std::shared_ptr<wdx::parameter_service_i>        core_m;
    unauthorized<wdx::parameter_service_frontend_i>  unauthorized_frontend_m;
    unauthorized<wdx::parameter_service_file_api_i>  unauthorized_file_api_m;
    service_identity_i                        const &service_identity_m;
    std::unique_ptr<http::request_handler_i>         rest_api_handler_m;
    std::unique_ptr<http::request_handler_i>         file_api_handler_m;

    std::vector<std::shared_ptr<wdx::model_provider_i>>              model_providers_m;
    std::vector<std::shared_ptr<wdx::device_description_provider_i>> device_description_providers_m;
    std::unique_ptr<wdx::parameter_provider_i>                       settings_provider_m;

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(parameter_service)

public:
    /// \brief Simple function to get the WDM content.
    using wdm_getter = std::function<wdx::wdm_content_t()>;

    /// \brief Simple function to get the WDD content for the root device/headstation.
    using wdd_getter = std::function<wdx::wdd_content_t(std::string ordernumber, std::string firmware_version)>;

    //------------------------------------------------------------------------------
    /// Default construct WAGO Parameter Service.
    ///
    /// \param rest_api_base_path
    ///   The base URL to be used for the REST frontend.
    /// \param file_api_base_path
    ///   The base URL to be used for the file frontend.
    /// \param doc_base_path
    ///   The base URL where the official (redocly) online documentation is
    ///   available at.
    /// \param settings_store
    ///   Implements settings store for service settings.
    /// \param get_wdm
    ///   Function to get the WDM content.
    /// \param get_wdd
    ///   Function to get the WDD content for the root device/headstation.
    /// \param password_backend
    ///   Implements password based authentication.
    /// \param token_backend
    ///   Implements token based authentication.
    //------------------------------------------------------------------------------
    parameter_service(std::string                               const &rest_api_base_path,
                      std::string                               const &file_api_base_path,
                      std::string                               const &doc_base_path,
                      service_identity_i                        const &service_identity,
                      std::shared_ptr<settings_store_i>                settings_store,
                      std::unique_ptr<permissions_i>                   permissions,
                      wdm_getter                                       get_wdm,
                      wdd_getter                                       get_wdd,
                      std::shared_ptr<auth::password_backend_i>        password_backend,
                      std::shared_ptr<auth::token_backend_i>           token_backend      = nullptr);

    ~parameter_service() noexcept;

    //------------------------------------------------------------------------------
    /// Use this method to load another WDM. The additional WDM must be in another
    /// WDM-Prefix than the main model. However, this is not checked, so the caller
    /// must ensure this.
    ///
    /// \param get_wdm Function to get the additional WDM content.
    //------------------------------------------------------------------------------
    void load_additional_wdm(wdm_getter get_wdm); 

    //------------------------------------------------------------------------------
    /// Use this method to load an additional WDDs.
    ///
    /// \param get_wdd Function to get the additional WDE content.
    //------------------------------------------------------------------------------
    void load_additional_wdd(wdd_getter get_wdd);

    //------------------------------------------------------------------------------
    /// Create a WDx REST-API request handler instance to be used as a server handler.
    //------------------------------------------------------------------------------
    http::request_handler_i & get_rest_api_handler();

    //------------------------------------------------------------------------------
    /// Create a WDx File-API request handler instance to be used as a server handler.
    //------------------------------------------------------------------------------
    http::request_handler_i & get_file_api_handler();

    //------------------------------------------------------------------------------
    /// Method to get the parameter service frontend interface for service requests.
    ///
    /// \return
    ///   Wrapper of frontend interface to obtain a reference with authorization checks
    //------------------------------------------------------------------------------
    wdx::unauthorized<wdx::parameter_service_frontend_i> & get_unauthorized_frontend();

    //------------------------------------------------------------------------------
    /// Method to get the parameter service file api interface for service requests.
    ///
    /// \return
    ///   Wrapper of file api interface to obtain a reference with authorization checks
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


} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_PARAMETER_SERVICE_HPP_
//---- End of source file ------------------------------------------------------
