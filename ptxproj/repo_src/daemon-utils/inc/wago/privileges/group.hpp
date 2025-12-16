//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility functions to get group information.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_PRIVILEGES_GROUP_HPP_
#define INC_WAGO_PRIVILEGES_GROUP_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <string>
#include <vector>

#include <sys/types.h>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace privileges {

//------------------------------------------------------------------------------
/// \brief Function to get the group ID for a given group name.
/// 
/// \param group_name Name of the group to get the ID for.
/// \return Group ID of the group.
/// \throw std::runtime_error If the group ID could not be determined.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API gid_t get_gid_for_group(char const * const group_name);

//------------------------------------------------------------------------------
/// \brief Function to get the group name for a given group ID.
///
/// \param group_id ID of the group to get the name for.
/// \return Name of the group.
/// \throw std::runtime_error If the group name could not be determined.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API std::string get_group_name(gid_t const group_id);

//------------------------------------------------------------------------------
/// \brief Function to get the groups name for a given user name.
///
/// \param user_name Name of the user to get the group names for.
/// \return Vector of group names the given user belongs to.
/// \throw std::runtime_error If the group names could not be determined.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API std::vector<std::string> get_user_group_names(std::string const &user_name);

} // Namespace privileges
} // Namespace wago


#endif // INC_WAGO_PRIVILEGES_GROUP_HPP_
//---- End of source file ------------------------------------------------------
