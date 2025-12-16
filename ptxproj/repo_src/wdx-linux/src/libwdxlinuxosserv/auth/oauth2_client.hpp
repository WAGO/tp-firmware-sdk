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
///  \brief    OAuth 2.0 client.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_CLIENT_HPP_
#define SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_CLIENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>
#include <wc/std_includes.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace auth {

class oauth2_client
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(oauth2_client)

public:
    explicit
    oauth2_client(std::string const &client_id,
                  std::string const &server_origin = "http://localhost",
                  std::string const &token_path    =                  "/auth/token",
                  std::string const &verify_path   =                  "/auth/verify",
                  std::string const &client_secret = "");
    oauth2_client(           oauth2_client&&) = default;
    oauth2_client& operator=(oauth2_client&&) = default;
    ~oauth2_client() noexcept;

    struct token_result
    {
        bool        success       = false;
        bool        expired       = false;
        std::string access_token;
        uint32_t    expires_in    = 0;
        std::string refresh_token;
    };

    token_result resource_owner_password_credentials_grant(std::string const &scopes,
                                                           std::string const &user,
                                                           std::string const &password) noexcept;
    token_result refresh_token_grant(std::string const refresh_token) noexcept;

private:
    std::string const client_id_m;
    std::string const server_origin_m;
    std::string const auth_path_m;
    std::string const token_path_m;
    std::string const verify_path_m;
    std::string const client_secret_m;
};


} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_CLIENT_HPP_
//---- End of source file ------------------------------------------------------
