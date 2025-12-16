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
///  \brief    Definitions specific to the File API used in several places.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_FILES_DEFINITIONS_HPP_
#define SRC_LIBWDXWDA_FILES_DEFINITIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/http_method.hpp"

namespace wago {
namespace wdx {
namespace wda {
namespace files {
//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const cors_allowed_headers_service_base[] = "Accept, Authorization, Content-Length, Content-Type, Wago-Wdx-No-Auth-Popup";
static constexpr char const cors_exposed_headers_service_base[] = "Content-Length, Content-Type, Location, Www-Authenticate, Wago-Wdx-Auth-Token, Wago-Wdx-Auth-Token-Expiration, Wago-Wdx-Auth-Token-Type";
static constexpr http::http_method const allowed_methods_service_base[] = {
    http::http_method::options,
    http::http_method::post
};
static constexpr char const cors_allowed_headers_file_id[] = "Accept, Authorization, Content-Length, Content-Type, Wago-Wdx-No-Auth-Popup, Range";
static constexpr char const cors_exposed_headers_file_id[] = "Content-Length, Content-Type, Content-Range, Www-Authenticate, Wago-Wdx-Auth-Token, Wago-Wdx-Auth-Token-Expiration, Wago-Wdx-Auth-Token-Type";
static constexpr http::http_method const allowed_methods_file_id[] = {
    http::http_method::options,
    http::http_method::head,
    http::http_method::get,
    http::http_method::put,
    http::http_method::patch
};
static constexpr char const context_query_param_name[] = "context";


} // Namespace files
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_FILES_DEFINITIONS_HPP_
//---- End of source file ------------------------------------------------------
