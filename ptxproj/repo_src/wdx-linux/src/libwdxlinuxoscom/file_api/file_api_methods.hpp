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
///  \brief    Common method ID definitions for the file api frontend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_METHODS_HPP_
#define SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_METHODS_HPP_

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

enum file_api_method_id : method_id_type
{
    file_read                       = 800,
    file_write                      = 810,
    file_get_info                   = 820,
    file_create                     = 830,
    file_create_parameter_upload_id = 840
};
WC_STATIC_ASSERT(sizeof(file_api_method_id) == sizeof(method_id_type));
constexpr char const * file_api_method_id_string(file_api_method_id id) {
    switch (id)
    {
        case file_read:                       return "file_read";
        case file_write:                      return "file_write";
        case file_get_info:                   return "file_get_info";
        case file_create:                     return "file_create";
        case file_create_parameter_upload_id: return "file_create_parameter_upload_id";

        default: return "";
    }
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_METHODS_HPP_
//---- End of source file ------------------------------------------------------
