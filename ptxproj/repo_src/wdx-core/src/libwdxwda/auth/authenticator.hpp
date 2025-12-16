//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Authenticator for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_AUTH_AUTHENTICATOR_HPP_
#define SRC_LIBWDXWDA_AUTH_AUTHENTICATOR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticated_request_handler_i.hpp"
#include "wago/wdx/wda/http/request_handler_i.hpp"
#include "wago/wdx/wda/trace_routes.hpp"
#include "wago/wdx/wda/auth/password_backend_i.hpp"
#include "wago/wdx/wda/auth/token_backend_i.hpp"

#include <memory>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
class auth_settings_i;

namespace auth {
using http::request_handler_i;
using http::request_i;
using std::shared_ptr;
using std::unique_ptr;
using std::string;


class authenticator : public request_handler_i
{
private:
    shared_ptr<auth_settings_i>                       auth_settings_m;
    shared_ptr<password_backend_i>                    basic_auth_m;
    shared_ptr<token_backend_i>                       token_auth_m;
    string                                      const service_base_m;
    string                                      const realm_m;
    trace_route                                 const route_m;
    shared_ptr<authenticated_request_handler_i>       handler_m;

public:
    authenticator(shared_ptr<auth_settings_i>                         auth_settings,
                  shared_ptr<password_backend_i>                      basic_auth_backend,
                  shared_ptr<token_backend_i>                         token_auth_backend,
                  string                                       const &service_base,
                  trace_route                                  const  route,
                  shared_ptr<authenticated_request_handler_i>         next_handler);
    ~authenticator() noexcept override;
    void handle(std::unique_ptr<request_i> request) noexcept override;
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


#endif // SRC_LIBWDXWDA_AUTH_AUTHENTICATOR_HPP_
//---- End of source file ------------------------------------------------------
