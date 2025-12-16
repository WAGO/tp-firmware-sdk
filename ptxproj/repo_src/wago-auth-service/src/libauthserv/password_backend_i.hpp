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
///  \brief    Interface for authentication backend for password based
///            authentication.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_PASSWORD_BACKEND_I_HPP_
#define SRC_LIBAUTHSERV_PASSWORD_BACKEND_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "auth_result.hpp"

#include <wc/structuring.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class password_backend_i
{
    WC_INTERFACE_CLASS(password_backend_i)

public:
    virtual auth_result authenticate(std::string const &user,
                                     std::string const &password) = 0;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_PASSWORD_BACKEND_I_HPP_
//---- End of source file ------------------------------------------------------

