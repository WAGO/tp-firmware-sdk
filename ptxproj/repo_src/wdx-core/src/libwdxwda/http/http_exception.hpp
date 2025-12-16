//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Exception object to represent an error which is deliverable through
///            our REST-API responses.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_HTTP_HTTP_EXCEPTION_HPP_
#define SRC_LIBWDXWDA_HTTP_HTTP_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/exception.hpp"
#include "wago/wdx/wda/http/http_status_code.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

using std::string;
using http::http_status_code;

class http_exception : public wda::exception
{
private:
    http_status_code const http_status_m;

public:
    http_exception(wda::exception const &e, // NOLINT(google-explicit-constructor)
                   http_status_code     const  http_status = http_status_code::internal_server_error);
    explicit http_exception(string           const &message,
                            http_status_code const  http_status = http_status_code::internal_server_error);

    virtual http_status_code get_http_status_code() const;
    virtual string           get_title()            const;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_HTTP_HTTP_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
