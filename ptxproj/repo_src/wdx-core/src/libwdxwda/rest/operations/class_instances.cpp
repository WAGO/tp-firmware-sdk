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
///  \brief    Implementation of class instance related REST-API operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"
#include "utils.hpp"
#include "call_core_frontend.hpp"

#include "parameter_service_frontend_extended_i.hpp"
#include "wda_ipc/representation.hpp"

#include <wc/log.h>
#include <wc/assertion.h>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

future<unique_ptr<response_i>> operation::get_referenced_class_instances(operation_i * operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    auto response_promise = std::make_shared<wago::promise<std::unique_ptr<response_i>>>();
    wdx::parameter_instance_path ref_parameter_path = get_parameter_instance_id_from_request_url(req);

    unsigned int page_limit, page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    auto pending_core_response = operation->get_service_frontend().get_parameters_by_path({ ref_parameter_path });
    pending_core_response.set_exception_notifier([response_promise](auto &&e) {
        response_promise->set_exception(e);
    });
    pending_core_response.set_notifier([
        response_promise,
        operation,
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        page_limit, page_offset,
        ref_parameter_path
    ] (std::vector<wdx::parameter_response> &&responses) mutable {
        if(responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        wdx::parameter_response core_response = responses.at(0);
        verify_instance_path(core_response, ref_parameter_path, false, false);    

        // only instance identity ref parameters can have referenced instances
        if (core_response.value->get_type() != wdx::parameter_value_types::instance_identity_ref)
        {
            throw http::http_exception("No referenced instances available for parameter with ID \"" +
                                       wda_ipc::to_string(ref_parameter_path) + "\", parameter is not of type '" +
                                       wda_ipc::to_string(wdx::parameter_value_types::instance_identity_ref) + "'.",
                                       http::http_status_code::not_found);
        }

        // collect all instances and all classes for that instances
        std::vector<wdx::parameter_instance_path> ref_class_paths;
        std::vector<wdx::parameter_instance_path> ref_instance_paths;

        std::vector<wdx::parameter_value> ref_values = (core_response.value->get_rank() == wdx::parameter_value_rank::scalar)
                                                     ? std::vector<wdx::parameter_value>(1, *core_response.value)
                                                     : core_response.value->get_items();

        for (auto const &single_value : ref_values)
        {
            auto instance_path = single_value.get_instance_identity_ref();
            auto class_path    = instance_path.substr(0, instance_path.find_last_of('/'));
            ref_instance_paths.push_back(
                wdx::parameter_instance_path(instance_path, ref_parameter_path.device_path)
            );
            ref_class_paths.push_back(
                wdx::parameter_instance_path(class_path, ref_parameter_path.device_path)
            );
        }

        // for each referenced class find the instances that actually exist. 
        // if referenced instance is not available, set to null in response data!
        auto pending_instances_of_classes_response = operation->get_service_frontend().get_parameters_by_path(ref_class_paths);
        pending_instances_of_classes_response.set_exception_notifier([response_promise](auto &&e) {
            response_promise->set_exception(e);
        });
        pending_instances_of_classes_response.set_notifier([
            response_promise,
            &serializer, base_path, query, doc_link,
            page_limit, page_offset,
            ref_instance_paths, 
            ref_class_paths
        ](std::vector<wdx::parameter_response> &&class_instances) mutable {
            WC_ASSERT(ref_class_paths.size() == ref_instance_paths.size());
            if(ref_instance_paths.size() != class_instances.size())
            {
                const char* error_msg = "Internal error: request-response count doesn't match.";
                WC_FAIL(error_msg);
                throw http_exception(error_msg, http_status_code::internal_server_error);
            }

            for (size_t i = 0; i < ref_class_paths.size(); ++i)
            {
                auto const &ref_class_path            = ref_class_paths.at(i);
                auto const &instance_path_to_check    = ref_instance_paths.at(i);
                bool has_instantiations_value = (class_instances.at(i).value != nullptr) && 
                                                (class_instances.at(i).value->get_type() == wdx::parameter_value_types::instantiations);
                auto const existing_instances = has_instantiations_value 
                                              ? class_instances.at(i).value->get_instantiations() 
                                              : std::vector<wdx::class_instantiation>({ /* empty list as fallback */ });
                auto found = std::find_if(existing_instances.begin(),
                                          existing_instances.end(), 
                                          [ref_class_path, instance_path_to_check](wdx::class_instantiation existing_instance) {
                                              auto existing_instance_path = (ref_class_path.parameter_path + "/" + wda_ipc::to_string(existing_instance.id));
                                              return (instance_path_to_check.parameter_path == existing_instance_path);
                                          });
                if (found == existing_instances.end())
                {
                    // set to empty string is equivalent to not set
                    ref_instance_paths.at(i).parameter_path = "";
                }
            }

            std::vector<class_instance_data> all_instances;
            all_instances.reserve(ref_instance_paths.size());
            for(auto const &instance : ref_instance_paths)
            {
                if( !instance.parameter_path.empty())
                {
                    all_instances.emplace_back(class_instance_data(instance));
                }
            }
            
            std::vector<class_instance_data> instances = apply_paging(all_instances, page_limit, page_offset);
            class_instance_collection_document document(base_path, query, {{ "doc", doc_link }}, std::move(instances), page_offset, page_limit, all_instances.size());

            // Create document
            response_promise->set_value(std::make_unique<response<class_instance_collection_document>>(http_status_code::ok, serializer, std::move(document)));
        });
    });
    return response_promise->get_future();
}

future<unique_ptr<response_i>> operation::get_class_instances(operation_i * operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    wdx::parameter_instance_path class_parameter_path = get_parameter_instance_id_from_request_url(req);

    unsigned int page_limit, page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        page_limit, page_offset,
        class_parameter_path
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        wdx::parameter_response core_response = responses.at(0);
        verify_instance_path(core_response, class_parameter_path, false, false);    

        // only class parameters can have instances
        if (core_response.value->get_type() != wdx::parameter_value_types::instantiations)
        {
            throw http::http_exception("No instances available for parameter with ID \"" +
                                       wda_ipc::to_string(class_parameter_path) + "\", parameter is not of type '" +
                                       wda_ipc::to_string(wdx::parameter_value_types::instantiations) + "'.",
                                       http::http_status_code::not_found);
        }

        // Create document
        auto instantiations = core_response.value->get_instantiations();
        std::vector<class_instance_data> all_instances;
        all_instances.reserve(instantiations.size());
        for(auto const &instance : instantiations)
        {
            all_instances.emplace_back(class_instance_data(class_parameter_path, instance.id));
        }
        
        std::vector<class_instance_data> instances = apply_paging(all_instances, page_limit, page_offset);
        class_instance_collection_document document(base_path, query, {{ "doc", doc_link }}, std::move(instances), page_offset, page_limit, all_instances.size());

        // Create document
        return std::make_unique<response<class_instance_collection_document>>(http_status_code::ok, serializer, std::move(document));
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameters_by_path,
                              std::move(core_response_handler),
                              { class_parameter_path });
}

