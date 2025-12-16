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
///  \brief    HTTP secure request handler to force HTTPS.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_HTTP_SECURE_HANDLER_HPP_
#define SRC_LIBAUTHSERV_HTTP_SECURE_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/request_handler_i.hpp"

#include <memory>

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

using std::unique_ptr;
using std::shared_ptr;


class secure_handler : public request_handler_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(secure_handler)
private:
    unique_ptr<request_handler_i> handler_m;
    bool                          allow_local_http_m;

public:
    secure_handler(unique_ptr<request_handler_i> next_handler,
                   bool                          allow_local_http = false);
    ~secure_handler() noexcept override;
    secure_handler(secure_handler&&) noexcept;
    secure_handler& operator=(secure_handler&&) noexcept;
    void handle(std::unique_ptr<request_i> request) noexcept override;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_HTTP_SECURE_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
