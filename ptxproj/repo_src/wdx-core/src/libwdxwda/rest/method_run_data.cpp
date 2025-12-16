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
///  \brief    Implementation of method run data class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "method_run_data.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
method_run_data::method_run_data(std::string                                            const &method_id,
                                 uint32_t                                               const  timeout,
                                 std::shared_ptr<wdx::method_invocation_named_response> const  response)
: method_id_m(method_id)
, timeout_m(timeout)
, response_m(response)
{ }

std::string method_run_data::get_associated_method() const
{
    return method_id_m;
}

uint32_t method_run_data::get_timeout() const
{
    return timeout_m;
}

std::shared_ptr<wdx::method_invocation_named_response> method_run_data::get_response() const
{
    return response_m;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
