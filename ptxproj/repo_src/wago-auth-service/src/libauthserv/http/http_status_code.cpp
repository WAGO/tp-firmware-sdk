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
///  \brief    Implementation of http status code related functions.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/http_status_code.hpp"

#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {
//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
std::string to_string(http_status_code const status_code)
{
    auto const code_raw = static_cast<unsigned>(status_code);
    WC_STATIC_ASSERT(sizeof(code_raw) >= sizeof(status_code));
    return std::to_string(code_raw);
}


} // Namespace http
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
