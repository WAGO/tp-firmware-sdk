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
///  \brief    Implementation of monitoring list related REST-API operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"
#include "rest/create_monitoring_list_request.hpp"
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

future<std::unique_ptr<response_i>> include_monitoring_list_relationships(frontend_i                                  &core_frontend,
                                                                          http::http_status_code                       response_code,
                                                                          std::shared_ptr<request>                     req,
                                                                          std::string                           const &self_link,
                                                                          std::vector<std::vector<std::string>>        request_includes,
                                                                          monitoring_list_data                        &wrapped_monitoring_list_response);

} // Anonymous namespace

future<unique_ptr<response_i>> operation::get_monitoring_lists(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

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
    ] (wdx::monitoring_lists_response &&core_response) {
        verify_core_response(core_response);

        vector<monitoring_list_resource> all_monitoring_list_resources;
        for(auto &list: core_response.monitoring_lists)
        {
            monitoring_list_data wrapped_monitoring_list_response(list.id, list.timeout_seconds);
            monitoring_list_resource resource(std::move(wrapped_monitoring_list_response), {
                {"parameters", basic_relationship({
                    {"related", base_path + "/" + wda_ipc::to_string(list.id) + "/parameters"}
                })}
            }, {
                {"self", base_path + "/" + wda_ipc::to_string(list.id)}
            });
            all_monitoring_list_resources.push_back(resource);
        }

        // Create document
        vector<monitoring_list_resource> monitoring_list_resources = apply_paging(all_monitoring_list_resources, page_limit, page_offset);
        monitoring_list_collection_document document(query, query, {{ "doc", doc_link }}, std::move(monitoring_list_resources), page_offset, page_limit, all_monitoring_list_resources.size());
        std::unique_ptr<response_i> resp = std::make_unique<response<monitoring_list_collection_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_monitoring_lists,
                              std::move(core_response_handler));
}

future<unique_ptr<response_i>> operation::post_monitoring_list(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get request parameters
    std::vector<std::vector<std::string>> includes;
    req->get_include_parameters(includes);

    // Parse request body
    deserializer_i const &deserializer = req->get_deserializer();
    string const body = req->get_content();
    if(body.empty())
    {
        string message = "Request body missing. Please provide a valid content as part of request body.";
        throw http_exception(std::move(message), http_status_code::bad_request);
    }
    create_monitoring_list_request monitoring_list;
    deserializer.deserialize(monitoring_list, body);

    // Deny one-time lists without include for parameters
    auto inc_param_predicate = [](std::vector<std::string> const &inner_include){
        return (inner_include.size() == 1) && inner_include.at(0) == "parameters";
    };
    if((monitoring_list.timeout == 0U) && (std::find_if(includes.begin(), includes.end(), inc_param_predicate) == includes.end()))
    {
        string message = "Requested one-time monitoring list (timeout value zero) without include of parameters. "
                         "Please add include of parameters or use a timeout > 0 for a normal monitoring list.";
        throw http_exception(std::move(message), http_status_code::bad_request);
    }

    auto resp_promise = std::make_shared<promise<unique_ptr<response_i>>>();

    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: create_monitoring_list");
    auto core_future = operation->get_service_frontend().create_monitoring_list_with_paths(monitoring_list.parameters, monitoring_list.timeout);
    core_future.set_notifier([
        resp_promise,
        req,
        &core_frontend=operation->get_service_frontend(),
        monitoring_list=std::move(monitoring_list),
        includes
    ](wdx::monitoring_list_response &&core_monitoring_list_response) {
            WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: create_monitoring_list");

            verify_core_response(core_monitoring_list_response);

            // Build response
            monitoring_list_data wrapped_monitoring_list_response = {
                core_monitoring_list_response.monitoring_list.id, monitoring_list.timeout
            };
            string monitoring_list_id = wda_ipc::to_string(core_monitoring_list_response.monitoring_list.id);
            // TODO: May we reuse the already for check requested parameters for includes as optimization?
            auto include_parameters = include_monitoring_list_relationships(
                                                        core_frontend, 
                                                        http_status_code::created, 
                                                        req,
                                                        req->get_request_uri().get_path() + "/" + monitoring_list_id, 
                                                        includes,
                                                        wrapped_monitoring_list_response);
            include_parameters.set_notifier([resp_promise] (unique_ptr<response_i> &&resp) {
                resp_promise->set_value(std::move(resp));
            });
            include_parameters.set_exception_notifier([resp_promise] (auto e) {
                resp_promise->set_exception(e);
            });
    });
    return resp_promise->get_future();
}

