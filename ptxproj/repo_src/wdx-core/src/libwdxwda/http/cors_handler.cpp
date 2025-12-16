//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of CORS helper functions.
///
///  \author   MaHE: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "cors_handler.hpp"
#include "head_response.hpp"
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

std::unique_ptr<response_i> cors_handler::get_options_response(request_i                      &request,
                                                               std::vector<http_method> const &allowed_methods)
{
    request.add_response_header("Allow", get_allow_header(allowed_methods).c_str());
    return std::make_unique<head_response>(http_status_code::no_content);
}

void cors_handler::add_cors_response_headers(request_i                      &request,
                                             std::vector<http_method> const &allowed_methods,
                                             char const allowed_headers[],
                                             char const exposed_headers[])
{
    auto origin = request.get_http_header("Origin");
    if(!origin.empty())
    {
        // TODO: how to handle these? Can we just ignore them if values have no effect on our
        //       response?
        //auto requested_methods = request.get_http_header("Access-Control-Request-Method");
        //auto requested_headers = request.get_http_header("Access-Control-Request-Headers");

        request.add_response_header("Vary", "Origin");
        request.add_response_header("Access-Control-Allow-Origin", origin);
        request.add_response_header("Access-Control-Max-Age", "86400");
        request.add_response_header("Access-Control-Allow-Credentials", "true");
        request.add_response_header("Access-Control-Allow-Methods", 
            get_allow_header(allowed_methods).c_str());
        request.add_response_header("Access-Control-Allow-Headers", allowed_headers);
        request.add_response_header("Access-Control-Expose-Headers", exposed_headers);
    }
}

std::string cors_handler::get_allow_header(std::vector<http_method> const &allowed_methods)
{
    std::string value;
    for(auto method : allowed_methods)
    {
        if (!value.empty())
        {
            value += ", ";
        }
        value += get_method_name(method);
    }
    return value;
}

} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
