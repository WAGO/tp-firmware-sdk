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
///  \brief    Handler for tokens used in OAuth 2 context.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_OAUTH_TOKEN_HANDLER_HPP_
#define SRC_LIBAUTHSERV_OAUTH_TOKEN_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/oauth_token_handler_i.hpp"
#include "wago/authserv/settings_store_i.hpp"
#include "token_handler.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <mutex>
#include <map>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class oauth_token_handler final : public oauth_token_handler_i
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(oauth_token_handler)
public:
    explicit oauth_token_handler(settings_store_i &settings_store);
    ~oauth_token_handler() noexcept override;

    std::string generate_auth_code(      uint32_t    const  lifetime_s,
                                         std::string const &client_id,
                                         std::string const &scopes,
                                         std::string const &user_name,
                                         std::string const &code_challenge) override;
    std::string generate_access_token(   uint32_t    const  lifetime_s,
                                         std::string const &client_id,
                                         std::string const &scopes,
                                         std::string const &user_name) override;
    std::string generate_refresh_token(  uint32_t    const  lifetime_s,
                                         std::string const &client_id,
                                         std::string const &scopes,
                                         std::string const &user_name) override;

    validation_result validate_auth_code(    std::string const &code,
                                             std::string const &code_verifier) override;
    validation_result validate_access_token( std::string const &token) override;
    validation_result validate_refresh_token(std::string const &token) override;

    void revoke_token(std::string const &token) override;
    void revoke_all_tokens() override;
    void cleanup_blacklist() noexcept override;

private:
    token_handler                 token_handler_m;
    std::mutex                    token_blacklist_mutex_m;
    std::map<std::string, time_t> token_blacklist_m;

    std::string generate_token(uint32_t    const  lifetime_s,
                               std::string const &type,
                               std::string const &client_id,
                               std::string const &scopes,
                               std::string const &user_name,
                               std::string const &additional_data = "");
    validation_result validate_token(std::string const &token,
                                     std::string const &type,
                                     std::string const &expected_additional_data = "") noexcept;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_OAUTH_TOKEN_HANDLER_HPP_
//---- End of source file ------------------------------------------------------

