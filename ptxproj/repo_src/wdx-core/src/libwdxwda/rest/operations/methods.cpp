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
///  \brief    Implementation of method related REST-API operations.
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
#include <chrono>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace {

method_invocation_in_args get_method_arguments_from_request_body(std::shared_ptr<request> req);

future<std::unique_ptr<response_i>> get_all_method_arg_definitions_internal(operation_i                          *operation, 
                                                                            std::shared_ptr<request>              req, 
                                                                            method_arg_definition_data::arg_type  arg_type);

future<std::unique_ptr<response_i>> get_method_arg_definition_internal(operation_i                          *operation, 
                                                                       std::shared_ptr<request>              req, 
                                                                       method_arg_definition_data::arg_type  arg_type);

future<std::unique_ptr<response_i>> get_method_definitions_internal(operation_i                 *operation,
                                                                    std::shared_ptr<request>     req, 
                                                                    wdx::parameter_filter const &parameter_filter);
std::unique_ptr<response_i> create_methods_response(std::vector<wdx::parameter_response>        parameter_responses,
                                                    serializer_i                         const &serializer,
                                                    bool                                 const  errors_as_attributes,
                                                    std::string                          const  request_path,
                                                    std::string                          const  request_query,
                                                    std::string                          const  request_doc_link,
                                                    unsigned                             const  page_limit,
                                                    unsigned                             const  page_offset,
                                                    unsigned                             const  page_element_max);

std::unique_ptr<response<method_invocation_document>> create_method_response(std::shared_ptr<request>                 req,
                                                                             std::string                              self_link,
                                                                             std::string                        const run_id,
                                                                             std::shared_ptr<method_run_object>       run_object,
                                                                             http_status_code                   const http_status,
                                                                             bool                                     force_state_progress = false);

std::unique_ptr<response<method_invocation_collection_document>> create_method_runs_collection_response(std::shared_ptr<request>                               req,
                                                                                                        std::vector<std::shared_ptr<method_run_object>> const &run_objects_page,
                                                                                                        size_t                                                 page_offset,
                                                                                                        size_t                                                 page_limit,
                                                                                                        size_t                                                 page_element_max);

future<std::unique_ptr<response_i>> redirect_method_arg_enum_internal(operation_i                          *operation,
                                                                      std::shared_ptr<request>              req,
                                                                      method_arg_definition_data::arg_type  arg_type);

}

