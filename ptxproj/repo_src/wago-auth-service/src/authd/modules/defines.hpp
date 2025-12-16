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
///  \brief    Header with common static information.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_AUTHD_MODULES_DEFINES_HPP_
#define SRC_AUTHD_MODULES_DEFINES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/preprocessing.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define WAGO_TOOL_NAME              "authd"
#define SERVICE_NAME                "WAGO Authentication Service"

namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const * const service_id             = "auth";
static constexpr char const * const service_file           = WAGO_TOOL_NAME;
static constexpr char const * const service_name           = SERVICE_NAME;
static constexpr char const * const service_version_string = WC_SUBST_STR(AUTHSERV_VERSION);

} // Namespace authserv
} // Namespace wago


#endif // SRC_AUTHD_MODULES_DEFINES_HPP_
//---- End of source file ------------------------------------------------------
