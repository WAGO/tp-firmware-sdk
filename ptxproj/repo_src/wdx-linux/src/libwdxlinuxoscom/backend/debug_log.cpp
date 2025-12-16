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

#include "backend_methods.hpp"
#include <wago/wdx/parameter_service_backend_extended_i.hpp>
#include "remote_backend_i.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

// parasoft-begin-suppress CERT_CPP-INT50-a-3 "Conversion of known integer values to enum is safe."

#define PROVIDER_DEBUG_LOG(_name_, _enum_name_)\
  template <>\
  char const * debug_log_object_name<wdx::_name_##_i>()\
  { return #_name_; }\
  template <>\
  char const * debug_log_method_name<wdx::_name_##_i>(method_id_type method_id)\
  {\
    return (_enum_name_##_string(static_cast<_enum_name_>(method_id)));\
  }

// List of interfaces we actually use over IPC
PROVIDER_DEBUG_LOG(parameter_provider, parameter_provider_method_id)
PROVIDER_DEBUG_LOG(file_provider, file_provider_method_id)
PROVIDER_DEBUG_LOG(device_extension_provider, device_extension_provider_method_id)
PROVIDER_DEBUG_LOG(device_description_provider, device_description_provider_method_id)
PROVIDER_DEBUG_LOG(model_provider, model_provider_method_id)


template <>
char const * debug_log_object_name<wdx::parameter_service_backend_extended_i>()
{ return "parameter_service_backend"; }

template <>
char const * debug_log_method_name<wdx::parameter_service_backend_extended_i>(method_id_type method_id)
{
  return backend_method_id_string(static_cast<backend_method_id>(method_id));
}

template <>
char const * debug_log_object_name<remote_backend_i>()
{ return "parameter_service_backend"; }

template <>
char const * debug_log_method_name<remote_backend_i>(method_id_type method_id)
{
  return backend_method_id_string(static_cast<backend_method_id>(method_id));
}
// parasoft-end-suppress CERT_CPP-INT50-a-3

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
