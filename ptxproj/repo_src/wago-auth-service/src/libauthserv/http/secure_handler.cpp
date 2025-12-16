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
///  \brief    HTTP secure request handler to force HTTPS.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "secure_handler.hpp"
#include "wago/authserv/http/request_i.hpp"
#include "head_response.hpp"

#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

secure_handler::secure_handler(unique_ptr<request_handler_i> next_handler,
                               bool                          allow_local_http)
: handler_m(std::move(next_handler))
, allow_local_http_m(allow_local_http)
{

}

secure_handler::~secure_handler() noexcept = default;

secure_handler::secure_handler(secure_handler&&) noexcept = default;

secure_handler& secure_handler::operator=(secure_handler&&) noexcept = default;

void secure_handler::handle(std::unique_ptr<request_i> request) noexcept
{
    try
    {
        if(!request->is_https() && (!allow_local_http_m || !request->is_localhost()))
        {
            wc_log(warning, "Got request via unsecured HTTP");
            request->respond(http::head_response(http::http_status_code::upgrade_required));
        }
        else
        {
            handler_m->handle(std::move(request));
        }
    }
    catch(...)
    {
        WC_DEBUG_LOG("Internal server error on secure request handling (exception caught)");
        request->respond(http::head_response(http::http_status_code::internal_server_error));
    }
}


} // Namespace http
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
