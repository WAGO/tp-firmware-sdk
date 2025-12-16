//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Authentication result struct.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_AUTH_AUTH_RESULT_HPP_
#define INC_WAGO_WDX_WDA_AUTH_AUTH_RESULT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

#include <wc/std_includes.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace auth {

struct auth_result
{
    std::string user_name;
    bool        success          = false;
    bool        expired          = false;
    std::string token;
    uint32_t    token_expires_in = 0;
};

} // Namespace auth
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_AUTH_AUTH_RESULT_HPP_
//---- End of source file ------------------------------------------------------
