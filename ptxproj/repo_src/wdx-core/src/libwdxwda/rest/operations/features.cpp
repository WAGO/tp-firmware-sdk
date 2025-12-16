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
///  \brief    Implementation of feature related REST-API operations.
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

future<unique_ptr<response_i>> operation::get_features_of_device(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

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

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        device,
        page_limit,
        page_offset
    ] (std::vector<wdx::feature_list_response> &&core_responses) {
        if(core_responses.size() != 1)
        {
            const char* error_msg = "Internal error: request-response count doesn't match.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        // Create document
        vector<feature_resource> feature_resources;
        for(auto const &list_response : core_responses)
        {
            verify_core_response(list_response, device);
            for(auto const &feature_response : list_response.features)
            {
                wdx::feature_response resp(wdx::status_codes::success);
                resp.device_path = list_response.device_path;
                resp.feature     = feature_response;
                feature_resources.emplace_back(std::move(resp));
            }
        }

        auto feature_defs = apply_paging(feature_resources, page_limit, page_offset);
        feature_collection_document document(base_path, query, {{"doc", doc_link}}, std::move(feature_defs), page_offset, page_limit, feature_resources.size());
        std::unique_ptr<response_i> resp = std::make_unique<response<feature_collection_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_features,
                              std::move(core_response_handler),
                              {device_path});
}

future<unique_ptr<response_i>> operation::get_all_features(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

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
    ] (std::vector<wdx::feature_list_response> &&core_responses) {
        // Create document
        vector<feature_resource> feature_resources;
        for(auto const &list_response : core_responses)
        {
            for(auto const &feature_response : list_response.features)
            {
                wdx::feature_response resp(wdx::status_codes::success);
                resp.device_path = list_response.device_path;
                resp.feature     = feature_response;
                feature_resources.emplace_back(std::move(resp));
            }
        }

        auto feature_defs = apply_paging(feature_resources, page_limit, page_offset);
        feature_collection_document document(base_path, query, {{"doc", doc_link}}, std::move(feature_defs), page_offset, page_limit, feature_resources.size());
        std::unique_ptr<response_i> resp = std::make_unique<response<feature_collection_document>>(http_status_code::ok, serializer, std::move(document));
        return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_features_of_all_devices,
                              std::move(core_response_handler));
}

future<unique_ptr<response_i>> operation::get_feature(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    std::string device;
    std::string feature_name;
    get_feature_id_parts_from_request_url(req, device, feature_name);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        device,
        feature_name
     ] (wdx::feature_response &&core_response) {
      verify_core_response(core_response, device + "-" + feature_name);

      // Create document
      feature_document document(base_path, query, {{ "doc", doc_link }}, feature_resource(core_response));
      std::unique_ptr<response_i> resp = std::make_unique<response<feature_document>>(http_status_code::ok, serializer, std::move(document));
      return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_feature_definition,
                              std::move(core_response_handler),
                              device,
                              feature_name);
}

future<unique_ptr<response_i>> operation::get_included_features(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    unsigned page_limit;
    unsigned page_offset;
    req->get_pagination_parameters(page_limit, page_offset);

    wdx::device_path_t device;
    std::string        feature_name;
    get_feature_id_parts_from_request_url(req, device, feature_name);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        device,
        feature_name,
        page_limit,
        page_offset
     ] (wdx::feature_response &&core_response) {
      verify_core_response(core_response, device + "-" + feature_name);

      // Create document
      vector<feature_resource> feature_resources;
      for(auto const &feature_response : core_response.feature.includes)
      {
          wdx::feature_response resp(wdx::status_codes::success);
          resp.device_path = core_response.device_path;
          resp.feature     = feature_response;
          feature_resources.emplace_back(std::move(resp));
      }

      auto const size         = feature_resources.size();
      auto       feature_defs = apply_paging(std::move(feature_resources), page_limit, page_offset);
      feature_collection_document document(base_path, query, {{"doc", doc_link}}, std::move(feature_defs), page_offset, page_limit, size);
      std::unique_ptr<response_i> resp = std::make_unique<response<feature_collection_document>>(http_status_code::ok, serializer, std::move(document));
      return resp;
    };

    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_feature_definition,
                              std::move(core_response_handler),
                              device,
                              feature_name);
}

future<unique_ptr<response_i>> operation::redirect_feature(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Check device ID
    if(!req->has_path_parameter(path_param_device_id))
    {
        throw http_exception("No valid device ID given.", http_status_code::internal_server_error);
    }

    // Check feature name
    if(!req->has_path_parameter(path_param_feature_name))
    {
        throw http_exception("No valid feature name given.", http_status_code::internal_server_error);
    }

    // Example: /wda/devices/0-0/features/myfeature
    std::string original_url_part = "/devices/"  + req->get_path_parameter(path_param_device_id)
                                  + "/features/" + req->get_path_parameter(path_param_feature_name);

    // Example: /wda/features/0-0-myfeature
    std::string new_url_part      = "/features/"  + req->get_path_parameter(path_param_device_id) 
                                  + "-" 
                                  + req->get_path_parameter(path_param_feature_name);

    std::unique_ptr<response_i> resp = simple_redirect(req->get_method(),
                                                       req->get_request_uri().as_string(),
                                                       original_url_part, new_url_part);
    return resolved_future(std::move(resp));
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