future<unique_ptr<response_i>> operation::get_class_instance(operation_i * operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    wdx::parameter_instance_path class_parameter_path = get_parameter_instance_id_from_request_url(req);
    wdx::instance_id_t           instance_no          = get_instance_number_from_request_url(req);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        instance_no,
        class_parameter_path
    ] (std::vector<wdx::parameter_response> &&responses) {
        if(responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        wdx::parameter_response core_response = responses.at(0);
        verify_instance_path(core_response, class_parameter_path, false, false);    

        // only class parameters can have instances
        if (core_response.value->get_type() != wdx::parameter_value_types::instantiations)
        {
            throw http::http_exception("No instances available for parameter with ID \"" +
                                       wda_ipc::to_string(class_parameter_path) + "\", parameter is not of type '" +
                                       wda_ipc::to_string(wdx::parameter_value_types::instantiations) + "'.",
                                       http::http_status_code::not_found);
        }

        // Create document
        for(auto const &instance : core_response.value->get_instantiations())
        {
            if (instance.id == instance_no)
            {
                class_instance_data instance_data(class_parameter_path, instance_no);
                // Create document and response
                class_instance_document document(base_path, query, {{ "doc", doc_link }}, class_instance_resource(instance_data));
                return std::make_unique<response<class_instance_document>>(http_status_code::ok, serializer, std::move(document));
            }
        }

        // Document is not created when instance not found
        throw http::http_exception("Instance with instance number \"" + wda_ipc::to_string(instance_no) + 
                                   "\" not found for class parameter with ID \"" +
                                   wda_ipc::to_string(class_parameter_path) + "\".",
                                   http::http_status_code::not_found);
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_parameters_by_path,
                              std::move(core_response_handler),
                              { class_parameter_path });
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