future<unique_ptr<response_i>> operation::get_methods(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    // Get filters like that for BETA parameters from query 
    // And always filter to only get methods
    auto parameter_filter = extract_filters_from_query(
                                {
                                    {"beta",       combine_with_beta_filter       },
                                    {"deprecated", combine_with_deprecated_filter },
                                    {"device",     combine_with_device_filter     },
                                    {"path",       combine_with_path_filter       }
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

        // FIXME: core lib currently gives us "methods_do_not_have_value" for every method instance.
        //        as the instance lookup should work regardless of provider registration, 
        //        we will convert this status into "success" for the moment.
        for(auto &res : core_response.param_responses)
        {
            if(res.status == wdx::status_codes::methods_do_not_have_value){
                res.status = wdx::status_codes::success;
            }
        }

        // Create document
        std::unique_ptr<response_i> resp = create_methods_response(core_response.param_responses,
                                                                    serializer,
                                                                    errors_as_attributes,
                                                                    base_path, query, doc_link,
                                                                    page_limit, page_offset, core_response.total_entries);
        return resp;
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_method_definitions,
                              std::move(core_response_handler),
                              parameter_filter, static_cast<size_t>(page_offset), static_cast<size_t>(page_limit));
}

future<unique_ptr<response_i>> operation::get_method(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    vector<wdx::parameter_instance_path> instance_paths = { get_method_instance_id_from_request_url(req) };

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
        method_response response_method = responses.at(0);
        verify_instance_path(response_method, instance_paths.at(0), true, false);

        method_document document(base_path, query, {{ "doc", doc_link }}, method_resource(response_method));
        std::unique_ptr<response_i> resp = std::make_unique<response<method_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameter_definitions_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

future<unique_ptr<response_i>> operation::get_method_runs(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check path and if method exists
    auto instance_path = get_method_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, true, false);

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    // This is currently not a core operation
    auto all_run_objects     = operation->get_run_manager()->get_runs(instance_path);
    auto page_of_run_objects = apply_paging(all_run_objects, page_limit, page_offset);

    std::unique_ptr<response_i> resp = create_method_runs_collection_response(req, page_of_run_objects, page_offset, page_limit, all_run_objects.size());
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::get_method_run(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check existence of the method instance
    auto instance_path = get_method_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, true, false);

    // check for the running invocation of that method
    if(!req->has_path_parameter(path_param_method_run_id))
    {
        throw http_exception("No valid method invocation ID given. Please provide a valid method invocation ID as part of the request URL.",
                              http_status_code::internal_server_error);
    }
    std::string run_id = req->get_path_parameter(path_param_method_run_id);

    // throws http 404|410 when no run found for id, no error handling needed here
    auto run_object = operation->get_run_manager()->get_run(instance_path, run_id);

    std::unique_ptr<response_i> resp = create_method_response(req, req->get_request_uri().get_path(), run_id, run_object, http_status_code::ok);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::post_method_run(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check max method runs as a soft border (parallel requests may overrun this limit)
    auto run_manager = operation->get_run_manager();
    if(run_manager->max_runs_reached())
    {
        throw http_exception("Reached internal maximum of parallel method runs.", http_status_code::internal_server_error);
    }

    // Get query parameter for sync/async response
    auto response_mode = req->get_result_behavior();

    auto  instance_path = get_method_instance_id_from_request_url(req);
    auto  base_path     = req->get_request_uri().get_path();
    auto  doc_link      = req->get_doc_link();
    auto  timeout_span  = operation->get_run_result_timeout_span();

    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: invoke_method_by_path");
    auto pending_core_response = operation->get_service_frontend().invoke_method_by_path(instance_path,
                                                                                          get_method_arguments_from_request_body(req));
    if(pending_core_response.has_value()) // Check for sync permanent errors
    {
        auto core_response = pending_core_response.get();
        verify_core_response(core_response, instance_path); // FIXME: Use verify_instance_path?
        pending_core_response = resolved_future(std::move(core_response));
    }

    // Select automatic if no specific behavior is requested
    if(response_mode == any)
    {
        response_mode = automatic;
    }

    // Wait until timeout to decide sync or async behavior
    if(response_mode == automatic)
    {
        auto status = pending_core_response.wait_for(std::chrono::seconds(3));
        switch(status)
        {
#ifdef NDEBUG
            // get warned in debug builds if not all future status codes are handled in this switch
            default:
#endif
            case std::future_status::deferred:
                WC_FAIL(("Unknown future status " + std::to_string(static_cast<unsigned>(status))).c_str());
                WC_FALLTHROUGH;
            case std::future_status::ready:
                response_mode = sync;
                break;
            case std::future_status::timeout:
                response_mode = async;
                break;
        }
    }

    // Wait for response async
    if(response_mode == async)
    {
        auto run_id = run_manager->add_run(instance_path, timeout_span, std::move(pending_core_response), [](std::string){
            WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: invoke_method_by_path");
        });
        auto run_object = run_manager->get_run(instance_path, run_id);
        auto self_link  = base_path + "/" + run_id;
        std::unique_ptr<response_i> resp = create_method_response(req, self_link, run_id, run_object, http_status_code::created, true);
        return resolved_future(std::move(resp));
    }

    // Wait for response sync
    {
        WC_ASSERT(response_mode == sync);
        auto resp_promise = std::make_shared<promise<unique_ptr<response_i>>>();
        run_manager->add_run(instance_path, timeout_span, std::move(pending_core_response), [
            req,
            resp_promise,
            instance_path,
            base_path,
            doc_link,
            run_manager
        ](std::string run_id){
            WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: invoke_method_by_path");

            auto run_object = run_manager->get_run(instance_path, run_id);
            auto self_link  = base_path + "/" + run_id;
            std::unique_ptr<response_i> resp = create_method_response(req, self_link, run_id, run_object, http_status_code::created);
            resp_promise->set_value(std::move(resp));
        });
        return resp_promise->get_future();
    }
}

future<unique_ptr<response_i>> operation::delete_method_run(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check existence of the method instance
    auto instance_path = get_method_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, true, false);

    // check for the running invocation of that method
    if(!req->has_path_parameter(path_param_method_run_id))
    {
        throw http_exception("No valid method invocation ID given.", http_status_code::internal_server_error);
    }
    std::string run_id = req->get_path_parameter(path_param_method_run_id);

    // throws http 404|410 when no run found for id, no error handling needed here
    operation->get_run_manager()->remove_run(instance_path, run_id);

    std::unique_ptr<response_i> resp =  std::make_unique<http::head_response>(http_status_code::no_content);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::get_all_method_definitions(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    // Get filters like that for BETA parameters from query 
    // And always filter to only get methods
    auto parameter_filter = extract_filters_from_query(
                                { 
                                    {"beta",       combine_with_beta_filter       },
                                    {"deprecated", combine_with_deprecated_filter },
                                    {"device",     combine_with_device_filter     },
                                    {"path",       combine_with_path_filter       }
                                },
                                req->get_filter_queries()
                            );
    return get_method_definitions_internal(operation, req, parameter_filter);
}

future<unique_ptr<response_i>> operation::get_method_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    vector<wdx::parameter_instance_path> instance_paths = { get_method_definition_id_from_request_url(req) };

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
        verify_instance_path(responses.at(0), instance_paths.at(0), true, true);

        method_definition_data response_method = responses.at(0);

        method_definition_document document(base_path, query, {{ "doc", doc_link }}, method_definition_resource(response_method));
        std::unique_ptr<response_i> resp = std::make_unique<response<method_definition_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameter_definitions_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

future<unique_ptr<response_i>> operation::get_all_method_inarg_definitions(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    return get_all_method_arg_definitions_internal(operation, std::move(req), method_arg_definition_data::arg_type::in);
}

future<unique_ptr<response_i>> operation::get_method_inarg_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    return get_method_arg_definition_internal(operation, std::move(req), method_arg_definition_data::arg_type::in);
}

future<unique_ptr<response_i>> operation::get_all_method_outarg_definitions(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    return get_all_method_arg_definitions_internal(operation, std::move(req), method_arg_definition_data::arg_type::out);
}

future<unique_ptr<response_i>> operation::get_method_outarg_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    return get_method_arg_definition_internal(operation, std::move(req), method_arg_definition_data::arg_type::out);
}

