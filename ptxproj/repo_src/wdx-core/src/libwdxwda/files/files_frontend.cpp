//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    File-API frontend for WAGO Parameter Service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "files_frontend.hpp"
#include "file_operation.hpp"
#include "http/cors_handler.hpp"
#include "wago/wdx/wda/http/request_i.hpp"
#include "definitions.hpp"
#include "wago/wdx/wda/settings_store_i.hpp"

#include <wc/log.h>

#include <regex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace files {

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
files_frontend::files_frontend(std::string                                     const &service_base,
                               shared_ptr<settings_store_i>                    const  settings_store,
                               unauthorized<wdx::parameter_service_file_api_i> const  frontend)
: service_base_m(service_base)
, settings_store_m(settings_store)
, frontend_m(frontend)
{ }

files_frontend::~files_frontend() noexcept =default;

void files_frontend::handle(std::unique_ptr<request_i> request, auth::authentication_info auth_info) noexcept
{
    try
    {
        file_operation operation(frontend_m.authorize(user_data(auth_info.user_name)));
        std::string uri = request->get_request_uri().as_string();
        bool check_uri = uri.find(service_base_m) == 0;
        WC_ASSERT(check_uri);
        if(!check_uri)
        {
            wc_log(log_level_t::error, "Invalid service base in request (expected service base: " + service_base_m + ")");
            request->respond(http::head_response(http::http_status_code::internal_server_error));
            return;
        }
        auto expected_slash_pos = service_base_m.size();

        // /files
        // /files/abc
        // 0123456789
        std::string path = request->get_request_uri().get_path();
        bool is_service_base            =  path.length() == expected_slash_pos;
        bool is_service_base_plus_slash = (path.length() == (expected_slash_pos + 1)) && (path.at(expected_slash_pos) == '/');

        if (is_service_base || is_service_base_plus_slash)
        {
            std::vector<http::http_method> allowed_methods(std::begin(allowed_methods_service_base), std::end(allowed_methods_service_base));
            http::cors_handler::add_cors_response_headers(*request, allowed_methods, cors_allowed_headers_service_base, cors_exposed_headers_service_base);

            switch(request->get_method())
            {
                case http::http_method::options:
                    request->respond(*http::cors_handler::get_options_response(*request, allowed_methods));
                    break;
                case http::http_method::post:
                    operation.create_upload_id(std::move(request), get_upload_id_timeout());
                    break;
                default:
                    request->add_response_header("Allow", http::cors_handler::get_allow_header(allowed_methods).c_str());
                    request->respond(http::head_response(http::http_status_code::method_not_allowed));
            }
            return;
        } 

        WC_ASSERT(uri.at(expected_slash_pos) == '/');
        wdx::wdmm::file_id file_id = uri.substr(expected_slash_pos + 1);

        std::vector<http::http_method> allowed_methods(std::begin(allowed_methods_file_id), std::end(allowed_methods_file_id));
        http::cors_handler::add_cors_response_headers(*request, allowed_methods, cors_allowed_headers_file_id, cors_exposed_headers_file_id);
        switch(request->get_method())
        {
            case http::http_method::options:
                request->respond(*http::cors_handler::get_options_response(*request, allowed_methods));
                break;
            case http::http_method::head:
                operation.get_file_info(file_id, std::move(request));
                break;
            case http::http_method::get:
                operation.get_file_content(file_id, std::move(request));
                break;
            case http::http_method::put:
            case http::http_method::patch:
                operation.set_file_content(file_id, std::move(request));
                break;
            default:
                request->add_response_header("Allow", http::cors_handler::get_allow_header(allowed_methods).c_str());
                request->respond(http::head_response(http::http_status_code::method_not_allowed));
        }
    }
    catch(const std::exception& e)
    {
        WC_DEBUG_LOG("An unexpected internal server error on files frontend request handling (exception caught)");
        request->respond(http::head_response(http::http_status_code::internal_server_error));
    }
}

uint16_t files_frontend::get_upload_id_timeout() const
{
    char const * const option = settings_store_i::file_api_upload_id_timeout;
    auto timeout_value = std::stoul(settings_store_m->get_setting(option));
    WC_ASSERT_RETURN(timeout_value <= UINT16_MAX, UINT16_MAX);
    return static_cast<uint16_t>(timeout_value);
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
