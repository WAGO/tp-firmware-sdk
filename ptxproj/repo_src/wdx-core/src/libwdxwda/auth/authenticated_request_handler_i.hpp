//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    HTTP request handler for authenticated requests
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_AUTH_AUTHENTICATED_REQUEST_HANDLER_I_HPP_
#define SRC_LIBWDXWDA_AUTH_AUTHENTICATED_REQUEST_HANDLER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>
#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {
    class request_i;
}

namespace auth {

struct authentication_info
{
    std::string user_name;
};

inline bool operator==(const authentication_info& lhs, const authentication_info& rhs)
{
    return lhs.user_name == rhs.user_name;
}

class authenticated_request_handler_i
{
    WC_INTERFACE_CLASS(authenticated_request_handler_i)

public:
    virtual void handle(std::unique_ptr<http::request_i> request, authentication_info auth_info) noexcept = 0;
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


#endif // SRC_LIBWDXWDA_AUTH_AUTHENTICATED_REQUEST_HANDLER_I_HPP_
//---- End of source file ------------------------------------------------------
