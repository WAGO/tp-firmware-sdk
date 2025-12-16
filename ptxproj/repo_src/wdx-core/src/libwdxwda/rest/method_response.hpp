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
///  \brief    Wrapper for parameter responses from the core api, which are 
///            used for methods.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_METHOD_RESPONSE_HPP_
#define SRC_LIBWDXWDA_REST_METHOD_RESPONSE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/responses.hpp"
#include <wc/compiler.h>

#include <string>
#include <map>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {


struct method_response : public wdx::parameter_response
{
    using wdx::parameter_response::parameter_response;
    method_response(parameter_response const &resp)
    : parameter_response(resp)
    { }
    method_response(parameter_response &&resp)
    : parameter_response(std::move(resp))
    { }
};


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_METHOD_RESPONSE_HPP_
//---- End of source file ------------------------------------------------------
