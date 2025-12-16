//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Parameter Provider for auth service token lifetimes
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_AUTH_SERVICE_PROVIDER_HPP_
#define INC_WAGO_AUTHSERV_AUTH_SERVICE_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/oauth_token_handler_i.hpp"
#include "wago/authserv/settings_store_i.hpp"
#include "wago/wdx/base_parameter_provider.hpp"

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class auth_service_provider final
: public wdx::base_parameter_provider
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(auth_service_provider)

public:
    auth_service_provider(std::shared_ptr<settings_store_i>      settings_store,
                            std::shared_ptr<oauth_token_handler_i> oauth_token_handler);
    ~auth_service_provider() noexcept override;

    // Interface parameter_provider_i
    std::string display_name() override;
    wdx::parameter_selector_response get_provided_parameters() override;
    future<std::vector<wdx::value_response>> get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids) override;
    future<std::vector<wdx::set_parameter_response>> set_parameter_values(std::vector<wdx::value_request> value_requests) override;
    future<wdx::method_invocation_response> invoke_method(wdx::parameter_instance_id                         method_id,
                                                          std::vector<std::shared_ptr<wdx::parameter_value>> in_args) override;

private:
    std::shared_ptr<settings_store_i>      settings_store_m;
    std::shared_ptr<oauth_token_handler_i> oauth_token_handler_m;
};

} // Namespace authserv
} // Namespace wago

#endif // INC_WAGO_AUTHSERV_AUTH_SERVICE_PROVIDER_HPP_
//---- End of source file ------------------------------------------------------

