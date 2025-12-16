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
///  \brief    Authenticator for WAGO auth service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_AUTHENTICATOR_HPP_
#define SRC_LIBAUTHSERV_AUTHENTICATOR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticator_i.hpp"
#include "password_backend_i.hpp"
#include "password_change_backend_i.hpp"

#include <memory>

#include <wago/future.hpp>
#include <wc/structuring.h>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

// the authenticator methods may throw http_exception with the internal error
class authenticator final : public authenticator_i
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(authenticator)
private:
    std::shared_ptr<password_backend_i>              password_backend_m;
    std::shared_ptr<password_change_backend_i>       password_change_backend_m;
    std::string                                const service_base_m;
    std::string                                const realm_m;

    bool                                             admin_login_without_password;

public:
    authenticator(std::shared_ptr<password_backend_i>               password_backend,
                  std::shared_ptr<password_change_backend_i>        password_change_backend,
                  std::string                                const &service_base);
    ~authenticator() noexcept override;
    bool admin_has_no_password() override;
    bool has_form_auth_data(http::request_i const &request) override;
    bool has_form_password_change_data(http::request_i const &request) override;
    auth_result authenticate(http::request_i          const &request,
                             std::vector<std::string> const &scopes) override;
    wago::future<void> password_change(http::request_i const &request) override;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_AUTHENTICATOR_HPP_
//---- End of source file ------------------------------------------------------
