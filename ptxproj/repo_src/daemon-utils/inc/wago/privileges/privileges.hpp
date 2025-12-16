//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility functions to handle privileges.
///
///  \author   PEn : WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_PRIVILEGES_PRIVILEGES_HPP_
#define INC_WAGO_PRIVILEGES_PRIVILEGES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"
#include <sys/types.h>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace privileges {

//------------------------------------------------------------------------------
/// Function to drop privileges and use newly given user/group name.
/// Existing supplementary groups of the new user are initialized and applied.
///
/// \remark This function is not thread-safe.
/// \param new_user_name
///   User name of user to execute as. If set to nullptr, an exception is 
///   thrown.
/// \param new_group_name
///   Group name of group to execute as. If omitted or `nullptr`, 
///   `new_user_name` is also used as `new_group_name`. 
/// \throw std::exception Exceptions are thrown if anything goes wrong during 
///        dropping the user's privileges. If an exception is thrown, the 
///        application should terminate. The state of the user and groups is
///        unknown at this moment and cannot be recovered.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API void drop_privileges(char const * new_user_name,
                                         char const * new_group_name = nullptr);

//------------------------------------------------------------------------------
/// Function to drop privileges and use newly given user/group ID.
/// Existing supplementary groups of the new user are initialized and applied.
///
/// \remark This function is not thread-safe.
/// \param new_user_id
///   User ID of user to execute as.
/// \param new_group_id
///   Group ID of group to execute as.
/// \throw std::exception Exceptions are thrown if anything goes wrong during 
///        dropping the user's privileges. If an exception is thrown, the 
///        application should terminate. The state of the user and groups is
///        unknown at this moment and cannot be recovered.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API void drop_privileges(uid_t const new_user_id,
                                         gid_t const new_group_id);

} // Namespace privileges
} // Namespace wago


#endif // INC_WAGO_PRIVILEGES_PRIVILEGES_HPP_
//---- End of source file ------------------------------------------------------
