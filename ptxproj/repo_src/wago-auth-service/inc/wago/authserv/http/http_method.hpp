//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    HTTP methods as listed in RFC 7231 (Section 4) and RFC 5789.
///            \see <a href="https://tools.ietf.org/html/rfc7231#section-4.3">RFC 7231 Section 4.3</a>
///            \see <a href="https://tools.ietf.org/html/rfc5789">RFC 5789</a>
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_HTTP_HTTP_METHOD_HPP_
#define INC_WAGO_AUTHSERV_HTTP_HTTP_METHOD_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"

#include <wc/std_includes.h>
#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

/// HTTP Methods according to https://tools.ietf.org/html/rfc7231#section-4.3 and
/// https://tools.ietf.org/html/rfc5789
///
/// 7231-4.3.1. GET
/// 7231-4.3.2. HEAD
/// 7231-4.3.3. POST
/// 7231-4.3.4. PUT
/// 7231-4.3.5. DELETE
/// 7231-4.3.6. CONNECT
/// 7231-4.3.7. OPTIONS
/// 7231-4.3.8. TRACE
/// 5789        PATCH
enum class http_method: uint8_t
{
    get     = 0,
    head    = 1,
    post    = 2,
    put     = 3,
    delete_ = 4,
    connect = 5,
    options = 6,
    trace   = 7,
    patch   = 8,

    max_num = 8
};

constexpr http_method const all_http_methods[] = {
    http_method::get,
    http_method::head,
    http_method::post,
    http_method::put,
    http_method::delete_,
    http_method::connect,
    http_method::options,
    http_method::trace,
    http_method::patch
};

constexpr char const * const method_names[] = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "OPTIONS",
    "TRACE",
    "PATCH"
};
WC_STATIC_ASSERT(WC_ARRAY_LENGTH(method_names) == WC_ARRAY_LENGTH(all_http_methods));

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

/// Get the HTTP method for its string representation
/// 
/// \param method_name 
///    The name/string representation of an HTTP method, e.g. "GET" or "POST".
///    Only uppercase names are allowed.
/// \return 
///    The HTTP method represented by method_name
/// \throws 
///    wago::authserv::exception is thrown, when method_name is not matching 
///    any known HTTP method.
http_method get_method_from_name(char const * const method_name);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

/// Get an HTTP method's string representation, e.g. "GET" or "POST".
/// 
/// \param method
///    The method to get the name for
/// \return 
///    The HTTP method name for the given method
constexpr char const * get_method_name(http_method const method)
{
    return method_names[static_cast<unsigned>(method)];
}
WC_STATIC_ASSERT(get_method_name(http_method::max_num) != nullptr);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_HTTP_HTTP_METHOD_HPP_
//---- End of source file ------------------------------------------------------
