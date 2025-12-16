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
///  \brief    Implementation of parameter related REST-API operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"
#include "rest/parameter_filter.hpp"
#include "rest/data_exception.hpp"
#include "http/head_response.hpp"
#include "utils.hpp"
#include "call_core_frontend.hpp"

#include "wda_ipc/representation.hpp"

#include <wc/log.h>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace {

std::unique_ptr<response_i> create_parameters_response(std::vector<wdx::parameter_response>        parameter_responses,
                                                       serializer_i                         const &serializer,
                                                       bool                                 const  errors_as_attributes,
                                                       std::string                          const  request_path,
                                                       std::string                          const  request_query,
                                                       std::string                          const  request_doc_link,
                                                       unsigned                             const  page_limit,
                                                       unsigned                             const  page_offset,
                                                       unsigned                             const  page_element_max);

void set_parameters_deferred(operation_i                          *operation,
                             frontend_i                           &core_frontend,
                             std::vector<wdx::value_path_request>  deferred_parameter_requests);

future<std::unique_ptr<response_i>> get_parameter_definitions_internal(operation_i                 *operation,
                                                                       std::shared_ptr<request>     req,
                                                                       wdx::parameter_filter const &parameter_filter);

} // Anonymous namespace

future<unique_ptr<response_i>> operation::get_parameters(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    // Get filters like that for BETA parameters from query 
    // And always filter out methods
    auto parameter_filter = extract_filters_from_query(
                                { 
                                    {"beta",                   combine_with_beta_filter        },
                                    {"deprecated",             combine_with_deprecated_filter  },
                                    {"device",                 combine_with_device_filter      },
                                    {"path",                   combine_with_path_filter        },
                                    {"definition.writeable",   combine_with_writeable_filter   },
                                    {"definition.userSetting", combine_with_userSetting_filter }
                                },
                                req->get_filter_queries()
                          );

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        errors_as_attributes=req->get_errors_as_data_attributes_parameter(),
        page_limit,
        page_offset
    ] (wdx::parameter_response_list_response &&core_response) {
        // Create document
        std::unique_ptr<response_i> resp = create_parameters_response(core_response.param_responses, 
                                                                      serializer, 
                                                                      errors_as_attributes,
                                                                      base_path, query, doc_link,
                                                                      page_limit, page_offset, core_response.total_entries);
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_parameters,
                              std::move(core_response_handler),
                              parameter_filter, static_cast<size_t>(page_offset), static_cast<size_t>(page_limit));
}

future<unique_ptr<response_i>> operation::get_parameter(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    vector<wdx::parameter_instance_path> instance_paths = { get_parameter_instance_id_from_request_url(req) };

    // in order to detect methods among parameters, we must pre-check the instance
    // FIXME: still needed?
    verify_instance_path(operation->get_service_frontend(), instance_paths.at(0), false, false);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        instance_paths
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(instance_paths.size() != responses.size())
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        wdx::parameter_response response_parameter = responses.at(0);
        verify_core_response(response_parameter, instance_paths.at(0)); // FIXME: Use verify_instance_path?

        // Create document
        parameter_document document(base_path, query, {{ "doc", doc_link }}, parameter_resource(response_parameter));
        std::unique_ptr<response_i> resp = std::make_unique<response<parameter_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameters_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

future<unique_ptr<response_i>> operation::set_parameters(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    deserializer_i const &deserializer = req->get_deserializer();

    vector<wdx::value_path_request> set_value_requests;
    deserializer.deserialize(set_value_requests, req->get_content());

    auto core_response_handler = [
        operation,
        set_value_requests,
        doc_link=req->get_doc_link(),        
        &serializer=req->get_serializer(),
        handle_deferred_as_error=req->get_deferred_parameters_as_errors(),
        handle_adjusted_as_error=req->get_adjusted_parameters_as_errors(),
        &core_frontend=operation->get_service_frontend()
    ] (std::vector<wdx::set_parameter_response> &&responses) {
        if(set_value_requests.size() != responses.size())
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }

        // Collect errors and info about deferred and adjusted parameters
        char const * error_message = "Some or all parameter requests deferred.";
        std::vector<shared_ptr<data_error>>  parameter_errors;
        std::vector<wdx::value_path_request> deferred_parameter_requests;
        std::vector<set_parameter_resource>  adjusted_parameters;

        for(size_t index = 0; index < responses.size(); ++index)
        {
            auto const &requested_parameter = set_value_requests.at(index);
            auto const &response = responses.at(index);
            auto const  is_deferred = response.status == core_status_code::wda_connection_changes_deferred;
            auto const  is_adjusted = response.status == core_status_code::success_but_value_adjusted;

            if(is_deferred)
            {
                deferred_parameter_requests.push_back(set_value_requests.at(index));
                if(handle_deferred_as_error)
                {
                    parameter_errors.push_back(std::make_shared<data_error>(
                        response, requested_parameter.param_path, index
                    ));
                }
            }
            else if(is_adjusted)
            {
                adjusted_parameters.push_back(set_parameter_resource(requested_parameter, response));
                if(handle_adjusted_as_error)
                {
                    parameter_errors.push_back(std::make_shared<data_error>(
                        response, requested_parameter.param_path, index
                    ));
                }
            }
            else if(response.has_error() || !response.is_determined())
            {
                error_message = "Failed to write some or all parameters.";
                parameter_errors.push_back(std::make_shared<data_error>(
                    response, requested_parameter.param_path, index
                ));
            }
        }

        // write deferred parameters again, if there have been any of them.
        bool const has_deferred_parameters = !deferred_parameter_requests.empty();
        if(has_deferred_parameters)
        {
            set_parameters_deferred(operation, core_frontend, deferred_parameter_requests);
        }

        if(!parameter_errors.empty())
        {
            throw data_exception(error_message, parameter_errors);
        }

        std::unique_ptr<response_i> resp;
        bool const has_adjusted_parameters = !adjusted_parameters.empty();
        if (has_deferred_parameters || has_adjusted_parameters)
        {
            set_parameter_collection_document document({{ "doc", doc_link }}, std::move(adjusted_parameters));
            resp = std::make_unique<response<set_parameter_collection_document>>(has_deferred_parameters ? http_status_code::accepted : http_status_code::ok, 
                                                                                 serializer, std::move(document));
        }
        else
        {
            resp = std::make_unique<http::head_response>(http_status_code::no_content);
        }
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              set_parameter_values_by_path_connection_aware,
                              std::move(core_response_handler),
                              set_value_requests, true);
}

