//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WEB-API frontend for WAGO Auth Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "web_frontend.hpp"
#include "wago/authserv/settings_store_i.hpp"
#include "settings_store.hpp"
#include "wago/authserv/http/request_i.hpp"
#include "wago/authserv/http/http_status_code.hpp"
#include "http/head_response.hpp"
#include "http/http_exception.hpp"
#include "system_abstraction.hpp"
#include "definitions.hpp"
#include "authorize.hpp"
#include "token.hpp"
#include "verify.hpp"
#include "password_change.hpp"
#include "response_helper.hpp"

#include <wc/log.h>

#include <stdexcept>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {


#if WDX_USER_MODEL_ACCOUNTMANAGEMENT
constexpr auto wdx_user_model = paramd_backend::wdx_user_model::account_management;
#elif WDX_USER_MODEL_PASSWORDMANAGEMENT
constexpr auto wdx_user_model = paramd_backend::wdx_user_model::password_management;
#else
#error WDX_USER_MODEL_* is not set. Either *_ACCOUNTMANAGEMENT or *_PASSWORDMANAGEMENT  must be set.
#endif


void preload_template(std::string const &file_path, std::shared_ptr<std::vector<std::string>> &template_cache);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
web_frontend::web_frontend(std::string                            const &service_file,
                           std::string                            const &service_name,
                           std::string                            const &service_base,
                           std::shared_ptr<settings_store_i>             settings_store,
                           wdx::parameter_service_frontend_i            &parameter_service_frontend,
                           std::shared_ptr<oauth_token_handler_i>        oauth_token_handler)
: service_base_m(service_base)
, settings_store_m(std::move(settings_store))
, pam_backend_m(std::make_shared<pam_backend>(service_file))
, paramd_backend_m(std::make_shared<paramd_backend>(wdx_user_model, parameter_service_frontend))
, authenticator_m(pam_backend_m, paramd_backend_m, service_base + endpoint_auth)
, oauth_token_handler_m(std::move(oauth_token_handler))
, html_templater_m(std::make_shared<response_helper>(service_name, settings_store_m))
{}

web_frontend::~web_frontend() noexcept = default;

request_handler_i & web_frontend::get_handler()
{
    return *this;
}

void web_frontend::handle(std::unique_ptr<request_i> unique_request) noexcept
{
    std::shared_ptr<request_i> request = std::move(unique_request);
    try
    {
        try
        {
            // Switch handling by endpoint
            WC_ASSERT(request->get_request_uri().get_path().substr(0, service_base_m.length()) == service_base_m);
            std::string const endpoint = request->get_request_uri().get_path().substr(service_base_m.length());
            WC_DEBUG_LOG("Handle request for \"" + endpoint + "\"");
            if(endpoint == endpoint_auth)
            {
                handle_authorize(request);
            }
            else if(endpoint == endpoint_token)
            {
                handle_token(*request);
            }
            else if(endpoint == endpoint_verify)
            {
                handle_verify(*request);
            }
            else if(endpoint == endpoint_password_change)
            {
                handle_password_change(request);
            }
            else
            {
                request->respond(http::head_response(http::http_status_code::not_found));
            }
        }
        catch(http::http_exception const &)
        {
            throw;
        }
        catch(std::exception const &e)
        {
            throw http::http_exception(e.what());
        }
        catch(...)
        {
            throw http::http_exception("Unknown exception occurred.");
        }
    }
    catch(http::http_exception const &e)
    {
        if(!request->is_responded())
        {
            std::string const error_message = e.get_title() + ": " + e.what();
            if(e.get_http_status_code() == http::http_status_code::internal_server_error)
            {
                std::string const prefix = "An unexpected internal server error on request handling (exception caught): ";
                WC_DEBUG_LOG(prefix + error_message);
            }
            else
            {
                wc_log(log_level_t::error, error_message);
            }
            request->respond(http::head_response(e.get_http_status_code()));
        }
    }
}

void web_frontend::handle_authorize(std::shared_ptr<request_i> request)
{
    // Implementation should satisfy RFC 6749:
    // https://www.rfc-editor.org/rfc/rfc6749
    switch(request->get_method())
    {
        case http::http_method::post:
        {
            if(!request->is_content_type_matching(post_content_type, post_content_subtype))
            {
                throw http::http_exception("Reject request with unsupported media type.",
                                           http::http_status_code::unsupported_media_type);
            }
        }
        WC_FALLTHROUGH;

        case http::http_method::get:
        {
            process_authorize_request(html_templater_m, settings_store_m, request, oauth_token_handler_m, authenticator_m);
        }
        break;

        default:
            request->respond(http::head_response(http::http_status_code::method_not_allowed));
    }

}

void web_frontend::handle_token(request_i &request)
{
    // Implementation should satisfy RFC 6749:
    // https://www.rfc-editor.org/rfc/rfc6749
    switch(request.get_method())
    {
        case http::http_method::post:
        {
            if(!request.is_content_type_matching(post_content_type, post_content_subtype))
            {
                request.respond(http::head_response(http::http_status_code::unsupported_media_type));
            }
            else
            {
                process_token_request(*settings_store_m, request, *oauth_token_handler_m, authenticator_m);
            }
        }
        break;

        default:
            request.respond(http::head_response(http::http_status_code::method_not_allowed));
    }
}

void web_frontend::handle_verify(request_i &request)
{
    // Implementation should satisfy RFC 7662:
    // https://www.rfc-editor.org/rfc/rfc7662

    // ONLY localhost is allowed to verify tokens.
    if(!request.is_localhost()) {
        request.respond(http::head_response(http::http_status_code::forbidden));
        return;
    }

    switch(request.get_method())
    {
        case http::http_method::post:
        {
            if(!request.is_content_type_matching(post_content_type, post_content_subtype))
            {
                request.respond(http::head_response(http::http_status_code::unsupported_media_type));
            }
            else if(!request.has_form_parameter(form_param_token))
            {
                wc_log(log_level_t::warning, "Got verify request without access token");
                request.respond(http::head_response(http::http_status_code::bad_request));
            }
            else
            {
                process_verify_request(request, *oauth_token_handler_m);
            }
        }
        break;

        default:
            request.respond(http::head_response(http::http_status_code::method_not_allowed));
    }
}

void web_frontend::handle_password_change(std::shared_ptr<request_i> request)
{
    switch(request->get_method())
    {
        case http::http_method::post:
        {
            if(!request->is_content_type_matching(post_content_type, post_content_subtype))
            {
                throw http::http_exception("Reject request with unsupported media type.",
                                           http::http_status_code::unsupported_media_type);
            }
        }
        WC_FALLTHROUGH;

        case http::http_method::get:
        {
            process_password_change_request(html_templater_m, request, authenticator_m);
        }
        break;

        default:
            request->respond(http::head_response(http::http_status_code::method_not_allowed));
    }
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
