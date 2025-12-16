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
///  \brief    Exception object to represent an error which is deliverable through
///            our WEB-API responses.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_HTTP_HTTP_EXCEPTION_HPP_
#define SRC_LIBAUTHSERV_HTTP_HTTP_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/exception.hpp"
#include "wago/authserv/http/http_status_code.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

using std::string;
using http::http_status_code;

class http_exception : public authserv::exception
{
private:
    http_status_code const http_status_m;

public:
    http_exception(authserv::exception const &e, // NOLINT(google-explicit-constructor)
                   http_status_code    const  http_status = http_status_code::internal_server_error);
    explicit http_exception(string           const &message,
                            http_status_code const  http_status = http_status_code::internal_server_error);

    virtual http_status_code get_http_status_code() const;
    virtual string           get_title()            const;
};


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_HTTP_HTTP_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
