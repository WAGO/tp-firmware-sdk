//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    OAuth 2.0 authentication backend for user/password combination and token.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_BACKEND_HPP_
#define SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_BACKEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/wda/auth/password_backend_i.hpp>
#include <wago/wdx/wda/auth/token_backend_i.hpp>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
    class settings_store_i;
}

namespace linuxos {
namespace serv {
namespace auth {

using wdx::wda::auth::password_backend_i;
using wdx::wda::auth::token_backend_i;
using wdx::wda::auth::auth_result;


class oauth2_backend final : public password_backend_i, public token_backend_i
{
public:
    class backend_impl;

private:
    std::unique_ptr<backend_impl> impl;

public:
    oauth2_backend(std::shared_ptr<wdx::wda::settings_store_i>       settings_store,
                   size_t                                       const broken_token_slowdown);
    ~oauth2_backend() noexcept override;
    auth_result authenticate(std::string const &user,
                             std::string const &password) override;
    auth_result authenticate(std::string   const &token,
                             uint32_t    * const  remaining_time = nullptr) override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr uint32_t const token_default_lifetime = 5 * 60;
constexpr uint32_t const oauth_token_cache_time = 5 * 60;


} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_BACKEND_HPP_
//---- End of source file ------------------------------------------------------
