//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Interface for authentication backend for token based 
///            authentication.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_AUTH_PASSWORD_BACKEND_I_HPP_
#define INC_WAGO_WDX_WDA_AUTH_PASSWORD_BACKEND_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/auth/auth_result.hpp"

#include <wc/structuring.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace auth {

class password_backend_i
{
    WC_INTERFACE_CLASS(password_backend_i)

public:
    virtual auth_result authenticate(std::string const &user,
                                     std::string const &password) = 0;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace auth
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_AUTH_PASSWORD_BACKEND_I_HPP_
//---- End of source file ------------------------------------------------------

