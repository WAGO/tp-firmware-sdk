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
///  \brief    Implementation of OAuth error information.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "oauth_error.hpp"
#include "error_document.hpp"
#include "utils/url_encode.hpp"

#include <sstream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {
std::string encode_description_for_body(std::string const &text);
std::string build_url_message(std::string const &error,
                              std::string const &error_description,
                              std::string const &error_uri);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
oauth_error::oauth_error(error_type  type,
                         std::string description,
                         std::string uri)
: exception(description)
, description_m(std::move(description))
, uri_m(std::move(uri))
{
    switch(type)
    {
#ifdef NDEBUG
        // get warned in debug builds if not all types are handled in this switch
        default:
#endif
        case error_type::invalid_request:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "invalid_request";
            break;

        case error_type::invalid_client:
            // Currently there is no client authentication, but things may change later.
            // From https://www.rfc-editor.org/rfc/rfc6749#section-5.2 :
            // The authorization server MAY return an HTTP 401 (Unauthorized) status code to indicate which
            // HTTP authentication schemes are supported.  If the client attempted to authenticate via the
            // "Authorization" request header field, the authorization server MUST respond with an
            // HTTP 401 (Unauthorized) status code and include the "WWW-Authenticate" response header field
            // matching the authentication scheme used by the client.
            status_code_m = http::http_status_code::bad_request;
            type_m        = "invalid_client";
            break;

        case error_type::invalid_grant:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "invalid_grant";
            break;

        case error_type::unauthorized_client:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "unauthorized_client";
            break;

        case error_type::unsupported_grant_type:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "unsupported_grant_type";
            break;

        case error_type::invalid_scope:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "invalid_scope";
            break;

        case error_type::unsupported_response_type:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "unsupported_response_type";
            break;

        case error_type::access_denied:
            status_code_m = http::http_status_code::bad_request;
            type_m        = "access_denied";
            break;

        case error_type::server_error:
            status_code_m = http::http_status_code::internal_server_error;
            type_m        = "server_error";
            break;
    }
}

error_document oauth_error::get_as_document() const
{
    return error_document(status_code_m, type_m, encode_description_for_body(description_m), encode_url(uri_m));
}

std::string oauth_error::get_as_query() const
{
    return build_url_message(type_m, description_m, uri_m);
}


namespace {

std::string hexnumber(unsigned value)
{
    std::stringstream stream;
    stream << std::hex << value;
    return stream.str();
}

std::string encode_description_for_body(std::string const &text)
{
    std::string encoded;

    // https://www.rfc-editor.org/rfc/rfc6749#section-5.2
    // Values MUST NOT include characters outside the set %x20-21 / %x23-5B / %x5D-7E -> for description
    for(char const &c : text)
    {
        auto const char_value = static_cast<unsigned>(c);
        if(char_value < 0x20)  // Control characters
        {
            (void)char_value;  // just ignored.
        }
        if(char_value == 0x22) // Double quotes '"'
        {
            encoded += "\'";   // replaced by single quotes.
        }
        if(char_value == 0x5C) // Back slash '\'
        {
            encoded += "|";    // replaced by pipe sign .
        }
        if(char_value > 0x7E)  // DEL and non-ASCII characters
        {
            (void)char_value;  // just ignored.
        }
        else
        {
            encoded += c;
        }
    }

    return encoded;
}

std::string build_url_message(std::string const &error,
                              std::string const &error_description,
                              std::string const &error_uri)
{
    std::string message = "error=" + error; // No need to encode, error string was defined internally on construction
    if(!error_description.empty())
    {
        message += "&" "error_description=" + encode_url(error_description);
    }
    if(!error_uri.empty())
    {
        message += "&" "error_uri=" + encode_url(error_uri);
    }

    return message;
}
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
