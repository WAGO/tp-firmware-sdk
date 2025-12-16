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
///  \brief    Implementation of REST specific exception class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "core_exception.hpp"
#include "core_error.hpp"

#include <wc/structuring.h>
#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
core_exception::core_exception(core_error const &error)
: http::http_exception(error.get_message(), (error.get_core_status_code() != core_status_code::success 
                                            ? error.get_http_status_code() 
                                            : http::http_status_code::internal_server_error))
, error_m(error)
{
    WC_ASSERT(error.get_core_status_code() != core_status_code::success);
}

core_status_code core_exception::get_core_status_code() const
{
    return error_m.get_core_status_code();
}

domain_status_code core_exception::get_domain_status_code() const
{
    return error_m.get_domain_status_code();
}

std::string core_exception::get_title() const
{
    return error_m.get_title();
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
