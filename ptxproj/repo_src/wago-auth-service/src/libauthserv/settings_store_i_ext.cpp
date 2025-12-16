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
///  \brief    Implementation of interface extensions for settings_store_i.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/settings_store_i.hpp"

#include <algorithm>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------------

bool settings_store_i::oauth_client::supports_grant_type(std::string const &grant_type) const {
    return std::find(grant_types.begin(), grant_types.end(), grant_type) != grant_types.end();
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char const* settings_store_i::global_setting_names[];
constexpr char const* settings_store_i::global_setting_defaults[];

constexpr char const settings_store_i::client_label_name[];
constexpr char const settings_store_i::client_redirect_uri_name[];
constexpr char const settings_store_i::client_grant_types_name[];

constexpr char const settings_store_i::resource_server_label_name[];
constexpr char const settings_store_i::resource_server_scopes_name[];

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
