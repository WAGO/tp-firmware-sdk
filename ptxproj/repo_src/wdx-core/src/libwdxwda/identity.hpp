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
///  \brief    Header with static identity information.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_IDENTITY_HPP_
#define SRC_LIBWDXWDA_IDENTITY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/preprocessing.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const * const service_id             = "wda";
static constexpr char const * const service_name           = "WAGO Device Access";
static constexpr uint16_t     const major_version          = WDXWDA_MAJOR;
static constexpr uint16_t     const minor_version          = WDXWDA_MINOR;
static constexpr uint16_t     const bugfix_version         = WDXWDA_BUGFIX;
static constexpr char const * const version_revision       = WC_SUBST_STR(WDXWDA_REVISION);
static constexpr char const * const service_version_string = WC_SUBST_STR(WDXWDA_VERSION);

} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_IDENTITY_HPP_
//---- End of source file ------------------------------------------------------
