//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Definitions specific to (inter-process) communication.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_DEFINITIONS_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_DEFINITIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/definitions.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
#define SERVICE_NAME                 "WAGO Parameter Service"
#define SERVICE_LOG_PREFIX           SERVICE_NAME " Com: "

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

struct no_return {};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr uint32_t const frontend_object_id         = 0;
constexpr uint32_t const file_api_object_id         = 0;
constexpr uint32_t const backend_object_id          = 0;
constexpr uint32_t const generated_object_ids_start = backend_object_id + 1;

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_DEFINITIONS_HPP_
//---- End of source file ------------------------------------------------------
