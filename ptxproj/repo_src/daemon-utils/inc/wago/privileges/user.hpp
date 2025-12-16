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
///  \brief    Utility functions to get user information.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_PRIVILEGES_USER_HPP_
#define INC_WAGO_PRIVILEGES_USER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <string>

#include <sys/types.h>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace privileges {

//------------------------------------------------------------------------------
/// \brief Function to get the user ID for a given user name.
/// 
/// \param user_name Name of the user to get the ID for.
/// \return User ID of the user.
/// \throw std::runtime_error If the user ID could not be determined.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API uid_t get_uid_for_user(char const * const user_name);

//------------------------------------------------------------------------------
/// \brief Function to get the user name for a given user ID.
///
/// \param user_id ID of the user to get the name for.
/// \return Name of the user.
/// \throw std::runtime_error If the user name could not be determined.
//------------------------------------------------------------------------------
WAGO_PRIVILEGES_API std::string get_user_name(uid_t const user_id);

} // Namespace privileges
} // Namespace wago


#endif // INC_WAGO_PRIVILEGES_USER_HPP_
//---- End of source file ------------------------------------------------------
