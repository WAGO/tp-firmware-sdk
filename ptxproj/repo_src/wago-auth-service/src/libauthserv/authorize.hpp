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
#ifndef SRC_LIBAUTHSERV_AUTHORIZE_HPP_
#define SRC_LIBAUTHSERV_AUTHORIZE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class authenticator_i;
class oauth_token_handler_i;
class settings_store_i;
class response_helper_i;
namespace http {
class request_i;
}

void process_authorize_request(std::shared_ptr<response_helper_i>     const  html_templater,
                               std::shared_ptr<settings_store_i>      const  settings_store,
                               std::shared_ptr<http::request_i>              request,
                               std::shared_ptr<oauth_token_handler_i>        token_handler,
                               authenticator_i                              &auth_handler);

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_AUTHORIZE_HPP_
//---- End of source file ------------------------------------------------------
