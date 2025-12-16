//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Response interface for HTTP responses.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_HTTP_RESPONSE_I_HPP_
#define INC_WAGO_WDX_WDA_HTTP_RESPONSE_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/api.h"
#include "http_status_code.hpp"

#include <wc/structuring.h>

#include <string>
#include <map>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

using std::string;
using std::map;

/// An HTTP response is usually used to respond to an HTTP request 
/// (wago::wdx::wda::ttp::http_request_i). Is is able to provide 
/// response headers as well as a response content.
class response_i
{
    WC_INTERFACE_CLASS(response_i)

public:
    /// Get the HTTP status code to be used for the response
    /// 
    /// \return
    ///    The HTTP status code
    virtual http_status_code get_status_code() const = 0;

    /// Get the HTTP response header fields to be included in the
    /// response
    /// 
    /// \return
    ///    The response header fields with their values
    virtual map<string, string> const & get_response_header() const = 0;

    /// Get the content type of the response body
    ///
    /// \return
    ///    The content type of the response body
    virtual string const & get_content_type() const = 0;

    /// Get the content length of the response body
    ///
    /// \return
    ///    The content length of the response body
    virtual string const & get_content_length() const = 0;

    /// Discover if response has body content
    ///
    /// \return
    ///    True, if response has body content
    virtual bool           has_content() const = 0;

    /// Get the response body's content
    ///
    /// \return
    ///    The response body's content
    virtual string         get_content() const = 0;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_HTTP_RESPONSE_I_HPP_
//---- End of source file ------------------------------------------------------
