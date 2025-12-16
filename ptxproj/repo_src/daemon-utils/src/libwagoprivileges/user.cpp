//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
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

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/privileges/user.hpp"
#include "utils/errno.hpp"

#include <stdexcept>
#include <pwd.h>

namespace wago {
namespace privileges {

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
uid_t get_uid_for_user(char const * const user_name)
{
    // 16k as also used in linux manpage example
    constexpr size_t   pw_buffer_size = 16384u;
    char               pw_buffer[pw_buffer_size];
    passwd             pw_data;
    passwd           * pw_result = nullptr;
    auto               pw_status = getpwnam_r(user_name, &pw_data, pw_buffer, pw_buffer_size, &pw_result);

    if(pw_result == nullptr)
    {
        std::string error_message = std::string("getpwnam_r: Unable to determine user ID for user \"") + user_name + "\": ";
        if(pw_status != 0)
        {
            error_message += utils::errno_to_string(pw_status);
        }
        else
        {
            error_message += "No user exists with that name.";
        }
        throw std::runtime_error(error_message);
    }

    return pw_data.pw_uid;
}

std::string get_user_name(uid_t const user_id)
{
    // 16k as also used in linux manpage example
    constexpr size_t   pw_buffer_size = 16384u;
    char               pw_buffer[pw_buffer_size];
    passwd             pw_data;
    passwd           * pw_result = nullptr;
    auto               pw_status = getpwuid_r(user_id, &pw_data, pw_buffer, pw_buffer_size, &pw_result);

    if(pw_result == nullptr)
    {
        std::string error_message = std::string("getpwuid_r: Unable to determine user name for UID \"") + std::to_string(user_id) + "\": ";
        if(pw_status != 0)
        {
            error_message += utils::errno_to_string(pw_status);
        }
        else
        {
            error_message += "No user exists with that UID.";
        }
        throw std::runtime_error(error_message);
    }

    return pw_data.pw_name;
}

} // Namespace privileges
} // Namespace wago


//---- End of source file ------------------------------------------------------
