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
///  \brief    Interface of handler for tokens used in OAuth 2 context.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_OAUTH_TOKEN_HANDLER_I_HPP_
#define INC_WAGO_AUTHSERV_OAUTH_TOKEN_HANDLER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <wc/std_includes.h>

#include <wc/structuring.h>
#include <string>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class oauth_token_handler_i
{
    WC_INTERFACE_CLASS(oauth_token_handler_i)

public:
    virtual std::string generate_auth_code(    uint32_t    const  duration_s,
                                               std::string const &client_id,
                                               std::string const &scopes,
                                               std::string const &user_name,
                                               std::string const &code_challenge) = 0;
    virtual std::string generate_access_token( uint32_t    const  duration_s,
                                               std::string const &client_id,
                                               std::string const &scopes,
                                               std::string const &user_name) = 0;
    virtual std::string generate_refresh_token(uint32_t    const  duration_s,
                                               std::string const &client_id,
                                               std::string const &scopes,
                                               std::string const &user_name) = 0;

    struct validation_result
    {
        bool        valid            = false;
        bool        expired          = false;
        uint32_t    remaining_time_s =     0;
        std::string client_id;
        std::string scopes;
        std::string user_name;
    };

    virtual validation_result validate_auth_code(    std::string const &code,
                                                     std::string const &code_verifier) = 0;
    virtual validation_result validate_access_token( std::string const &token) = 0;
    virtual validation_result validate_refresh_token(std::string const &token) = 0;

    virtual void revoke_token(std::string const &token) = 0;
    virtual void revoke_all_tokens() = 0;
    virtual void cleanup_blacklist() noexcept = 0;
};


} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_OAUTH_TOKEN_HANDLER_I_HPP_
//---- End of source file ------------------------------------------------------

