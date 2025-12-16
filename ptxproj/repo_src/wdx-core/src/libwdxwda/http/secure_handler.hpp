//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    HTTP secure request handler to force HTTPS.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_HTTP_SECURE_HANDLER_HPP_
#define SRC_LIBWDXWDA_HTTP_SECURE_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/request_handler_i.hpp"

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

using std::unique_ptr;
using std::shared_ptr;


class secure_handler : public request_handler_i
{
private:
    shared_ptr<request_handler_i> handler_m;

public:
    secure_handler(shared_ptr<request_handler_i> next_handler);
    ~secure_handler() noexcept override;
    void handle(std::unique_ptr<request_i> request) noexcept override;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_HTTP_SECURE_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
