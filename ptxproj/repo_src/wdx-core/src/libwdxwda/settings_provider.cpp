//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of the settings parameter provider.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "settings_provider.hpp"

#include <wc/assertion.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

static constexpr char const * g_settings_provider_display_name = "WDA Settings Provider";

// Parameter ID for the allowed unauthenticated requests setting
static constexpr parameter_id_t g_parameter_allowed_unauthenticated_requests = 8000;

// Enum type name for WAGO Device Access Request Types
static constexpr char const * g_enum_wago_device_access_request_types = "WAGODeviceAccessRequestTypes";

// Enum value for scan_devices request type
static constexpr uint16_t g_enum_scan_devices = 1;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

settings_provider::settings_provider(std::shared_ptr<settings_store_i> const &settings_store)
: settings_store_m(settings_store)
{ }

std::string settings_provider::display_name()
{
    return g_settings_provider_display_name;
}

parameter_selector_response settings_provider::get_provided_parameters()
{
    parameter_selector_response response;
    
    auto selector = parameter_selector::all_with_definition(g_parameter_allowed_unauthenticated_requests, device_selector::headstation());
    response.selected_parameters.push_back(selector);
    
    return response;
}

wago::future<std::vector<value_response>> settings_provider::get_parameter_values(std::vector<parameter_instance_id> parameter_ids)
{
    std::vector<value_response> result(parameter_ids.size());

    for(size_t i = 0; i < parameter_ids.size(); ++i)
    {
        try
        {
            switch (parameter_ids[i].id)
            {
                case g_parameter_allowed_unauthenticated_requests:
                    result[i].set_value(load_allowed_unauthenticated_requests());
                    break;
                default:
                    // Do nothing, response has already correct status by default for this case
                    break;
            }
        }
        catch(std::exception const &exception)
        {
            result[i].set_error(status_codes::internal_error, exception.what());
        }
    }

    wago::promise<std::vector<value_response>> promise;
    promise.set_value(std::move(result));
    return promise.get_future();
}

wago::future<std::vector<set_parameter_response>> settings_provider::set_parameter_values(std::vector<value_request> value_requests)
{
    std::vector<set_parameter_response> result(value_requests.size());

    for(size_t i = 0; i < value_requests.size(); ++i)
    {
        try
        {
            switch (value_requests[i].param_id.id)
            {
                case g_parameter_allowed_unauthenticated_requests:
                    save_allowed_unauthenticated_requests(value_requests[i].value);
                    result[i].set_success();
                    break;
                default:
                    // Do nothing, response has already correct status by default for this case
                    break;
            }
        }
        catch(std::exception const &exception)
        {
            result[i].set_error(status_codes::internal_error, exception.what());
        }
    }

    wago::promise<std::vector<set_parameter_response>> promise;
    promise.set_value(std::move(result));
    return promise.get_future();
}

std::shared_ptr<parameter_value> settings_provider::load_allowed_unauthenticated_requests()
{
    std::vector<uint16_t> enum_values;
    std::string setting = settings_store_m->get_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices);
    if(setting == "true")
    {
        enum_values.push_back(g_enum_scan_devices);
    }
    else if(setting != "false")
    {
        throw std::runtime_error("unexpected config value (expected either true or false)");
    }
    return parameter_value::create_enum_value_array(enum_values);
}

void settings_provider::save_allowed_unauthenticated_requests(std::shared_ptr<parameter_value> const &value)
{
    auto items = value->get_items();
    if(items.empty())
    {
        // turn off
        settings_store_m->write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, "false");
    }
    else if(    (items.size() > 1)
             || (items.at(0).get_enum_value() != g_enum_scan_devices))
    {
        throw std::runtime_error("unknown config option");
    }
    else
    {  
        // turn on
        settings_store_m->write_setting(settings_store_i::allow_unauthenticated_requests_for_scan_devices, "true");
    }
}

} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
