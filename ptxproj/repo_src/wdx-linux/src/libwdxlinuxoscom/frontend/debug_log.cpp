//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of debug log helpers
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/debug_log.hpp"

#include "frontend_methods.hpp"
#include <wago/wdx/parameter_service_frontend_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

template <>
char const * debug_log_object_name<wdx::parameter_service_frontend_i>()
{ return "parameter_service_frontend"; }

template <>
char const * debug_log_method_name<wdx::parameter_service_frontend_i>(method_id_type method_id)
{
  return frontend_method_id_string(static_cast<frontend_method_id>(method_id)); //parasoft-suppress CERT_CPP-INT50-a-3 "Conversion of known integer values to enum is safe."
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
