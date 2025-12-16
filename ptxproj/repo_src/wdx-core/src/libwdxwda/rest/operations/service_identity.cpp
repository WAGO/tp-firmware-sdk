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
///  \brief    Implementation of service identity related REST-API operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"

#include <wc/log.h>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

future<unique_ptr<response_i>> operation::get_service(operation_i *operation, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    serializer_i const &serializer = req->get_serializer();

    service_identity_resource resource(&operation->get_service_identity(), {
        {"devices", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/devices"}
        })},
        {"parameters", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/parameters"}
        })},
        {"methods", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/methods"}
        })},
        {"monitoring-lists", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/monitoring-lists"}
        })},
        {"features", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/features"}
        })},
        {"parameter-definitions", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/parameter-definitions"}
        })},
        {"method-definitions", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/method-definitions"}
        })},
        {"enum-definitions", basic_relationship({
            {"related", req->get_request_uri().get_path() + "/enum-definitions"}
        })}
    });

    // Create document
    service_identity_document document(req->get_request_uri().get_path(), req->get_request_uri().get_query(), {{ "doc", req->get_doc_link() }}, std::move(resource));
    std::unique_ptr<response_i> resp = std::make_unique<response<service_identity_document>>(http_status_code::ok, serializer, std::move(document));
    return resolved_future(std::move(resp));
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
