//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Common method ID definitions for the backend.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_METHODS_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_METHODS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/method_id.hpp"

#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

enum backend_method_id : method_id_type
{
    // Device management
    backend_register_devices                           = 101,
    backend_unregister_devices                         = 102,
    backend_unregister_all_devices                     = 103,

    // Provider management
    backend_create_parameter_provider_proxies          = 211,
    backend_register_parameter_providers               = 212,
    backend_unregister_parameter_providers             = 213,
    backend_update_parameter_provider_proxies          = 214,

    backend_create_file_provider_proxies               = 221,
    backend_register_file_providers                    = 223,
    backend_unregister_file_providers                  = 224,
    backend_reregister_file_providers                  = 225,

    backend_create_model_provider_proxies              = 231,
    backend_register_model_providers                   = 232,
    backend_unregister_model_providers                 = 233,

    backend_create_device_description_provider_proxies = 241,
    backend_register_device_description_providers      = 242,
    backend_unregister_device_description_providers    = 243,

    backend_create_device_extension_provider_proxies   = 251,
    backend_register_device_extension_providers        = 252,
    backend_unregister_device_extension_providers      = 253
};
WC_STATIC_ASSERT(sizeof(backend_method_id) == sizeof(method_id_type));
constexpr char const * backend_method_id_string(backend_method_id id) {
    switch (id)
    {
        case backend_register_devices:                           return "register_devices";
        case backend_unregister_devices:                         return "unregister_devices";
        case backend_unregister_all_devices:                     return "unregister_all_devices";
        case backend_update_parameter_provider_proxies:          return "update_parameter_provider_proxies";
        case backend_create_parameter_provider_proxies:          return "create_parameter_provider_proxies";
        case backend_register_parameter_providers:               return "register_parameter_providers";
        case backend_unregister_parameter_providers:             return "unregister_parameter_providers";
        case backend_create_file_provider_proxies:               return "create_file_provider_proxies";
        case backend_register_file_providers:                    return "register_file_providers";
        case backend_unregister_file_providers:                  return "unregister_file_providers";
        case backend_reregister_file_providers:                  return "reregister_file_providers";
        case backend_create_model_provider_proxies:              return "create_model_provider_proxies";
        case backend_register_model_providers:                   return "register_model_providers";
        case backend_unregister_model_providers:                 return "unregister_model_providers";
        case backend_create_device_description_provider_proxies: return "create_device_description_provider_proxies";
        case backend_register_device_description_providers:      return "register_device_description_providers";
        case backend_unregister_device_description_providers:    return "unregister_device_description_providers";
        case backend_create_device_extension_provider_proxies:   return "create_device_extension_provider_proxies";
        case backend_register_device_extension_providers:        return "register_device_extension_providers";
        case backend_unregister_device_extension_providers:      return "unregister_device_extension_providers";

        default: return "";
    }
}

enum parameter_provider_method_id : method_id_type
{
    display_name                          = 1,
    get_provided_parameters               = 2,
    get_parameter_values                  = 3,
    set_parameter_values                  = 4,
    invoke_method                         = 5,
    set_parameter_values_connection_aware = 6,
    create_parameter_upload_id            = 7,
    remove_parameter_upload_id            = 8
};
WC_STATIC_ASSERT(sizeof(parameter_provider_method_id) == sizeof(method_id_type));
constexpr char const * parameter_provider_method_id_string(parameter_provider_method_id id) {
    switch (id)
    {
        case display_name:                          return "display_name";
        case get_provided_parameters:               return "get_provided_parameters";
        case get_parameter_values:                  return "get_parameter_values";
        case set_parameter_values:                  return "set_parameter_values";
        case invoke_method:                         return "invoke_method";
        case set_parameter_values_connection_aware: return "set_parameter_values_connection_aware";
        case create_parameter_upload_id:            return "create_parameter_upload_id";
        case remove_parameter_upload_id:            return "remove_parameter_upload_id";

        default: return "";
    }
}

enum model_provider_method_id : method_id_type
{
    get_model_information = 1
};
WC_STATIC_ASSERT(sizeof(model_provider_method_id) == sizeof(method_id_type));
constexpr char const * model_provider_method_id_string(model_provider_method_id id) {
    switch (id)
    {
        case get_model_information: return "get_model_information";

        default: return "";
    }
}

enum device_description_provider_method_id : method_id_type
{
    get_provided_devices   = 1,
    get_device_information = 2
};
WC_STATIC_ASSERT(sizeof(device_description_provider_method_id) == sizeof(method_id_type));
constexpr char const * device_description_provider_method_id_string(device_description_provider_method_id id) {
    switch (id)
    {
        case get_provided_devices:   return "get_provided_devices";
        case get_device_information: return "get_device_information";

        default: return "";
    }
}

enum device_extension_provider_method_id : method_id_type
{
    get_device_extensions = 1
};
WC_STATIC_ASSERT(sizeof(device_extension_provider_method_id) == sizeof(method_id_type));
constexpr char const * device_extension_provider_method_id_string(device_extension_provider_method_id id) {
    switch (id)
    {
        case get_device_extensions: return "get_device_extensions";

        default: return "";
    }
}

enum file_provider_method_id : method_id_type
{
    read          = 1,
    write         = 2,
    get_file_info = 3,
    create        = 4
};
WC_STATIC_ASSERT(sizeof(file_provider_method_id) == sizeof(method_id_type));
constexpr char const * file_provider_method_id_string(file_provider_method_id id) {
    switch (id)
    {
        case read:          return "read";
        case write:         return "write";
        case get_file_info: return "get_file_info";
        case create:        return "create";

        default: return "";
    }
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_METHODS_HPP_
//---- End of source file ------------------------------------------------------
