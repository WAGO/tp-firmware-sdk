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
///  \brief    Implementation of general REST-API operations like redirects and
///            generic errors.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------


#include "rest/operation.hpp"
#include "utils.hpp"
#include "http/cors_handler.hpp"
#include "http/head_response.hpp"

#include <wc/log.h>
#include <wc/assertion.h>

namespace wago {
namespace wdx {
namespace wda {
namespace rest {

future<unique_ptr<response_i>> operation::options(operation_i *operation, std::shared_ptr<request> req)
{
    auto response = http::cors_handler::get_options_response(*req, operation->get_allowed_methods());
    return resolved_future(std::move(response));
}

future<unique_ptr<response_i>> operation::redirect_trailing_slash(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Use 307 in case of non-GET requests, as a client may always use a GET
    // when following a 308 redirect!
    http_status_code redirect_status_code = req->get_method() == http_method::get
        ? http_status_code::moved_permanently
        : http_status_code::temporary_redirect;

    // Get new location by removing the trailing slash
    string new_location = req->get_request_uri().get_path();
    new_location.pop_back();
    new_location += req->get_request_uri().get_query();

    WC_ASSERT(!new_location.empty());
    map<string, string> response_header = { { "Location" , std::move(new_location) } };

    std::unique_ptr<response_i> resp = std::make_unique<http::head_response>(redirect_status_code, response_header);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::redirect_to_lowercase(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    // Use 307 in case of non-GET requests, as a client may always use a GET
    // when following a 308 redirect!
    http_status_code redirect_status_code = req->get_method() == http_method::get
        ? http_status_code::moved_permanently
        : http_status_code::temporary_redirect;

    // Get new location by replacing the lowercase letters
    string new_location = req->get_request_uri().get_path();
    std::transform(new_location.begin(), new_location.end(), new_location.begin(), ::tolower);
    new_location += req->get_request_uri().get_query();
    WC_ASSERT(!new_location.empty());
    map<string, string> response_header = { { "Location" , std::move(new_location) } };

    std::unique_ptr<response_i> resp = std::make_unique<http::head_response>(redirect_status_code, response_header);
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::not_found(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    string message = "No resource found at \"" + req->get_request_uri().get_path() + "\".";
    std::unique_ptr<response_i> resp = get_error_response(req, std::make_shared<http_exception>(std::move(message), http_status_code::not_found));
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::not_implemented(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);

    string message = "URL endpoint \"" + req->get_request_uri().get_path() + "\" is not implemented yet.";
    std::unique_ptr<response_i> resp = get_error_response(req, std::make_shared<http_exception>(std::move(message), http_status_code::not_implemented));
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::not_acceptable(operation_i *, std::shared_ptr<request> req)
{
    string const content_type = req->get_http_header("Accept");
    string       message      = "Requested content type \"" + content_type + "\" is not supported. Only \"" + json_api_content_type + "\" is supported.";
    std::unique_ptr<response_i> resp = get_error_response(req, std::make_shared<http_exception>(std::move(message), http_status_code::not_acceptable));
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::not_supported_content_type(operation_i *, std::shared_ptr<request> req)
{
    string const content_type = req->get_content_type();
    string       message      = "Content type \"" + content_type + "\" is not supported. Only \"" + json_api_content_type + "\" is supported.";
    std::unique_ptr<response_i> resp = get_error_response(req, std::make_shared<http_exception>(std::move(message), http_status_code::unsupported_media_type));
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::method_not_allowed(operation_i *op, std::shared_ptr<request> req)
{
    string const method  = get_method_name(req->get_method());
    string       message = "Method \"" + method + "\" is not allowed on this resource.";
    std::unique_ptr<response_i> resp = get_error_response(req, std::make_shared<http_exception>(std::move(message), http_status_code::method_not_allowed));
    req->add_response_header("Allow", http::cors_handler::get_allow_header(op->get_allowed_methods()));
    return resolved_future(std::move(resp));
}

future<unique_ptr<response_i>> operation::get_head(operation_i *, std::shared_ptr<request> req)
{
    WC_DEBUG_LOG(string("Operation called: ") + __func__);
    auto response = std::make_unique<http::head_response>(http_status_code::ok);
    response->set_content_type(req->get_serializer().get_content_type());
    std::unique_ptr<response_i> resp = std::move(response);
    return resolved_future(std::move(resp));
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
