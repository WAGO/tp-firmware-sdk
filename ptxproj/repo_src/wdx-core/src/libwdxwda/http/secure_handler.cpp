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
///  \brief    HTTP secure request handler to force HTTPS.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "secure_handler.hpp"
#include "wago/wdx/wda/http/request_i.hpp"
#include "head_response.hpp"

#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

secure_handler::secure_handler(shared_ptr<request_handler_i> next_handler)
: handler_m(next_handler)
{

}

secure_handler::~secure_handler() noexcept = default;

void secure_handler::handle(std::unique_ptr<request_i> request) noexcept
{
    try
    {
        if(!request->is_https())
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
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