future<unique_ptr<response_i>> operation::set_parameter(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    auto const  instance_path = get_parameter_instance_id_from_request_url(req);
    auto const &deserializer = req->get_deserializer();

    wdx::value_path_request set_value_request(wdx::parameter_instance_path(), wdx::parameter_value::create(false));
    deserializer.deserialize(set_value_request, req->get_content());

    if(set_value_request.param_path == wdx::parameter_instance_path())
    {
        set_value_request.param_path = instance_path;
    }
    // FIXME: Operator == does not work as expected (not agnostic about '/' and '-')
    else if(wda_ipc::to_string(set_value_request.param_path) != wda_ipc::to_string(instance_path))
    {
        throw http_exception("Invalid ID \"" + wda_ipc::to_string(set_value_request.param_path)
                            + "\" in request data does not match the ID \"" + wda_ipc::to_string(instance_path)
                            + "\" given in the request URL.", http_status_code::bad_request);
    }

    auto core_response_handler = [
        operation,
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        instance_path,
        &core_frontend=operation->get_service_frontend(),
        set_value_request,
        doc_link=req->get_doc_link()
    ] (std::vector<wdx::set_parameter_response> &&responses) {
        if(responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        auto const &single_response = responses.at(0);

        verify_core_response(single_response, instance_path); // FIXME: Use verify_instance_path?
        bool is_deferred = single_response.status == core_status_code::wda_connection_changes_deferred;
        bool is_adjusted = single_response.status == core_status_code::success_but_value_adjusted;

        if(is_deferred)
        {
            set_parameters_deferred(operation, core_frontend, { set_value_request });
        }

        std::unique_ptr<response_i> resp;
        if (is_adjusted)
        {
            set_parameter_document document({{ "doc", doc_link }}, set_parameter_resource(set_value_request, 
                                                                                          single_response));
            resp = std::make_unique<response<set_parameter_document>>(http_status_code::ok, serializer, std::move(document));
        }
        else
        {
            auto http_code = is_deferred ? http_status_code::accepted : http_status_code::no_content;
            resp = std::make_unique<http::head_response>(http_code);
        }
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              set_parameter_values_by_path_connection_aware,
                              std::move(core_response_handler),
                              { set_value_request }, true);
}

future<unique_ptr<response_i>> operation::get_all_parameter_definitions(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    // Get filters like that for BETA parameters from query
    // And always filter out methods
    auto parameter_filter = extract_filters_from_query(
                                { 
                                    {"beta",        combine_with_beta_filter        },
                                    {"deprecated",  combine_with_deprecated_filter  },
                                    {"device",      combine_with_device_filter      },
                                    {"path",        combine_with_path_filter        },
                                    {"writeable",   combine_with_writeable_filter   },
                                    {"userSetting", combine_with_userSetting_filter }
                                },
                                req->get_filter_queries()
                          );
    return get_parameter_definitions_internal(operation, req, parameter_filter);
}

future<unique_ptr<response_i>> operation::get_parameter_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    vector<wdx::parameter_instance_path> instance_paths = { get_parameter_definition_id_from_request_url(req) };

    // in order to detect methods among parameters, we must pre-check the instance
    //verify_instance_path(operation->get_service_frontend(), instance_paths.at(0), false);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        instance_paths
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(instance_paths.size() != responses.size())
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        wdx::parameter_response response_parameter = responses.at(0);
        verify_instance_path(response_parameter, instance_paths.at(0), false, true);
        parameter_definition_data definition_data(response_parameter);

        // Create document
        parameter_definition_document document(base_path, query, {{ "doc", doc_link }}, parameter_definition_resource(definition_data));
        std::unique_ptr<response_i> resp = std::make_unique<response<parameter_definition_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameter_definitions_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

future<unique_ptr<response_i>> operation::get_monitoring_list_parameters(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    monitoring_list_id_t monitoring_list_id = get_monitoring_list_id_from_request_url(req);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        errors_as_attributes=req->get_errors_as_data_attributes_parameter(),
        page_limit,
        page_offset
    ] (wdx::monitoring_list_values_response &&core_response) {
        verify_core_response(core_response);
        vector<wdx::parameter_response> const all_parameter_responses = core_response.parameter_values;
        vector<wdx::parameter_response> const parameter_responses     = apply_paging(all_parameter_responses, page_limit, page_offset);
        std::unique_ptr<response_i> resp = create_parameters_response(parameter_responses, 
                                                                      serializer,
                                                                      errors_as_attributes, 
                                                                      base_path, query, doc_link,
                                                                      page_limit, page_offset, all_parameter_responses.size());
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_values_for_monitoring_list,
                              std::move(core_response_handler),
                              monitoring_list_id);
}