future<unique_ptr<response_i>> operation::get_contained_methods_of_feature(operation_i *operation, std::shared_ptr<request> req)
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
    return get_method_definitions_internal(operation, req, parameter_filter);
}

future<unique_ptr<response_i>> operation::redirect_method_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check parameter ID
    auto const method_id     = req->get_path_parameter(path_param_method_id);
    auto const instance_path = get_method_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, true, false);

    // Example: /wda/parameters/0-0-firmwareupdate-start/definition
    std::string const original_url_part = "/methods/" + method_id + "/definition";

    // Example: /wda/parameter-definitions/0-0-firmwareupdate-start
    std::string const new_url_part      = "/method-definitions/" + method_id;

    std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                       req->get_request_uri().as_string(),
                                                       original_url_part, new_url_part);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::redirect_method_device(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check parameter ID
    auto const method_id     = req->get_path_parameter(path_param_method_id);
    auto const instance_path = get_method_instance_id_from_request_url(req);
    verify_instance_path(operation->get_service_frontend(), instance_path, true, false);

    // Example: /wda/methods/0-0-firmwareupdate-start/device
    std::string const original_url_part = "/methods/" + method_id + "/device";

    // Example: /wda/devices/0-0
    std::string       device_path       = instance_path.device_path;
    std::replace(device_path.begin(), device_path.end(), '/', '-'); // FIXME: Workaround: Path contains '/'!
    std::string const new_url_part      = "/devices/" + device_path;

    std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                       req->get_request_uri().as_string(),
                                                       original_url_part, new_url_part);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::redirect_method_inarg_enum(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    method_arg_definition_data::arg_type arg_type = method_arg_definition_data::arg_type::in;
    return redirect_method_arg_enum_internal(operation, std::move(req), arg_type);
}

