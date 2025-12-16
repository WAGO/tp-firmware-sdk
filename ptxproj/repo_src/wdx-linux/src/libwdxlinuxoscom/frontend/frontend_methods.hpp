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
///  \brief    Common method ID definitions for the frontend.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_METHODS_HPP_
#define SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_METHODS_HPP_

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

enum frontend_method_id : method_id_type
{
    // Device requests
    frontend_get_all_devices              = 500,

    // Parameter read requests
    frontend_get_all_parameters           = 510,
    frontend_get_parameters               = 511,
    frontend_get_parameters_by_path       = 512,

    // Parameter write requests
    frontend_set_parameter_values         = 521,
    frontend_set_parameter_values_by_path = 522,

    // Method invocations
    frontend_invoke_method                = 531,
    frontend_invoke_method_by_path        = 532
};
WC_STATIC_ASSERT(sizeof(frontend_method_id) == sizeof(method_id_type));
constexpr char const * frontend_method_id_string(frontend_method_id id) {
    switch (id)
    {
        case frontend_get_all_devices:              return "get_all_devices";
        case frontend_get_all_parameters:           return "get_all_parameters";
        case frontend_get_parameters:               return "get_parameters";
        case frontend_get_parameters_by_path:       return "get_parameters_by_path";
        case frontend_set_parameter_values:         return "set_parameter_values";
        case frontend_set_parameter_values_by_path: return "set_parameter_values_by_path";
        case frontend_invoke_method:                return "invoke_method";
        case frontend_invoke_method_by_path:        return "invoke_method_by_path";

        default: return "";
    }
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_METHODS_HPP_
//---- End of source file ------------------------------------------------------
