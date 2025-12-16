//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Password change endpoint of the WEB-API frontend.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_PASSWORD_CHANGE_HPP_
#define SRC_LIBAUTHSERV_PASSWORD_CHANGE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class authenticator_i;
class response_helper_i;
namespace http {
class request_i;
}

void process_password_change_request(std::shared_ptr<response_helper_i> const  html_templater,
                                     std::shared_ptr<http::request_i>          request,
                                     authenticator_i                          &auth_handler);

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_PASSWORD_CHANGE_HPP_
//---- End of source file ------------------------------------------------------