future<unique_ptr<response_i>> operation::redirect_method_outarg_enum(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    method_arg_definition_data::arg_type arg_type = method_arg_definition_data::arg_type::out;
    return redirect_method_arg_enum_internal(operation, std::move(req), arg_type);
}


namespace {

method_invocation_in_args get_method_arguments_from_request_body(std::shared_ptr<request> req)
{
    deserializer_i const &deserializer = req->get_deserializer();
    string const body = req->get_content();
    if(body.empty())
    {
        string message = "Request body missing. Please provide a valid content as part of request body.";
        throw http_exception(std::move(message), http_status_code::bad_request);
    }
    method_invocation_in_args in_arguments;
    deserializer.deserialize(in_arguments, body);
    return in_arguments;
}

future<std::unique_ptr<response_i>> get_all_method_arg_definitions_internal(operation_i                         *operation,
                                                                            std::shared_ptr<request>             req, 
                                                                            method_arg_definition_data::arg_type arg_type)
{
    vector<wdx::parameter_instance_path> instance_paths = { get_method_definition_id_from_request_url(req) };

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        instance_paths,
        page_limit,
        page_offset,
        arg_type
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(instance_paths.size() != responses.size())
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        verify_instance_path(responses.at(0), instance_paths.at(0), true, true);

        // we need responses in vector of our own type
        method_definition_data response_method = responses.at(0);
        std::vector<method_arg_definition_data> method_arg_defs;
        auto const &response_method_args = (arg_type == method_arg_definition_data::arg_type::in)
                                         ? response_method.get_method_definition()->in_args
                                         : response_method.get_method_definition()->out_args;
        method_arg_defs.reserve(response_method_args.size());
        for(auto const &arg_def_resp : response_method_args) {
            method_arg_defs.emplace_back(responses.at(0), arg_def_resp.name, arg_type);
        }

        method_arg_definition_collection_document document(base_path, query, {{"doc", doc_link}}, std::move(method_arg_defs), page_offset, page_limit, method_arg_defs.size());
        return std::make_unique<response<method_arg_definition_collection_document>>(http_status_code::ok, serializer, std::move(document));
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameter_definitions_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

future<std::unique_ptr<response_i>> get_method_arg_definition_internal(operation_i                          *operation, 
                                                                       std::shared_ptr<request>              req, 
                                                                       method_arg_definition_data::arg_type  arg_type)
{
    vector<wdx::parameter_instance_path> instance_paths = { get_method_definition_id_from_request_url(req) };
    std::string arg_name = (arg_type == method_arg_definition_data::arg_type::in)
                         ? req->get_path_parameter(path_param_method_inarg_name)
                         : req->get_path_parameter(path_param_method_outarg_name);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        instance_paths,
        arg_type,
        arg_name
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(instance_paths.size() != responses.size())
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        verify_instance_path(responses.at(0), instance_paths.at(0), true, true);

        // we need responses in vector of our own type
        method_definition_data response_method = responses.at(0);
        std::unique_ptr<method_arg_definition_data> arg_def_data;
        try
        {
            arg_def_data = std::make_unique<method_arg_definition_data>(responses.at(0), arg_name, arg_type);
        }
        catch(std::out_of_range const&)
        {
            // no argument found -> error 404
            throw http_exception("Method argument with name \"" + arg_name + "\" does not exist.", http_status_code::not_found);
        }
        method_arg_definition_document document(base_path, query, {{"doc", doc_link}}, method_arg_definition_resource(std::move(*arg_def_data)));
        return std::make_unique<response<method_arg_definition_document>>(http_status_code::ok, serializer, std::move(document));
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameter_definitions_by_path,
                              std::move(core_response_handler),
                              std::move(instance_paths));
}

future<std::unique_ptr<response_i>> get_method_definitions_internal(operation_i                    *operation, 
                                                                    std::shared_ptr<request>        req, 
                                                                    wdx::parameter_filter    const &parameter_filter)
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
        page_limit,
        page_offset
    ] (wdx::parameter_response_list_response &&core_response) {

        // TODO: is deduction of string message by this type appropriate?
        verify_core_response(core_response);

        // we need responses in vector of our own type
        std::vector<method_definition_data> method_defs;
        method_defs.insert(method_defs.begin(), core_response.param_responses.begin(), core_response.param_responses.end());

        method_definition_collection_document document(base_path, query, {{"doc", doc_link}}, std::move(method_defs), page_offset, page_limit, core_response.total_entries);

        if(document.has_errors())
        {
            // throw an http exception
            vector<shared_ptr<data_error>> shared_error_ptrs;
            for(auto const &err : document.get_errors())
            {
                shared_error_ptrs.push_back(std::make_shared<data_error>(err));
            }
            throw data_exception("Failed to retrieve method definitions", shared_error_ptrs);
        }

        // Create document
        return std::make_unique<response<method_definition_collection_document>>(http_status_code::ok, serializer, std::move(document));
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_method_definitions,
                              std::move(core_response_handler),
                              parameter_filter, static_cast<size_t>(page_offset), static_cast<size_t>(page_limit));
}

