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
///  \brief    Request handler interface for HTTP request handlers.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_HTTP_REQUEST_HANDLER_I_HPP_
#define INC_WAGO_WDX_WDA_HTTP_REQUEST_HANDLER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/api.h"

#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

class request_i;

/// Classes implementing this interface are able to handle HTTP requests.
/// Usually instances of such classes are used to handle requests provided by an
/// HTTP server.
class request_handler_i
{
    WC_INTERFACE_CLASS(request_handler_i)

public:
    /// Handle an incoming request.
    /// 
    /// \param request
    ///    The request to be handled. Implementations must take care of
    ///    responding the request, as they're given the ownership of that
    ///    request object.
    virtual void handle(std::unique_ptr<http::request_i> request) noexcept = 0;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace fcgi
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_HTTP_REQUEST_HANDLER_I_HPP_
//---- End of source file ------------------------------------------------------
