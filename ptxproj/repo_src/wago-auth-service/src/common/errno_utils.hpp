//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Helper functions to work with errno.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_COMMON_ERRNO_UTILS_HPP_
#define SRC_COMMON_ERRNO_UTILS_HPP_

#include <string>
#include <cerrno>

namespace wago {
namespace authserv {

std::string errno_to_string(int errno_num);

} // authserv
} // wago

#endif // SRC_COMMON_ERRNO_UTILS_HPP_