std::unique_ptr<response_i> create_methods_response(std::vector<wdx::parameter_response>        parameter_responses,
                                                    serializer_i                         const &serializer,
                                                    bool                                 const  errors_as_attributes,
                                                    std::string                          const  request_path,
                                                    std::string                          const  request_query,
                                                    std::string                          const  request_doc_link,
                                                    unsigned                             const  page_limit,
                                                    unsigned                             const  page_offset,
                                                    unsigned                             const  page_element_max)
{
    std::vector<method_response> method_responses;
    method_responses.insert(method_responses.begin(), parameter_responses.begin(), parameter_responses.end());
    method_collection_document document(request_path, request_query, {{"doc", request_doc_link}}, std::move(method_responses), page_offset, page_limit, page_element_max);

    if(!errors_as_attributes && document.has_errors())
    {
        // throw an http exception
        vector<shared_ptr<data_error>> shared_error_ptrs;
        for(auto const &err : document.get_errors())
        {
            shared_error_ptrs.push_back(std::make_shared<data_error>(err));
        }
        throw data_exception("Failed to retrieve method instances", shared_error_ptrs);
    }

    // Create document
    return std::make_unique<response<method_collection_document>>(http_status_code::ok, serializer, std::move(document));
}

std::unique_ptr<response<method_invocation_document>> create_method_response(std::shared_ptr<request>                 req,
                                                                             std::string                              self_link,
                                                                             std::string                        const run_id,
                                                                             std::shared_ptr<method_run_object>       run_object,
                                                                             http_status_code                   const http_status,
                                                                             bool                                     force_state_progress)
{
    if(http_status == http_status_code::created)
    {
        req->add_response_header("Location", self_link);
    }
    map<string, string> data_links = {
        {"self", self_link}
    };
    method_run_data run_data(wda_ipc::to_string(run_object->method), run_object->get_timeout_left(), force_state_progress ? nullptr : run_object->invocation_response);
    method_invocation_resource invocation("runs", run_id, std::move(run_data), {}, std::move(data_links));

    // Create document
    method_invocation_document invocation_document(
        http_status == http_status_code::ok ? self_link : "", // Else case is "created", TODO: Replace by generic document self link
        "",
        {{"doc", req->get_doc_link()}},
        std::move(invocation));
    return std::make_unique<response<method_invocation_document>>(http_status, req->get_serializer(), std::move(invocation_document));
}

