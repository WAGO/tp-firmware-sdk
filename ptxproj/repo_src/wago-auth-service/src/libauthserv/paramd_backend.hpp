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
///  \brief    paramd backend for password change.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_PARAMD_BACKEND_HPP_
#define SRC_LIBAUTHSERV_PARAMD_BACKEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "password_change_backend_i.hpp"

#include <wago/wdx/linuxos/com/parameter_service_frontend_proxy.hpp>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class paramd_backend_impl;

class paramd_backend final : public password_change_backend_i
{
private:
    std::unique_ptr<paramd_backend_impl> impl_m;

public:
    enum wdx_user_model {
        password_management,
        account_management
    };
    paramd_backend(wdx_user_model                     model, 
                   wdx::parameter_service_frontend_i &frontend);
    ~paramd_backend() noexcept override;
    wago::future<void> change_password(std::string const &user,
                                       std::string const &password,
                                       std::string const &new_password) override;
};

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_PARAMD_BACKEND_HPP_
//---- End of source file ------------------------------------------------------
