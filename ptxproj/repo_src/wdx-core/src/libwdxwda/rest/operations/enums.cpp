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
///  \brief    Implementation of enum related REST-API operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"
#include "utils.hpp"
#include "call_core_frontend.hpp"

#include <wc/log.h>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

future<unique_ptr<response_i>> operation::get_all_enum_definitions(operation_i *operation, std::shared_ptr<request> req)
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
    ] (std::vector<wdx::enum_definition_response> &&core_responses) {

        auto enum_defs = apply_paging(core_responses, page_limit, page_offset);
        enum_definition_collection_document document(base_path, query, {{"doc", doc_link}}, std::move(enum_defs), page_offset, page_limit, core_responses.size());

        return std::make_unique<response<enum_definition_collection_document>>(http_status_code::ok, serializer, std::move(document));
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_all_enum_definitions,
                              std::move(core_response_handler));
}

future<unique_ptr<response_i>> operation::get_enum_definition(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    std::string enum_name = req->get_path_parameter(path_param_enum_definition_id);

    auto core_response_handler = [
        &serializer=req->get_serializer(),
        base_path=req->get_request_uri().get_path(),
        query=req->get_request_uri().get_query(),
        doc_link=req->get_doc_link(),
        enum_name
    ] (wdx::enum_definition_response &&core_response) {
        verify_core_response(core_response, enum_name);

        enum_definition_document document(base_path, query, {{"doc", doc_link}}, enum_definition_resource(std::move(core_response)));

        return std::make_unique<response<enum_definition_document>>(http_status_code::ok, serializer, std::move(document));
    };
    return CALL_CORE_FRONTEND(operation->get_service_frontend(),
                              get_enum_definition,
                              std::move(core_response_handler),
                              enum_name);
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