std::unique_ptr<response<method_invocation_collection_document>> create_method_runs_collection_response(std::shared_ptr<request>                               req,
                                                                                                        std::vector<std::shared_ptr<method_run_object>> const &run_objects_page,
                                                                                                        size_t                                                 page_offset,
                                                                                                        size_t                                                 page_limit,
                                                                                                        size_t                                                 page_element_max)
{
    // convert run objects to method invocation resources
    std::vector<method_invocation_resource> invocations;
    for(auto const &run_object : run_objects_page)
    {
        map<string, string> data_links = {
            {"self", req->get_request_uri().get_path() + "/" + run_object->get_id()}
        };
        method_run_data run_data(wda_ipc::to_string(run_object->method), run_object->get_timeout_left(), run_object->invocation_response);
        method_invocation_resource invocation("runs", run_object->get_id(), std::move(run_data), {}, std::move(data_links));

        invocations.push_back(invocation);
    }

    // create response document
    method_invocation_collection_document invocations_document(
        req->get_request_uri().get_path(), req->get_request_uri().get_query(), {{"doc", req->get_doc_link()}}, 
        std::move(invocations), page_offset, page_limit, page_element_max
    );
    return std::make_unique<response<method_invocation_collection_document>>(http_status_code::ok, req->get_serializer(), std::move(invocations_document));
}

future<std::unique_ptr<response_i>> redirect_method_arg_enum_internal(operation_i                          *operation,
                                                                      std::shared_ptr<request>              req,
                                                                      method_arg_definition_data::arg_type  arg_type)
{
    // Get path parameters
    auto const definition_id   = req->get_path_parameter(path_param_method_definition_id);
    auto const definition_path = get_method_definition_id_from_request_url(req);

    std::string arg_name = (arg_type == method_arg_definition_data::arg_type::in)
                         ? req->get_path_parameter(path_param_method_inarg_name)
                         : req->get_path_parameter(path_param_method_outarg_name);

    // Request definition to discover enum name
    auto core_response_handler = [
        req,
        definition_id,
        definition_path,
        arg_type,
        arg_name
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        auto const response_method = responses.at(0);
        verify_instance_path(response_method, definition_path, true, true);

        method_definition_data method_def_data(response_method);
        std::unique_ptr<method_arg_definition_data> arg_def_data;
        try
        {
            arg_def_data = std::make_unique<method_arg_definition_data>(response_method, arg_name, arg_type);
        }
        catch(std::out_of_range const&)
        {
            // no argument found -> error 404
            throw http_exception("Method argument with name \"" + arg_name + "\" does not exist.", http_status_code::not_found);
        }
        std::string const direction = (arg_type == method_arg_definition_data::arg_type::in) ? "in" : "out";
        if(arg_def_data->get_argument_definition()->value_type != wdx::parameter_value_types::enum_member)
        {
            throw http::http_exception("No enum definition available for method argument \"" + arg_name + "\" on "
                                       "method definition with ID \"" + definition_id + "\", "
                                       "associated " + direction + "-argument is not of type '" +
                                       wda_ipc::to_string(wdx::parameter_value_types::enum_member) + "'.",
                                       http::http_status_code::not_found);
        }

        std::string enum_name = arg_def_data->get_argument_definition()->enum_name;
        std::transform(enum_name.begin(), enum_name.end(), enum_name.begin(), ::tolower);

        // Example: /wda/method-definitions/0-0-some-method/<in|out>args/arg_name/enum
        std::string const original_url_part = "/method-definitions/" + definition_id +
                                              "/" + direction + "args/" + arg_name + "/enum";

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

} // Anonymous namespace

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
