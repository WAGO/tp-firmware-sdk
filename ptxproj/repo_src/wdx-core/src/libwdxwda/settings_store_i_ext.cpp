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
///  \brief    Implementation of interface extensions for settings_store_i.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/settings_store_i.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char const settings_store_i::allow_unauthenticated_requests_for_scan_devices[];
constexpr char const settings_store_i::file_api_upload_id_timeout[];
constexpr char const settings_store_i::run_result_timeout[];
constexpr char const settings_store_i::oauth2_origin[];
constexpr char const settings_store_i::oauth2_token_path[];
constexpr char const settings_store_i::oauth2_verify_access_path[];
constexpr char const settings_store_i::oauth2_client_id[];
constexpr char const settings_store_i::oauth2_client_secret[];


} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
