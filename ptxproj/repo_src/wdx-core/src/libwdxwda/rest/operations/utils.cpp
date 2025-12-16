//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of utility functions unsed among operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "utils.hpp"

#include "wda_ipc/representation.hpp"
#include "wago/wdx/wda/trace_routes.hpp"
#include "http/head_response.hpp"

#include <wc/trace.h>
#include <algorithm>


namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace {
wdx::parameter_instance_path const get_instance_id_from_request_url(std::shared_ptr<request> const req,
                                                                    char const *             const path_parameter_name,
                                                                    char const *             const name)
{
    // Check parameter ID
    if(!req->has_path_parameter(path_parameter_name))
    {
        throw http_exception("No valid " + string(name) + " instance ID given.", http_status_code::internal_server_error);
    }
    auto const candidate = req->get_path_parameter(path_parameter_name);
    try
    {
        return wda_ipc::from_string<wdx::parameter_instance_path>(candidate);
    }
    catch(...)
    {
        throw http_exception("Unknown " + string(name) + " instance ID.", http_status_code::not_found);
    }
}
}

wdx::parameter_instance_path const get_parameter_instance_id_from_request_url(std::shared_ptr<request> const req)
{
    return get_instance_id_from_request_url(req, path_param_parameter_id, "parameter");
}


wdx::parameter_instance_path const get_method_instance_id_from_request_url(std::shared_ptr<request> const req)
{
    return get_instance_id_from_request_url(req, path_param_method_id, "method");
}

wdx::parameter_instance_path const get_method_definition_id_from_request_url(std::shared_ptr<request> const req)
{
    return get_instance_id_from_request_url(req, path_param_method_definition_id, "method-definitions");
}

wdx::parameter_instance_path const get_parameter_definition_id_from_request_url(std::shared_ptr<request> const req)
{
    return get_instance_id_from_request_url(req, path_param_parameter_definition_id, "parameter-definitions");
}

wdx::instance_id_t get_instance_number_from_request_url(std::shared_ptr<request> const req)
{
    if(!req->has_path_parameter(path_param_instance_no))
    {
        throw http_exception("No valid instance number given.", http_status_code::internal_server_error);
    }
    auto const candidate = req->get_path_parameter(path_param_instance_no);
    try
    {
        return wda_ipc::from_string<wdx::instance_id_t>(candidate);
    }
    catch(...)
    {
        throw http_exception("Unknown instance number.", http_status_code::not_found);
    }
}

void get_feature_id_parts_from_request_url(shared_ptr<request> const  req,
                                           std::string               &device,
                                           std::string               &feature_name)
{
    if(!req->has_path_parameter(path_param_feature_id))
    {
        throw http_exception("No valid feature ID given.", http_status_code::internal_server_error);
    }
    try
    {
        std::string feature_id = req->get_path_parameter(path_param_feature_id);
        auto pos = feature_id.find('-', 0);
        if(pos == std::string::npos)
        {
            throw http_exception("Unknown feature ID.", http_status_code::not_found);
        }
        pos = feature_id.find('-', pos + 1);
        if(pos == std::string::npos)
        {
            throw http_exception("Unknown feature ID.", http_status_code::not_found);
        }
        device = feature_id.substr(0, pos);
        feature_name = feature_id.substr(pos + 1);
    }
    catch(...)
    {
        throw http_exception("Unknown feature ID.", http_status_code::not_found);
    }
}

monitoring_list_id_t get_monitoring_list_id_from_request_url(std::shared_ptr<request> const req)
{
    if(!req->has_path_parameter(path_param_monitoring_list_id))
    {
        throw http_exception("No valid monitoring list ID given.", http_status_code::internal_server_error);
    }
    try
    {
        return static_cast<monitoring_list_id_t>(std::stoul(req->get_path_parameter(path_param_monitoring_list_id)));
    }
    catch(...)
    {
        throw http_exception("Unknown monitoring list ID.", http_status_code::not_found);
    }
}