future<unique_ptr<response_i>> operation::get_monitoring_list(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get request parameters
    std::vector<std::vector<std::string>> includes;
    req->get_include_parameters(includes);
    monitoring_list_id_t monitoring_list_id = get_monitoring_list_id_from_request_url(req);

    auto resp_promise = std::make_shared<promise<unique_ptr<response_i>>>();

    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: get_monitoring_list");
    auto core_future = operation->get_service_frontend().get_monitoring_list(monitoring_list_id);
    core_future.set_notifier([
        resp_promise,
        req,
        &core_frontend=operation->get_service_frontend(),
        includes
    ](wdx::monitoring_list_response &&core_monitoring_list_response) {
            WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: get_monitoring_list");

            verify_core_response(core_monitoring_list_response);

            // Build response
            monitoring_list_data wrapped_monitoring_list_response = {
                core_monitoring_list_response.monitoring_list.id, core_monitoring_list_response.monitoring_list.timeout_seconds
            };

            // TODO: May we reuse the already for check requested parameters for includes as optimization?
            auto include_parameters = include_monitoring_list_relationships(
                                                        core_frontend, 
                                                        http_status_code::ok, 
                                                        req,
                                                        req->get_request_uri().get_path(),
                                                        includes,
                                                        wrapped_monitoring_list_response);
            include_parameters.set_notifier([resp_promise] (unique_ptr<response_i> &&resp) {
                resp_promise->set_value(std::move(resp));
            });
            include_parameters.set_exception_notifier([resp_promise] (auto e) {
                resp_promise->set_exception(e);
            });
    });
    return resp_promise->get_future();
}

future<unique_ptr<response_i>> operation::delete_monitoring_list(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    auto core_response_handler = [] (wdx::delete_monitoring_list_response &&core_response) {
        verify_core_response(core_response); // TODO how to provide monitoring list?
        std::unique_ptr<response_i> resp = std::make_unique<http::head_response>(http_status_code::no_content);
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              delete_monitoring_list,
                              std::move(core_response_handler),
                              get_monitoring_list_id_from_request_url(req));
}

namespace {

future<std::unique_ptr<response_i>> include_monitoring_list_relationships(frontend_i                                  &core_frontend,
                                                                          http::http_status_code                       response_code,
                                                                          std::shared_ptr<request>                     req,
                                                                          std::string                           const &self_link,
                                                                          std::vector<std::vector<std::string>>        request_includes,
                                                                          monitoring_list_data                        &wrapped_monitoring_list_response)
{
    string const parameters_relationship_name = "parameters";
    map<string, bool> include_relationships = {
        {parameters_relationship_name, false}
    };
    
    for(auto relationship_path : request_includes)
    {
        WC_ASSERT(!relationship_path.empty());
        auto first_relationship_name = relationship_path.at(0);
        if(include_relationships.count(first_relationship_name) > 0)
        {
            include_relationships[first_relationship_name] = true;
        }
    }

    map<string, basic_relationship> relationships;
    for(auto & relationship : include_relationships) {
        map<string,string> links({
            {"related", self_link + "/" + relationship.first}
        });
        if(relationship.second)
        {
            // TODO: for now we only support "parameters". The following code may be subject to a refactoring
            //       to make it more generic
            if(relationship.first == parameters_relationship_name)
            {
                WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: get values for monitoring list");
                wdx::monitoring_list_values_response monitoring_list_parameter_response = core_frontend.get_values_for_monitoring_list(wrapped_monitoring_list_response.get_monitoring_list_id()).get();
                WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done");
                verify_core_response(monitoring_list_parameter_response);

                wrapped_monitoring_list_response.set_related_data(parameters_relationship_name, monitoring_list_parameter_response.parameter_values, req->get_errors_as_data_attributes_parameter());
            }

            vector<related_resource> data = wrapped_monitoring_list_response.get_related_data(relationship.first);
            relationships.emplace(relationship.first, basic_relationship(links, data));
        }
        else
        {
            relationships.emplace(relationship.first, basic_relationship(links));
        }
    }

    // FIXME: This is a solution to ensure the self link is either present in the data 
    //        section of the document or in the toplevel, depending on whether the resource
    //        has been created using a POST request or has been queries using a GET request...
    //        There might be a better way of handling this?
    map<string, string> data_links;
    if(response_code == http_status_code::created)
    {
        req->add_response_header("Location", self_link);
        data_links = {
            {"self", self_link}
        };
    }
    monitoring_list_resource resource(std::move(wrapped_monitoring_list_response), relationships, data_links);

    // Create document
    auto document = (response_code == http_status_code::created) 
                        ? monitoring_list_document({{"doc", req->get_doc_link()}}, std::move(resource)) 
                        : monitoring_list_document(self_link, req->get_request_uri().get_query(), {{"doc", req->get_doc_link()}}, std::move(resource));
    std::unique_ptr<response_i> resp = std::make_unique<response<monitoring_list_document>>(response_code, req->get_serializer(), std::move(document));
    return resolved_future(std::move(resp));
}

} // Anonymous namespace

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
