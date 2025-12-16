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
///  \brief    Verify endpoint of the WEB-API frontend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_VERIFY_HPP_
#define SRC_LIBAUTHSERV_VERIFY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class oauth_token_handler_i;
namespace http {
class request_i;
}

void process_verify_request(http::request_i       &request,
                            oauth_token_handler_i &token_handler);


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_VERIFY_HPP_
//---- End of source file ------------------------------------------------------
