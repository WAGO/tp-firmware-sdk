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
///  \brief    Permissions implementation for WDx permissions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "permissions.hpp"
#include "system_abstraction_serv.hpp"
#include "string_utils.hpp"

#include <wc/assertion.h>

#include <algorithm>
#include <iostream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace auth {

namespace {
constexpr char const wdx_permission_prefix[]             = "wdx-";
constexpr auto const wdx_permission_prefix_length        = sizeof(wdx_permission_prefix) - 1;
constexpr char const wdx_read_permission_postfix[]       = "-ro";
constexpr auto const wdx_read_permission_postfix_length  = sizeof(wdx_read_permission_postfix) - 1;
constexpr char const wdx_write_permission_postfix[]      = "-rw";
constexpr auto const wdx_write_permission_postfix_length = sizeof(wdx_write_permission_postfix) - 1;

inline bool has_permission_with_postfix(std::string const &group_name,
                                        char        const  postfix[],
                                        size_t      const  postfix_length)
{
    if(group_name.length() <= (wdx_permission_prefix_length + postfix_length))
    {
        return false;
    }
    size_t const postfix_start = group_name.length() - postfix_length;
    bool   const has_postfix   = group_name.compare(postfix_start, postfix_length, postfix) == 0;

    return has_postfix;
}

inline bool is_read_permission(std::string const &group_name)
{
    return has_permission_with_postfix(group_name, wdx_read_permission_postfix, wdx_read_permission_postfix_length);
}

inline bool is_write_permission(std::string const &group_name)
{
    return has_permission_with_postfix(group_name, wdx_write_permission_postfix, wdx_write_permission_postfix_length);
}

inline void insert_permission(std::string           const &group_name,
                              std::set<std::string>       &container)
{
    WC_STATIC_ASSERT(wdx_write_permission_postfix_length == wdx_read_permission_postfix_length);
    size_t const postfix_length = wdx_read_permission_postfix_length;

    // Groups without WDx prefix have to be already removed at this point
    WC_ASSERT(group_name.compare(0, wdx_permission_prefix_length, wdx_permission_prefix) == 0);

    // Groups without permission postfix should not be inserted
    WC_ASSERT(group_name.length() > (wdx_permission_prefix_length + postfix_length));

    size_t const feature_name_length = group_name.length() - wdx_permission_prefix_length - postfix_length;
    container.insert(tolower_copy(group_name.substr(wdx_permission_prefix_length, feature_name_length)));
}
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
user_permissions permissions::get_user_permissions(std::string const &user_name) noexcept
{
    try
    {
        auto groups = serv::sal::wdx_permission::get_instance().get_user_groups(user_name);

        std::remove_if(groups.begin(), groups.end(), [](std::string const &group_name){
            return group_name.compare(0, wdx_permission_prefix_length, wdx_permission_prefix) != 0;
        });

        std::set<std::string> read_permissions;
        std::set<std::string> write_permissions;
        for(auto const &group_name : groups)
        {
            if(is_read_permission(group_name))
            {
                insert_permission(group_name, read_permissions);
            }
            else if(is_write_permission(group_name))
            {
                insert_permission(group_name, write_permissions);

                // Write permissions implicitly are also read permissions
                insert_permission(group_name, read_permissions);
            }
        }

        return user_permissions(user_name, std::move(read_permissions), std::move(write_permissions));
    }
    catch(std::exception const &e)
    {
        return user_permissions(status_codes::internal_error, "Failed to determine user permissions: " + std::string(e.what()));
    }
}

std::string permissions::get_permission_name(std::string          const &feature_name,
                                             permissions_i::types const  type) const noexcept
{
    try
    {
        std::string permission_type_postfix;
        switch(type)
        {
            case permissions_i::types::readonly:
                permission_type_postfix = wdx_read_permission_postfix;
                break;

            case permissions_i::types::readwrite:
                permission_type_postfix = wdx_write_permission_postfix;
                break;

            default:
                WC_FAIL("Missing permission type implementation!");
        }

        std::string permission_name = wdx_permission_prefix + feature_name + permission_type_postfix;
        std::transform(permission_name.begin(), permission_name.end(), permission_name.begin(),
                    [](unsigned char c){ return std::tolower(c); });

        return permission_name;
    }
    catch(...)
    {
        return "";
    }
}


} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
