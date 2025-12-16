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
///  \brief    Exceptions to transport error information tailored to OAuth 2.0.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_OAUTH_ERROR_HPP_
#define SRC_LIBAUTHSERV_OAUTH_ERROR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "error_document.hpp"
#include "wago/authserv/exception.hpp"
#include "wago/authserv/http/http_status_code.hpp"

#include <wc/structuring.h>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class oauth_error : public exception // parasoft-suppress CERT_CPP-ERR60-a-3 "bad_alloc exception is not thrown on Linux, copy constructor is not known to throw."
{
public:
    enum class error_type
    {
        invalid_request,
        invalid_client,
        invalid_grant,
        unauthorized_client,
        unsupported_grant_type,
        invalid_scope,
        unsupported_response_type,
        access_denied,
        server_error
    };

    WC_INTERFACE_IMPL_CLASS(oauth_error)

public:
    explicit oauth_error(error_type  type,
                         std::string description = "",
                         std::string uri = "");

    error_document get_as_document() const;
    std::string    get_as_query() const;

private:
    std::string            type_m;
    std::string            description_m;
    std::string            uri_m;
    http::http_status_code status_code_m;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_OAUTH_ERROR_HPP_
//---- End of source file ------------------------------------------------------
