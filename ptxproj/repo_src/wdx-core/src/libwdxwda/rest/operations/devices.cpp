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
///  \brief    Implementation of device related REST-API operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"
#include "utils.hpp"
#include "call_core_frontend.hpp"

#include "wda_ipc/representation.hpp"

#include <wc/log.h>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace {
constexpr char const * const kbus_related_feature = "LocalbusMasterKBus";
// FIXME: The RLB has been removed from the WDM 1.0.0 finally.
//        As a workaround, we'll add the API test feature as marker for a RLB test collection.
constexpr char const * const rlb_related_feature  = "APITest";
}

future<unique_ptr<response_i>> operation::get_all_devices(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    auto core_response_handler = [
            &serializer = req->get_serializer(),
            base_path   = req->get_request_uri().get_path(),
            query       = req->get_request_uri().get_query(),
            doc_link    = req->get_doc_link(),
            page_limit,
            page_offset
    ] (wdx::device_collection_response && collection_response) {

        vector<device_resource> all_devices;
        for(auto const &core_device : collection_response.devices)
        {
            all_devices.push_back(device_resource(core_device, {}, {
                {"self", base_path + "/" + wda_ipc::to_string(core_device.id)}
            }));
        }

        // Create document
        vector<device_resource> devices = apply_paging(all_devices, page_limit, page_offset);
        device_collection_document device_topology(base_path, query, {{"doc", doc_link}}, std::move(devices), page_offset, page_limit, all_devices.size());
        unique_ptr<response_i> resp = std::make_unique<response<device_collection_document>>(http_status_code::ok, serializer, std::move(device_topology));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_devices,
                              std::move(core_response_handler));
}

future<unique_ptr<response_i>> operation::get_device(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check device ID
    if(!req->has_path_parameter(path_param_device_id))
    {
        throw http_exception("No valid device ID given.", http_status_code::internal_server_error);
    }
    auto           device_path = req->get_path_parameter(path_param_device_id);
    wdx::device_id device;
    try
    {
        device = wda_ipc::from_string<wdx::device_id>(device_path);
    }
    catch(...)
    {
        throw http_exception("Unknown device ID.", http_status_code::not_found);
    }

    // Avoid confusion about prepared but inactive or special device collections on external API
    // Whitelist official local bus device collections and headstation (in root collection 0)
    // TODO: Remove RLB from whitelist
    if(    (device.device_collection_id != wdx::device_collections::rlb)
        && (device.device_collection_id != wdx::device_collections::kbus)
        && (device != wdx::device_id::headstation))
    {
        wdx::device_response workaround_response;
        workaround_response.status = core_status_code::unknown_device_collection;
        core_error error(workaround_response, device);
        throw core_exception(error);
    }

    auto core_response_handler = [
            &serializer = req->get_serializer(),
            base_path   = req->get_request_uri().get_path(),
            query       = req->get_request_uri().get_query(),
            doc_link    = req->get_doc_link(),
            device
    ] (wdx::device_response && device_resp) {
        verify_core_response(device_resp, device);
        // Create document
        device_document document(base_path, query, {{"doc", doc_link}}, device_resource(device_resp));
        std::unique_ptr<response_i> resp = std::make_unique<response<device_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_device,
                              std::move(core_response_handler),
                              device);
}

future<unique_ptr<response_i>> operation::get_subdevices_object(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check device ID
    if(!req->has_path_parameter(path_param_device_id))
    {
        throw http_exception("No valid device ID given.", http_status_code::internal_server_error);
    }
    auto           device_path = req->get_path_parameter(path_param_device_id);
    wdx::device_id device;
    try
    {
        device = wda_ipc::from_string<wdx::device_id>(device_path);
    }
    catch(...)
    {
        throw http_exception("Unknown device ID.", http_status_code::not_found);
    }

    auto resp_promise = std::make_shared<promise<unique_ptr<response_i>>>();

    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: get_features");
    auto collection_future = operation->get_service_frontend().get_features({device_path});
    collection_future.set_notifier([&serializer = req->get_serializer(),
                                      base_path  = req->get_request_uri().get_path(),
                                      query      = req->get_request_uri().get_query(),
                                      doc_link   = req->get_doc_link(),
                                      device,
                                      device_path,
                                      resp_promise](std::vector<wdx::feature_list_response> feature_lists){
        WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: get_features");

        WC_ASSERT(feature_lists.size() == 1); // we requested features for exactly one device

        if(feature_lists.at(0).has_error())
        {
            throw core_exception(core_error(feature_lists.at(0).status, "", 0,
                                            "subdevices", "", true, "device", device_path));
        }

        std::vector<std::string> device_collections;
        for(auto const & feature : feature_lists.at(0).features)
        {
            std::string feature_name = feature.name;

            if(feature_name == kbus_related_feature)
            {
                device_collections.emplace_back("kbus");
            }
            else if(feature_name == rlb_related_feature)
            {
                device_collections.emplace_back("rlb");
            }
        }
        auto device_id_str = wda_ipc::to_string(device);
        if( device_collections.empty() )
        {
            // Check for valid device to serve best matching error response
            http_exception exception("Device with ID \"" + device_id_str + "\" has no subdevices.", http_status_code::not_found);
            resp_promise->set_exception(std::make_exception_ptr(exception));
        }
        else
        {
            std::map<std::string, basic_relationship> collection_relationships;
            for(auto const & deviceCollectionName : device_collections)
            {
                collection_relationships.insert(
                    {deviceCollectionName, basic_relationship({
                        {"related", base_path + "/" + deviceCollectionName}
                    })}
                );
            }
            // need to provide self link
            std::map<std::string, std::string> data_links = {
                {"self", "/wda/devices/" + device_id_str + "/subdevices"}
            };
            generic_resource subdevices_resource("subdevices", device_id_str, nullptr, collection_relationships, data_links);

            // Create document
            generic_document document(base_path, query, {{"doc", doc_link}}, std::move(subdevices_resource));
            std::unique_ptr<response_i> resp = std::make_unique<response<generic_document>>(http_status_code::ok, serializer, std::move(document));
            resp_promise->set_value(std::move(resp));
        }
    });

    return resp_promise->get_future();
}

