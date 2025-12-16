//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of HTTP method related functions.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/http_method.hpp"
#include "wago/wdx/wda/exception.hpp"

#include <wc/assertion.h>

#include <cstring>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
http_method get_method_from_name(char const * const method_name)
{
    http_method result = http_method::get;

    WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(http_method));
    for(unsigned i = 0; i <= static_cast<unsigned>(http_method::max_num); ++i)
    {
        auto const enum_value = static_cast<http_method>(i); //parasoft-suppress CERT_CPP-INT50-a-3 "Enum values of HTTP method are explicitly designed to be searched. Values are defined without gap and with an explicit end marker 'http_method::max_num'."
        if(strncmp(method_names[i], method_name, strlen(method_names[i]) + 1) == 0)
        {
            result = enum_value;
            break;
        }
        if(enum_value == http_method::max_num)
        {
            throw wago::wdx::wda::exception(string("Unknown HTTP method: ") + method_name);
        }
    }

    return result;
}


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
