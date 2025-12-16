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
///  \brief    parameter provider for the token lifetime and revoke methods
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/auth_service_provider.hpp"
#include <wc/log.h>

namespace wago {
namespace authserv {

#define DISPLAY_NAME "Auth Service Provider"

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {
std::shared_ptr<wdx::parameter_value> get_global_setting_as_uint64_param(settings_store_i::global_setting setting,
                                                                         settings_store_i &settings_store);
std::shared_ptr<wdx::parameter_value> get_global_setting_as_bool_param(settings_store_i::global_setting setting,
                                                                         settings_store_i &settings_store);
constexpr uint32_t g_parameter_oauth2server_authcodelifetime = 6000;
constexpr uint32_t g_parameter_oauth2server_accesstokenlifetime = 6001;
constexpr uint32_t g_parameter_oauth2server_refreshtokenlifetime = 6002;
constexpr uint32_t g_parameter_oauth2server_revokealltokens = 6003;
constexpr uint32_t g_parameter_oauth2server_revoketoken = 6004;
constexpr uint32_t g_parameter_oauth2server_silentmodeenabled = 6005;
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

auth_service_provider::auth_service_provider(std::shared_ptr<settings_store_i>      settings_store,
                                             std::shared_ptr<oauth_token_handler_i> oauth_token_handler)
: settings_store_m(settings_store)
, oauth_token_handler_m(oauth_token_handler)
{ }

auth_service_provider::~auth_service_provider() noexcept
{ }

std::string auth_service_provider::display_name()
{
    return DISPLAY_NAME;
}

wdx::parameter_selector_response auth_service_provider::get_provided_parameters()
{
    wdx::parameter_selector_response params({
        wdx::parameter_selector::all_of_feature("OAuth2Server_V2", wdx::device_selector::headstation())
    });

    return params;
}

future<std::vector<wdx::value_response>> auth_service_provider::get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids)
{
    std::vector<wdx::value_response> result(parameter_ids.size());

    for(size_t i = 0; i < parameter_ids.size(); ++i)
    {
        try
        {
            switch (parameter_ids[i].id)
            {
                case g_parameter_oauth2server_authcodelifetime:
                    WC_DEBUG_LOG( DISPLAY_NAME ": Get auth code");
                    result[i].set_value(get_global_setting_as_uint64_param(settings_store_i::auth_code_lifetime, *settings_store_m));
                    break;
                case g_parameter_oauth2server_accesstokenlifetime:
                    WC_DEBUG_LOG( DISPLAY_NAME ": Get access token");
                    result[i].set_value(get_global_setting_as_uint64_param(settings_store_i::access_token_lifetime, *settings_store_m));
                    break;
                case g_parameter_oauth2server_refreshtokenlifetime:
                    WC_DEBUG_LOG( DISPLAY_NAME ": Get refresh token");
                    result[i].set_value(get_global_setting_as_uint64_param(settings_store_i::refresh_token_lifetime, *settings_store_m));
                    break;
                case g_parameter_oauth2server_silentmodeenabled:
                    WC_DEBUG_LOG( DISPLAY_NAME ": Get silent mode enabled");
                    result[i].set_value(get_global_setting_as_bool_param(settings_store_i::silent_mode_enabled, *settings_store_m));
                    break;
                default:
                    // Do nothing, response has already correct status by default for this case
                    break;
            }
        }
        catch (std::exception const &ex)
        {
            auto const error_message = std::string(DISPLAY_NAME ": Failed to get value: ") + ex.what();
            wc_log(log_level_t::error, error_message);
            result[i].set_error(wdx::status_codes::internal_error);
        }
    }

    return resolved_future(std::move(result));
}

//using AFTER_LAST as marker for unknown parameters
constexpr settings_store_i::global_setting unknown_parameter = settings_store_i::AFTER_LAST;

future<std::vector<wdx::set_parameter_response>> auth_service_provider::set_parameter_values(std::vector<wdx::value_request> value_requests)
{
    bool all_values_success = false;
    settings_store_i::configuration_error_map errors;

    std::vector<settings_store_i::global_setting> requested_config_keys;
    settings_store_i::global_config_map requested_config;
    try
    {
        for(size_t i = 0; i < value_requests.size(); ++i)
        {
            switch (value_requests[i].param_id.id)
            {
                case g_parameter_oauth2server_authcodelifetime:
                    requested_config_keys.push_back(settings_store_i::auth_code_lifetime);
                    requested_config.emplace(settings_store_i::auth_code_lifetime, std::to_string(value_requests[i].value->get_uint64()));
                    break;
                case g_parameter_oauth2server_accesstokenlifetime:
                    requested_config_keys.push_back(settings_store_i::access_token_lifetime);
                    requested_config.emplace(settings_store_i::access_token_lifetime, std::to_string(value_requests[i].value->get_uint64()));
                    break;
                case g_parameter_oauth2server_refreshtokenlifetime:
                    requested_config_keys.push_back(settings_store_i::refresh_token_lifetime);
                    requested_config.emplace(settings_store_i::refresh_token_lifetime, std::to_string(value_requests[i].value->get_uint64()));
                    break;
                case g_parameter_oauth2server_silentmodeenabled:
                    requested_config_keys.push_back(settings_store_i::silent_mode_enabled);
                    requested_config.emplace(settings_store_i::silent_mode_enabled, value_requests[i].value->get_boolean() ? "true" : "false");
                    break;
                default:
                    requested_config_keys.push_back(unknown_parameter);
                    break;
            }
        }

        settings_store_m->set_global_config(requested_config, errors);
        all_values_success = errors.empty();
    }
    catch (std::exception const &ex)
    {
        wc_log(log_level_t::error, DISPLAY_NAME ": Failed to set parameter: " + std::string(ex.what()));
        errors.clear();
    }

    std::vector<wdx::set_parameter_response> result(value_requests.size());
    for(size_t i = 0; i < value_requests.size(); ++i)
    {
        if (requested_config_keys[i] == unknown_parameter) {} // response has already correct state by default
        else if (all_values_success) { result[i].set_success(); }
        else if (errors.empty()) { result[i].set_error(wdx::status_codes::internal_error); }
        else if (errors.count(requested_config_keys[i]) > 0)
        {
            std::string error_msg = errors.at(requested_config_keys[i]);
            result[i].set_error(wdx::status_codes::invalid_value, error_msg);
            wc_log(log_level_t::warning, DISPLAY_NAME ": Failed to set " +
                                         std::string(settings_store_i::global_setting_names[requested_config_keys[i]]) +
                                         ": " + error_msg);
        }
        else { result[i].set_error(wdx::status_codes::other_invalid_value_in_set); }
    }

    return resolved_future(std::move(result));
}

future<wdx::method_invocation_response> auth_service_provider::invoke_method(wdx::parameter_instance_id                         method_id,
                                                                             std::vector<std::shared_ptr<wdx::parameter_value>> in_args)
{
    wdx::method_invocation_response response;

    try
    {
        switch (method_id.id)
        {
            case g_parameter_oauth2server_revokealltokens:
                WC_DEBUG_LOG( DISPLAY_NAME ": Revoke all");
                oauth_token_handler_m->revoke_all_tokens();
                response.set_success();
                break;
            case g_parameter_oauth2server_revoketoken:
            {
                WC_ASSERT(in_args[0]->get_rank() == wdx::parameter_value_rank::scalar);
                WC_ASSERT(in_args[0]->get_type() == wdx::parameter_value_types::string);
                std::string token = in_args[0]->get_string();
                WC_DEBUG_LOG(DISPLAY_NAME ": Revoke token \"" + token + "\"");
                oauth_token_handler_m->revoke_token(token);
                response.set_success();
                break;
            }
            default:
                // Do nothing, response has already correct status by default for this case
                break;
        }
    }
    catch(std::exception const &ex)
    {
        auto const error_message = std::string(DISPLAY_NAME ": Execute method: ") + ex.what();
        wc_log(log_level_t::error, error_message);
        response.set_error(wdx::status_codes::internal_error);
    }

    return resolved_future(std::move(response));
}

namespace {
std::shared_ptr<wdx::parameter_value> get_global_setting_as_uint64_param(settings_store_i::global_setting setting,
                                                                         settings_store_i &settings_store)
{
    std::string value_string = settings_store.get_global_setting(setting);
    WC_STATIC_ASSERT(sizeof(uint64_t) == sizeof(unsigned long long));
    std::size_t pos = 0;
    uint64_t value = std::stoull(value_string, &pos); // unsigned long long is equivalent to uint64
    if(pos != value_string.length())
    {
        std::string setting_name = "unknown";
        if(setting < (sizeof(settings_store_i::global_setting_names) / sizeof(*settings_store_i::global_setting_names)))
        {
            setting_name = settings_store_i::global_setting_names[setting];
        }
        throw std::runtime_error("Invalid value for setting '" + setting_name + "'");
    }
    return wdx::parameter_value::create_uint64(value);
}

std::shared_ptr<wdx::parameter_value> get_global_setting_as_bool_param(settings_store_i::global_setting setting,
                                                                       settings_store_i &settings_store)
{
    std::string value_string = settings_store.get_global_setting(setting);
    if(!(value_string == "true" || value_string == "false"))
    {
        std::string setting_name = "unknown";
        if(setting < (sizeof(settings_store_i::global_setting_names) / sizeof(*settings_store_i::global_setting_names)))
        {
            setting_name = settings_store_i::global_setting_names[setting];
        }
        throw std::runtime_error("Invalid value for setting '" + setting_name + "'");
    }
    bool value = (value_string == "true");
    return wdx::parameter_value::create_boolean(value);
}
} // Namespace anonym

} // Namespace authserv
} // Namespace wago

//---- End of source file ------------------------------------------------------

