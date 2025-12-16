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
///  \brief    Group management functions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_GROUP_MANAGEMENT_HPP_
#define SRC_LIBAUTHSERV_GROUP_MANAGEMENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <vector>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

std::vector<std::string> get_groups(std::string const &user) noexcept;
bool is_authorized_for_scope(std::string              const &user,
                             std::vector<std::string> const &scopes);


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_GROUP_MANAGEMENT_HPP_
//---- End of source file ------------------------------------------------------
