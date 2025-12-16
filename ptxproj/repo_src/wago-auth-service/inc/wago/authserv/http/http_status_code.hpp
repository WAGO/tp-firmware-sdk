//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2019-2023 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    HTTP Status Codes as listed in RFC 7231 (Section 6).
///            \see <a href="https://tools.ietf.org/html/rfc7231#section-6.2.1>RFC 7231 Section 6.2.1</a>
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_HTTP_HTTP_STATUS_CODE_HPP_
#define INC_WAGO_AUTHSERV_HTTP_HTTP_STATUS_CODE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"

#include <wc/std_includes.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

/// HTTP Status Codes according to https://tools.ietf.org/html/rfc7231#section-6
///
/// 100 Continue
/// 101 Switching Protocols
/// 200 OK
/// 201 Created
/// 202 Accepted
/// 203 Non-Authoritative Information
/// 204 No Content
/// 205 Reset Content
/// 206 Partial Content
/// 300 Multiple Choices
/// 301 Moved Permanently
/// 302 Found
/// 303 See Other
/// 304 Not Modified
/// 305 Use Proxy
/// 307 Temporary Redirect
/// 400 Bad Request
/// 401 Unauthorized
/// 402 Payment Required
/// 403 Forbidden
/// 404 Not Found
/// 405 Method Not Allowed
/// 406 Not Acceptable
/// 407 Proxy Authentication Required
/// 408 Request Timeout
/// 409 Conflict
/// 410 Gone
/// 411 Length Required
/// 412 Precondition Failed
/// 413 Payload Too Large
/// 414 URI Too Long
/// 415 Unsupported Media Type
/// 416 Range Not Satisfiable
/// 417 Expectation Failed
/// 418 I'm A Teapot
/// 426 Upgrade Required
/// 500 Internal Server Error
/// 501 Not Implemented
/// 502 Bad Gateway
/// 503 Service Unavailable
/// 504 Gateway Timeout
/// 505 HTTP Version Not Supported
enum class http_status_code: uint16_t
{
    continue_                     = 100,
    switching_protocols           = 101,

    ok                            = 200,
    created                       = 201,
    accepted                      = 202,
    non_authorative_information   = 203,
    no_content                    = 204,
    reset_content                 = 205,
    partial_content               = 206,

    multiple_choices              = 300,
    moved_permanently             = 301,
    found                         = 302,
    see_other                     = 303,
    not_modified                  = 304,
    use_proxy                     = 305,
    unused                        = 306,
    temporary_redirect            = 307,

    bad_request                   = 400,
    unauthorized                  = 401,
    payment_required              = 402,
    forbidden                     = 403,
    not_found                     = 404,
    method_not_allowed            = 405,
    not_acceptable                = 406,
    proxy_authentication_required = 407,
    request_timeout               = 408,
    conflict                      = 409,
    gone                          = 410,
    length_required               = 411,
    precondition_failed           = 412,
    payload_too_large             = 413,
    uri_too_long                  = 414,
    unsupported_media_type        = 415,
    range_not_satisfiable         = 416,
    expectattion_failed           = 417,
    im_a_teapot                   = 418,
    upgrade_required              = 426,

