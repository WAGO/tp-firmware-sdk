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
///  \brief    WEB-API frontend for WAGO Auth Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_WEB_FRONTEND_HPP_
#define SRC_LIBAUTHSERV_WEB_FRONTEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/request_handler_i.hpp"
#include "settings_store.hpp"
#include "authenticator.hpp"
#include "pam_backend.hpp"
#include "paramd_backend.hpp"
#include "oauth_token_handler.hpp"

#include <wc/structuring.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class response_helper;

using http::request_i;
using http::request_handler_i;

class web_frontend : public request_handler_i
{
private:
    std::string                           const service_base_m;
    std::shared_ptr<settings_store_i>           settings_store_m;
    std::shared_ptr<pam_backend>                pam_backend_m;
    std::shared_ptr<paramd_backend>             paramd_backend_m;
    authenticator                               authenticator_m;
    std::shared_ptr<oauth_token_handler_i>      oauth_token_handler_m;
    std::shared_ptr<response_helper>            html_templater_m;

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(web_frontend)

public:
    web_frontend(std::string                            const &service_file,
                 std::string                            const &service_name,
                 std::string                            const &service_base,
                 std::shared_ptr<settings_store_i>             settings_store,
                 wdx::parameter_service_frontend_i            &paramd_backend,
                 std::shared_ptr<oauth_token_handler_i>        oauth_token_handler);
    ~web_frontend() noexcept override;
    virtual request_handler_i & get_handler();

    // Interface request_handler_i
    void handle(std::unique_ptr<request_i> request) noexcept override;

private:
    void handle_authorize(      std::shared_ptr<request_i>  request);
    void handle_token(          request_i                  &request);
    void handle_verify(         request_i                  &request);
    void handle_password_change(std::shared_ptr<request_i>  request);
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_WEB_FRONTEND_HPP_
//---- End of source file ------------------------------------------------------
