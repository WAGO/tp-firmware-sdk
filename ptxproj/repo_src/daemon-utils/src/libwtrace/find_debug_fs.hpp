//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Global helper function to find Debug FS for kernel tracer.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWTRACE_FIND_DEBUG_FS_HPP_
#define SRC_LIBWTRACE_FIND_DEBUG_FS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

namespace wago {
namespace wtrace {

std::string find_debug_fs();

} // Namespace wtrace
} // Namespace wago


#endif // SRC_LIBWTRACE_FIND_DEBUG_FS_HPP_
//---- End of source file ------------------------------------------------------
