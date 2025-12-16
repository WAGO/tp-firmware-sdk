//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Authentication result struct.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_AUTH_RESULT_HPP_
#define SRC_LIBAUTHSERV_AUTH_RESULT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

struct auth_result
{
    bool success;
    bool expired;
    bool unauthorized;
    std::string user_name;
};

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_AUTH_RESULT_HPP_
//---- End of source file ------------------------------------------------------