void verify_instance_path(wdx::parameter_response      const &response,
                          wdx::parameter_instance_path const &instance_path,
                          bool                         const  method,
                          bool                         const  definition)
{
    std::string parameter_path = wda_ipc::to_string(instance_path);
    std::string type_label     = (std::string(method ? "method" : "parameter") + std::string(definition ? " definition" : ""));
    if(response.has_error())
    {
        switch (response.status)
        {
            case core_status_code::unknown_device:
            case core_status_code::unknown_device_collection:
            case core_status_code::invalid_device_slot:
            case core_status_code::invalid_device_collection:
            { 
                // FIXME: core gives us device ids seperated by '/' instead of '-' sometimes.
                std::string fixed_device_path = instance_path.device_path;
                std::replace(fixed_device_path.begin(), fixed_device_path.end(), '/', '-');
                throw core_exception(core_error(response.status, response.get_message(), response.domain_specific_status_code,
                                                type_label, parameter_path, true, "device", fixed_device_path));
                break;
            }
            default:
                throw core_exception(core_error(response.status, response.get_message(), response.domain_specific_status_code,
                                                type_label, parameter_path, true));
                break;
        }
    }
    if(method  && (response.definition->value_type != wdx::parameter_value_types::method))
    {
        throw core_exception(core_error(core_status_code::not_a_method, "", 0,
                                        type_label, parameter_path, true));
    }
    if(!method && (response.definition->value_type == wdx::parameter_value_types::method))
    {
        // TODO: we might NOT use "UNKOWN_PARAMETER_PATH". But a pendant for parameters is missing atm...
        throw core_exception(core_error(core_status_code::unknown_parameter_path, "", 0, 
                                        type_label, parameter_path, true));
    }
}

void verify_instance_path(wdx::parameter_service_frontend_extended_i       &core_frontend,
                          wdx::parameter_instance_path               const &instance_path,
                          bool                                       const  method,
                          bool                                       const  definition)
{
    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: get parameter definitions by path");
    auto const response = core_frontend.get_parameter_definitions_by_path({ instance_path }).get().at(0);
    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done");
    verify_instance_path(response, instance_path, method, definition);
}


std::unique_ptr<response_i> get_error_response(std::shared_ptr<request>        const req,
                                          std::shared_ptr<http_exception>       error)
{
    std::unique_ptr<response_i> error_response;

    try
    {
        serializer_i const &serializer = req->get_serializer();
        error_response = std::make_unique<response<std::shared_ptr<http_exception>>>(error->get_http_status_code(), serializer, std::move(error));
    }
    catch(...)
    {
        error_response = std::make_unique<http::head_response>(error->get_http_status_code());
    }

    return error_response;
}

std::unique_ptr<response_i> simple_redirect(http_method const &request_method,
                                            std::string const &request_uri,
                                            std::string const &request_part,
                                            std::string const &replacement)
{
    WC_DEBUG_LOG("Redirect: Request URI: "          + request_uri );
    WC_DEBUG_LOG("Redirect: Request URL part: "     + request_part);
    WC_DEBUG_LOG("Redirect: Replacement URL part: " + replacement );

    string new_location = request_uri;
    WC_ASSERT(!new_location.empty());
    size_t const start_position = new_location.find(request_part);
    WC_ASSERT(start_position != string::npos);
    new_location.replace(start_position, request_part.size(), replacement);
    WC_DEBUG_LOG("Redirect: Relocation URI: " + new_location);
    map<string, string> response_header = { { "Location" , std::move(new_location) } };

    // Use 307 in case of non-GET or -HEAD requests, as a client may always use a GET
    // when following a 301 redirect!
    // Note: We're not using the 308, because of known incompatibility with IE11 on Win10
    http_status_code redirect_status_code = ((request_method == http_method::head) || (request_method == http_method::get))
        ? http_status_code::moved_permanently
        : http_status_code::temporary_redirect;

    return std::make_unique<http::head_response>(redirect_status_code, response_header);
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
