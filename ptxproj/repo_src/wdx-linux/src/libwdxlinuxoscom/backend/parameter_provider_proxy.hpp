//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Parameter provider proxy for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_PARAMETER_PROVIDER_PROXY_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_PARAMETER_PROVIDER_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/exception.hpp"
#include "common/abstract_proxy.hpp"
#include "backend_methods.hpp"

#include <wago/wdx/parameter_provider_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class parameter_provider_proxy final : public abstract_proxy<wdx::parameter_provider_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(parameter_provider_proxy)

public:
    using abstract_proxy::abstract_proxy;
    ~parameter_provider_proxy() noexcept override = default;

    void set_name(std::string name)
    {
        provider_name = std::move(name);
    }

    void set_provided_parameters(wdx::parameter_selector_response selected_parameters)
    {
        providers_parameters = std::move(selected_parameters);
    }

    std::string display_name() override
    {
        return provider_name;
    }

    wdx::parameter_selector_response get_provided_parameters() override 
    {
        return providers_parameters;
    }

    future<std::vector<wdx::value_response>> get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids) override
    {
        return call<future<std::vector<wdx::value_response>>>(parameter_provider_method_id::get_parameter_values, parameter_ids);
    }

    future<std::vector<wdx::set_parameter_response>> set_parameter_values(std::vector<wdx::value_request> value_requests) override
    {
        return call<future<std::vector<wdx::set_parameter_response>>>(parameter_provider_method_id::set_parameter_values, value_requests);
    }

    future<std::vector<wdx::set_parameter_response>> set_parameter_values_connection_aware(std::vector<wdx::value_request> value_requests, bool defer_wda_web_connection_changes) override
    {
        return call<future<std::vector<wdx::set_parameter_response>>>(parameter_provider_method_id::set_parameter_values_connection_aware, value_requests, defer_wda_web_connection_changes);
    }

    future<wdx::method_invocation_response> invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args) override
    {
        return call<future<wdx::method_invocation_response>>(parameter_provider_method_id::invoke_method, method_id, in_args);
    }

    future<wdx::register_file_provider_response> create_parameter_upload_id(wdx::parameter_id_t context) override
    {
        return call<future<wdx::register_file_provider_response>>(parameter_provider_method_id::create_parameter_upload_id, context);
    }

    future<wdx::response> remove_parameter_upload_id(wdx::file_id id, wdx::parameter_id_t context) override
    {
        return call<future<wdx::response>>(parameter_provider_method_id::remove_parameter_upload_id, id, context);
    }

private:
    std::string                      provider_name        = "No name set, yet";
    wdx::parameter_selector_response providers_parameters = wdx::parameter_selector_response(wdx::status_codes::internal_error,
                                                                                             "No parameters selected, yet");
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_PARAMETER_PROVIDER_PROXY_HPP_
//---- End of source file ------------------------------------------------------
