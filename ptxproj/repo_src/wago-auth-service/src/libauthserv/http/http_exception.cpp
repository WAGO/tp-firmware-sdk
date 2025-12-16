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
///  \brief    Implementation of HTTP specific exception class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "http_exception.hpp"

#include <wc/structuring.h>
#include <wc/assertion.h>

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
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
http_exception::http_exception(exception        const &e,
                               http_status_code const  http_status)
: http_exception(e.what(), http_status)
{ }

http_exception::http_exception(string           const &message,
                               http_status_code const  http_status)
: exception(message)
, http_status_m((is_http_error(http_status) || (http_status == http_status_code::accepted)) ? http_status : http_status_code::internal_server_error)
{
    // allow error codes and "202 Accepted", which may be trated as an error in some cases
    WC_ASSERT(is_http_error(http_status) || (http_status == http_status_code::accepted));
}

http_status_code http_exception::get_http_status_code() const
{
    return http_status_m;
}

string http_exception::get_title() const
{
    return get_http_status_code_text(http_status_m);
}


} // Namespace http
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