    internal_server_error         = 500,
    not_implemented               = 501,
    bad_gateway                   = 502,
    service_unavailable           = 503,
    gateway_timeout               = 504,
    http_version_not_supported    = 505
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

/// Get the string representation of an HTTP status code, e.g. "200" for 
/// http_status_code::ok.
/// 
/// \param status_code
///    The HTTP status code to represent as a string
/// \return
///    The string representation for the given HTTP status code
std::string to_string(http_status_code const status_code);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// Check whether an HTTP status is considered an error or not. Status codes
/// above or equal `400` are considered to be errorneous.
/// 
/// \param status_code
///    The HTTP status code to check
/// \return
///    True, if code is considered an error, false otherwise
constexpr bool is_http_error(http_status_code const status_code)
{
    return static_cast<unsigned>(status_code) >= 400;
}

/// Get verbose and human-readable text for an HTTP status code, e.g. "OK" for 
/// http_status_code::ok.
/// 
/// \param status_code
///    The code to get the text for
/// \return
///    The text for the given HTTP status code
constexpr char const * get_http_status_code_text(http_status_code const status_code)
{
    char const * http_status_code_text = "";
    switch(status_code)
    {
        case http_status_code::continue_:
            http_status_code_text = "Continue";
            break;
        case http_status_code::switching_protocols:
            http_status_code_text = "Switching Protocols";
            break;
        case http_status_code::ok:
            http_status_code_text = "OK";
            break;
        case http_status_code::created:
            http_status_code_text = "Created";
            break;
        case http_status_code::accepted:
            http_status_code_text = "Accepted";
            break;
        case http_status_code::non_authorative_information:
            http_status_code_text = "Non-Authoritative Information";
            break;
        case http_status_code::no_content:
            http_status_code_text = "No Content";
            break;
        case http_status_code::reset_content:
            http_status_code_text = "Reset Content";
            break;
        case http_status_code::partial_content:
            http_status_code_text = "Partial Content";
            break;
        case http_status_code::multiple_choices:
            http_status_code_text = "Multiple Choices";
            break;
        case http_status_code::moved_permanently:
            http_status_code_text = "Moved Permanently";
            break;
        case http_status_code::found:
            http_status_code_text = "Found";
            break;
        case http_status_code::see_other:
            http_status_code_text = "See Other";
            break;
        case http_status_code::not_modified:
            http_status_code_text = "Not Modified";
            break;
        case http_status_code::use_proxy:
            http_status_code_text = "Use Proxy";
            break;
        case http_status_code::unused:
            http_status_code_text = "(Unused)";
            break;
        case http_status_code::temporary_redirect:
            http_status_code_text = "Temporary Redirect";
            break;
        case http_status_code::bad_request:
            http_status_code_text = "Bad Request";
            break;
        case http_status_code::unauthorized:
            http_status_code_text = "Unauthorized";
            break;
        case http_status_code::payment_required:
            http_status_code_text = "Payment Required";
            break;
        case http_status_code::forbidden:
            http_status_code_text = "Forbidden";
            break;
        case http_status_code::not_found:
            http_status_code_text = "Not Found";
            break;
        case http_status_code::method_not_allowed:
            http_status_code_text = "Method Not Allowed";
            break;
        case http_status_code::not_acceptable:
            http_status_code_text = "Not Acceptable";
            break;
        case http_status_code::proxy_authentication_required:
            http_status_code_text = "Proxy Authentication Required";
            break;
        case http_status_code::request_timeout:
            http_status_code_text = "Request Timeout";
            break;
        case http_status_code::conflict:
            http_status_code_text = "Conflict";
            break;
        case http_status_code::gone:
            http_status_code_text = "Gone";
            break;
        case http_status_code::length_required:
            http_status_code_text = "Length Required";
            break;
        case http_status_code::precondition_failed:
            http_status_code_text = "Precondition Failed";
            break;
        case http_status_code::payload_too_large:
            http_status_code_text = "Payload Too Large";
            break;
        case http_status_code::uri_too_long:
            http_status_code_text = "URI Too Long";
            break;
        case http_status_code::unsupported_media_type:
            http_status_code_text = "Unsupported Media Type";
            break;
        case http_status_code::range_not_satisfiable:
            http_status_code_text = "Range Not Satisfiable";
            break;
        case http_status_code::expectattion_failed:
            http_status_code_text = "Expectation Failed";
            break;
        case http_status_code::im_a_teapot:
            http_status_code_text = "I'm A Teapot";
            break;
        case http_status_code::upgrade_required:
            http_status_code_text = "Upgrade Required";
            break;
        case http_status_code::internal_server_error:
            http_status_code_text = "Internal Server Error";
            break;
        case http_status_code::not_implemented:
            http_status_code_text = "Not Implemented";
            break;
        case http_status_code::bad_gateway:
            http_status_code_text = "Bad Gateway";
            break;
        case http_status_code::service_unavailable:
            http_status_code_text = "Service Unavailable";
            break;
        case http_status_code::gateway_timeout:
            http_status_code_text = "Gateway Timeout";
            break;
        case http_status_code::http_version_not_supported:
            http_status_code_text = "HTTP Version Not Supported";
            break;
        default:
            break;
    }

    return http_status_code_text;
}


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_HTTP_HTTP_STATUS_CODE_HPP_
//---- End of source file ------------------------------------------------------
