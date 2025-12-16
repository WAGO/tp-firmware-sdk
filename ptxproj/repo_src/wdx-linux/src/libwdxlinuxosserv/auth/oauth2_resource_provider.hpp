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
///  \brief    OAuth 2.0 resource provider.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_RESOURCE_PROVIDER_HPP_
#define SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_RESOURCE_PROVIDER_HPP_

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

class oauth2_resource_provider
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(oauth2_resource_provider)

public:
    explicit
    oauth2_resource_provider(std::string const &client_id,
                             std::string const &provider_scope,
                             std::string const &server_origin = "http://localhost",
                             std::string const &verify_path   =                  "/auth/verify",
                             std::string const &client_secret = "");
    oauth2_resource_provider(           oauth2_resource_provider&&) = default;
    oauth2_resource_provider& operator=(oauth2_resource_provider&&) = default;
    ~oauth2_resource_provider()noexcept;

    struct verify_result
    {
        bool        active     = false;
        std::string scope;
        std::string client_id;
        std::string username;
        uint32_t    expires_in = 0;
    };
    verify_result access_token_verification(std::string const &access_token) noexcept;

private:
    std::string const client_id_m;
    std::string const provider_scope_m;
    std::string const server_origin_m;
    std::string const verify_path_m;
    std::string const client_secret_m;
};


} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_AUTH_OAUTH2_RESOURCE_PROVIDER_HPP_
//---- End of source file ------------------------------------------------------
