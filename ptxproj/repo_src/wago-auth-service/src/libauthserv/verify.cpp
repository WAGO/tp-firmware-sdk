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
///  \brief    Implementation of the verify endpoint.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "verify.hpp"
#include "definitions.hpp"
#include "verify_document.hpp"
#include "http/head_response.hpp"

#include "wago/authserv/http/request_i.hpp"
#include "wago/authserv/oauth_token_handler_i.hpp"

#include <wc/log.h>

#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

void process_verify_request(http::request_i       &request,
                            oauth_token_handler_i &token_handler)
{
    try
    {
        // Verify token (currently this is limited to be an access token)
        auto const result = token_handler.validate_access_token(request.get_form_parameter(form_param_token));

        // TODO: Implement some kind of token scanning protection
        //       (e.g. penalty for recurring requests with invalid tokens)
        if(!result.valid || result.expired)
        {
            wc_log(log_level_t::warning, "Got verify request with invalid access token" +
                                          std::string(result.expired ? " (expired)" : ""));
            request.respond(verify_document());
        }
        else
        {
            wc_log(log_level_t::debug, "Successfully verified an access token");
            request.respond(verify_document(result.user_name, result.scopes, result.remaining_time_s));
        }
    }
    catch(...)
    {
        wc_log(log_level_t::error, "Caught unexpected exception on verify request");
        request.respond(http::head_response(http::http_status_code::internal_server_error));
    }
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