future<unique_ptr<response_i>> operation::get_contained_parameters_of_feature(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    std::string device;
    std::string feature_name;
    get_feature_id_parts_from_request_url(req, device, feature_name);

    // Get filters like that for BETA parameters from query
    // And always filter out methods
    auto parameter_filter = wdx::parameter_filter::only_feature(feature_name)
                          | wdx::parameter_filter::only_device(wdx::device_selector::specific(wda_ipc::from_string<wdx::device_id>(device)))
                          | extract_filters_from_query(
                                { 
                                    {"beta",       combine_with_beta_filter       },
                                    {"deprecated", combine_with_deprecated_filter },
                                    {"path",       combine_with_path_filter       }
                                },
                                req->get_filter_queries()
                            );
    return get_parameter_definitions_internal(operation, req, parameter_filter);
}

future<unique_ptr<response_i>> operation::redirect_parameter_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check parameter ID
    auto const parameter_id  = req->get_path_parameter(path_param_parameter_id);
    auto const instance_path = get_parameter_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, false, false);

    // Example: /wda/parameters/0-0-identity-ordernumber/definition
    std::string const original_url_part = "/parameters/" + parameter_id + "/definition";

    // Example: /wda/parameter-definitions/0-0-identity-ordernumber
    std::string const new_url_part      = "/parameter-definitions/" + parameter_id;

    std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                       req->get_request_uri().as_string(),
                                                       original_url_part, new_url_part);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::redirect_parameter_device(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check parameter ID
    auto const parameter_id  = req->get_path_parameter(path_param_parameter_id);
    auto const instance_path = get_parameter_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, false, false);

    // Example: /wda/parameters/0-0-identity-ordernumber/device
    std::string const original_url_part = "/parameters/" + parameter_id + "/device";

    // Example: /wda/devices/0-0
    std::string       device_path       = instance_path.device_path;
    std::replace(device_path.begin(), device_path.end(), '/', '-'); // FIXME: Workaround: Path contains '/'!
    std::string const new_url_part      = "/devices/" + device_path;

    std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                       req->get_request_uri().as_string(),
                                                       original_url_part, new_url_part);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::redirect_parameter_enum(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get path parameters
    auto const definition_id   = req->get_path_parameter(path_param_parameter_definition_id);
    auto const definition_path = get_parameter_definition_id_from_request_url(req);

    // Request definition to discover enum name
    auto core_response_handler = [
        req,
        definition_id,
        definition_path
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        auto const response_parameter = responses.at(0);
        verify_instance_path(response_parameter, definition_path, false, true);
        if(response_parameter.definition->value_type != wdx::parameter_value_types::enum_member)
        {
            throw http::http_exception("No enum definition available for parameter definition with ID \"" +
                                       definition_id + "\", associated parameter is not of type '" +
                                       wda_ipc::to_string(wdx::parameter_value_types::enum_member) + "'.",
                                       http::http_status_code::not_found);
        }

        std::string enum_name = response_parameter.definition->enum_name;
        std::transform(enum_name.begin(), enum_name.end(), enum_name.begin(), ::tolower);

        // Example: /wda/parameter-definitions/0-0-webserver-protocolls/enum
        std::string const original_url_part = "/parameter-definitions/" + definition_id + "/enum";

        // Example: /wda/enum-definitions/webservertransferprotocols
        std::string const new_url_part      = "/enum-definitions/" + enum_name;
        std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                           req->get_request_uri().as_string(),
                                                           original_url_part, new_url_part);
        return resp;
    };

    std::vector<wdx::parameter_instance_path> instance_paths({ definition_path });
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameter_definitions_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

