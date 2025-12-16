//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Interface for password change backend for password based
///            authentication.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_PASSWORD_CHANGE_BACKEND_I_HPP_
#define SRC_LIBAUTHSERV_PASSWORD_CHANGE_BACKEND_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>

#include <memory>
#include <string>
#include <stdexcept>

#include <wago/future.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class password_change_error : public std::runtime_error
{
    public:
        using runtime_error::runtime_error;
};

class password_change_backend_i
{
    WC_INTERFACE_CLASS(password_change_backend_i)

public:
    virtual wago::future<void> change_password(std::string const &user,
                                               std::string const &password,
                                               std::string const &new_password) = 0;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_PASSWORD_CHANGE_BACKEND_I_HPP_
//---- End of source file ------------------------------------------------------

