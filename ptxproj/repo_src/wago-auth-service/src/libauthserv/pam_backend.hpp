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
///  \brief    PAM authentication backend for user/password combination.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_PAM_BACKEND_HPP_
#define SRC_LIBAUTHSERV_PAM_BACKEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "password_backend_i.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class pam_backend final : public password_backend_i
{
private:
    std::string const pam_service_name;

public:
    pam_backend(std::string service_file);
    ~pam_backend() noexcept override;
    auth_result authenticate(std::string const &user,
                             std::string const &password) override;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_PAM_BACKEND_HPP_
//---- End of source file ------------------------------------------------------