namespace {

std::unique_ptr<response_i> create_parameters_response(std::vector<wdx::parameter_response>        parameter_responses,
                                                       serializer_i                         const &serializer,
                                                       bool                                 const  errors_as_attributes,
                                                       std::string                          const  request_path,
                                                       std::string                          const  request_query,
                                                       std::string                          const  request_doc_link,
                                                       unsigned                             const  page_limit,
                                                       unsigned                             const  page_offset,
                                                       unsigned                             const  page_element_max)
{
    parameter_collection_document document(request_path, request_query, {{ "doc", request_doc_link }}, std::move(parameter_responses), page_offset, page_limit, page_element_max);

    if(!errors_as_attributes && document.has_errors())
    {
        // throw an http exception
        vector<shared_ptr<data_error>> shared_error_ptrs;
        for(auto const &err : document.get_errors())
        {
            shared_error_ptrs.push_back(std::make_shared<data_error>(err));
        }
        throw data_exception("Failed to read parameter values", shared_error_ptrs);
    }

    // Create document
    return std::make_unique<response<parameter_collection_document>>(http_status_code::ok, serializer, std::move(document));
}





void set_parameters_deferred(operation_i                          *operation,
                             frontend_i                           &core_frontend,
                             std::vector<wdx::value_path_request>  deferred_parameter_requests)
{
    operation->set_deferred_handler([&core_frontend, deferred_parameter_requests=std::move(deferred_parameter_requests)] () {
        WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: set_parameter_values_by_path (deferred parameters");
        auto deferred_call = core_frontend.set_parameter_values_by_path(deferred_parameter_requests);
        deferred_call.set_notifier([deferred_parameter_requests] (std::vector<wdx::set_parameter_response> &&deferred_responses) {
            WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: set_parameter_values_by_path (deferred parameters");
            size_t i = 0;
            for(auto const &resp : deferred_responses)
            {
                if(resp.has_error())
                {
                    wc_log(error, "Failed to write deferred parameter: id=" + wda_ipc::to_string(deferred_parameter_requests.at(i).param_path) + "core status=" + wdx::to_string(resp.status));
                }
                ++i;
            }
        });
    });
}

future<std::unique_ptr<response_i>> get_parameter_definitions_internal(operation_i                 *operation,
                                                                       std::shared_ptr<request>     req,
                                                                       wdx::parameter_filter const &parameter_filter)
{
    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        errors_as_attributes=req->get_errors_as_data_attributes_parameter(),
        page_limit,
        page_offset
    ] (wdx::parameter_response_list_response &&core_response) {
        // Create document
        std::vector<parameter_definition_data> definitions;
        definitions.reserve(core_response.param_responses.size());
        for(auto const &param_response : core_response.param_responses)
        {
            definitions.emplace_back(param_response);
        }
        parameter_definition_collection_document document(base_path, query, {{ "doc", doc_link }}, std::move(definitions), page_offset, page_limit, core_response.total_entries);

        if(!errors_as_attributes && document.has_errors())
        {
            // throw an http exception
            vector<shared_ptr<data_error>> shared_error_ptrs;
            for(auto const &err : document.get_errors())
            {
                shared_error_ptrs.push_back(std::make_shared<data_error>(err));
            }
            throw data_exception("Failed to read parameter definitions", shared_error_ptrs);
        }

        // Create document
        return std::make_unique<response<parameter_definition_collection_document>>(http_status_code::ok, serializer, std::move(document));
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_parameter_definitions,
                              std::move(core_response_handler),
                              parameter_filter, static_cast<size_t>(page_offset), static_cast<size_t>(page_limit));
}

} // Anonymous namespace

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
