//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility functions to handle privileges.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MeHE: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/privileges/privileges.hpp"
#include "wago/privileges/user.hpp"
#include "wago/privileges/group.hpp"
#include "utils/errno.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <string>
#include <stdexcept>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

namespace wago {
namespace privileges {
//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {

void drop_privileges(uid_t        const new_user_id,
                     gid_t        const new_group_id,
                     char const * const new_user_name);
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

/// User ID for root.
constexpr uid_t const root_uid = 0U;
/// Group ID for root.
constexpr gid_t const root_gid = 0U;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void drop_privileges(char const * new_user_name,
                     char const * new_group_name)
{
    if(new_user_name == nullptr)
    {
        constexpr char const * error_message = "No user name given, unable to drop privileges.";
        wc_log(log_level_t::fatal, error_message);
        throw std::runtime_error(error_message);
    }

    // Use same name for group as for user when no group name is given
    if(new_group_name == nullptr)
    {
        new_group_name = new_user_name;
    }

    uid_t const new_user_id  = get_uid_for_user(new_user_name);
    gid_t const new_group_id = get_gid_for_group(new_group_name);

    return drop_privileges(new_user_id, new_group_id, new_user_name);
}

void drop_privileges(uid_t const new_user_id,
                     gid_t const new_group_id)
{
    WC_ASSERT_RETURN(new_user_id  != root_uid, void());
    WC_ASSERT_RETURN(new_group_id != root_gid, void());

    auto new_user_name = get_user_name(new_user_id);

    return drop_privileges(new_user_id, new_group_id, new_user_name.c_str());
}

namespace {

void drop_privileges(uid_t        const new_user_id,
                     gid_t        const new_group_id,
                     char const * const new_user_name)
{
    WC_ASSERT_RETURN(new_user_id   != root_uid, void());
    WC_ASSERT_RETURN(new_group_id  != root_gid, void());
    WC_ASSERT_RETURN(new_user_name != nullptr,  void());

    if(getuid() == root_uid)
    {
        // Init groups for target user to enable also supplementary groups
        if(initgroups(new_user_name, new_group_id) != 0)
        {
            std::string const error_message = std::string("initgroups: Unable to initialize groups: ") + utils::errno_to_string(errno);
            wc_log(log_level_t::fatal, error_message.c_str());
            throw std::runtime_error(error_message);
        }

        // Drop group privileges
        if(setresgid(new_group_id, new_group_id, new_group_id) != 0)
        {
            std::string const error_message = std::string("setgid: Unable to drop group privileges: ") + utils::errno_to_string(errno);
            wc_log(log_level_t::fatal, error_message.c_str());
            throw std::runtime_error(error_message);
        }

        // Drop user privileges
        if(setresuid(new_user_id, new_user_id, new_user_id) != 0)
        {
            std::string const error_message = std::string("setuid: Unable to drop user privileges: ") + utils::errno_to_string(errno);
            wc_log(log_level_t::fatal, error_message.c_str());
            throw std::runtime_error(error_message);
        }

        // Check if dropped permanently
        if(setuid(root_uid) != -1)
        {
            std::string const error_message = std::string("Became root user again after dropping privileges, exit for security reasons");
            wc_log(log_level_t::fatal, error_message.c_str());
            throw std::runtime_error(error_message);
        }
    }
}

}

} // Namespace privileges
} // Namespace wago


//---- End of source file ------------------------------------------------------