future<unique_ptr<response_i>> operation::get_subdevices_by_collection_name(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check device ID
    if(!req->has_path_parameter(path_param_device_id))
    {
        throw http_exception("No valid device ID given.", http_status_code::internal_server_error);
    }
    std::string device_path = req->get_path_parameter(path_param_device_id);
    wdx::device_id device;
    try
    {
        device = wda_ipc::from_string<wdx::device_id>(device_path);
    }
    catch(...)
    {
        throw http_exception(std::string("Unknown device ID."), http_status_code::not_found);
    }

    // Get collection name
    if(!req->has_path_parameter(path_param_collection_name))
    {
        throw http_exception("No valid collection name given.", http_status_code::internal_server_error);
    }
    string collection_name = req->get_path_parameter(path_param_collection_name);
    std::transform(collection_name.begin(), collection_name.end(), collection_name.begin(), [](unsigned char c){
        return std::tolower(c);
    });

    // Get pagination parameters
    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    auto resp_promise = std::make_shared<promise<unique_ptr<response_i>>>();

    std::string needed_feature;
    // FIXME: Convert function needed (device_collection_id_t)
    // TODO: check if numerical values have to be functional
    if(collection_name == "kbus" || collection_name == "1")
    {
        needed_feature = kbus_related_feature;
    }
    else if(collection_name == "rlb" || collection_name == "2")
    {
        needed_feature = rlb_related_feature;
    }
    else
    {
        needed_feature = "";
    }

    WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation: get_features");
    auto collection_future = operation->get_service_frontend().get_features({device_path});
    collection_future.set_notifier([ resp_promise,
                                    &core_frontend=operation->get_service_frontend(),
                                    &serializer=req->get_serializer(),
                                      errors_as_attributes=req->get_errors_as_data_attributes_parameter(),
                                      base_path=req->get_request_uri().get_path(),
                                      query=req->get_request_uri().get_query(),
                                      doc_link=req->get_doc_link(),
                                      collection_name,
                                      device_path,
                                      device,
                                      page_limit,
                                      page_offset,
                                      needed_feature ](std::vector<wdx::feature_list_response> feature_lists){
        WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Core operation done: get_features");

        WC_ASSERT(feature_lists.size() == 1); // we requested features for exactly one device

        // If device is not found or any other error occurs, the collection cannot exist.
        if(feature_lists.at(0).has_error())
        {
            throw core_exception(core_error(feature_lists.at(0).status, "", 0,
                                            "device collection", collection_name, true, "device", device_path));
        }

        bool collection_available = false;
        for(auto const &feature :feature_lists.at(0).features)
        {
            std::string feature_name = feature.name;

            if( !needed_feature.empty() && (feature_name == needed_feature) )
            {
                collection_available = true;
                break;
            }
        }
        if( !collection_available )
        {
            auto collection_exception = core_exception(core_error(core_status_code::unknown_device_collection, "", 0,
                                                                    "device collection", collection_name, true,
                                                                    "device", device_path));
            resp_promise->set_exception(std::make_exception_ptr(collection_exception));
        }
        else
        {
            auto core_response_handler = [
                    &serializer,
                    base_path,
                    query,
                    doc_link,
                    page_limit,
                    page_offset,
                    device
            ] (wdx::device_collection_response && collection_response) {
                verify_core_response(collection_response, device);

                vector<device_resource> all_devices;
                for(auto const &core_device : collection_response.devices)
                {
                    all_devices.push_back(device_resource(core_device, {}, {
                        {"self", "/wda/devices/" + wda_ipc::to_string(core_device.id)}
                    }));
                }

                // Create document
                vector<device_resource> devices = apply_paging(all_devices, page_limit, page_offset);
                device_collection_document device_topology(base_path, query, {{ "doc", doc_link }}, std::move(devices), page_offset, page_limit, all_devices.size());
                std::unique_ptr<response_i> resp = std::make_unique<response<device_collection_document>>(http_status_code::ok, serializer, std::move(device_topology));
                return resp;
            };

            auto get_subdevices = CALL_CORE_FRONTEND(core_frontend,
                                                      get_subdevices_by_collection_name,
                                                      std::move(core_response_handler),
                                                      collection_name);

            get_subdevices.set_notifier([resp_promise] (unique_ptr<response_i> &&resp) {
                resp_promise->set_value(std::move(resp));
            });
            get_subdevices.set_exception_notifier([resp_promise] (auto e) {
                resp_promise->set_exception(e);
            });
        }
    });

    return resp_promise->get_future();
}

future<unique_ptr<response_i>> operation::redirect_headstation(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                       req->get_request_uri().as_string(),
                                                       short_headstation, short_headstation_target);
    return resolved_future(std::move(resp));
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
