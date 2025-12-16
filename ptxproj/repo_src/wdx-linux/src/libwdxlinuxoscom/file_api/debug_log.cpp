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

#include "file_api_methods.hpp"
#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

template <>
char const * debug_log_object_name<wdx::parameter_service_file_api_i>()
{ return "parameter_service_file_api"; }

template <>
char const * debug_log_method_name<wdx::parameter_service_file_api_i>(method_id_type method_id)
{
  return file_api_method_id_string(static_cast<file_api_method_id>(method_id)); //parasoft-suppress CERT_CPP-INT50-a-3 "Conversion of known integer values to enum is safe."
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
