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
///  \brief    Authenticator interface for WAGO auth service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_AUTHENTICATOR_I_HPP_
#define SRC_LIBAUTHSERV_AUTHENTICATOR_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/request_i.hpp"
#include "auth_result.hpp"

#include <wago/future.hpp>

#include <wc/structuring.h>
#include <vector>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class authenticator_i
{
    WC_INTERFACE_CLASS(authenticator_i)

public:
    virtual bool admin_has_no_password() = 0;
    virtual bool has_form_auth_data(http::request_i const &request) = 0;
    virtual bool has_form_password_change_data(http::request_i const &request) = 0;
    virtual auth_result authenticate(http::request_i          const &request,
                                     std::vector<std::string> const &scopes) = 0;
    virtual wago::future<void> password_change(http::request_i const &request) = 0;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_AUTHENTICATOR_I_HPP_
//---- End of source file ------------------------------------------------------
