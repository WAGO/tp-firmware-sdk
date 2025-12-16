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
///  \brief    Utility functions to get group information.
///
///  \author   MeHE: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/privileges/group.hpp"
#include "utils/errno.hpp"

#include <wc/std_includes.h>

#include <string>
#include <stdexcept>
#include <grp.h>

namespace wago {
namespace privileges {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {
constexpr int const usual_group_count =        256;
constexpr int const max_group_count   = 100 * 1024;
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
gid_t get_gid_for_group(char const * const group_name)
{
    // 16k as also used in linux manpage example
    constexpr size_t   gr_buffer_size = 16384u;
    char               gr_buffer[gr_buffer_size];
    group              gr_data;
    group            * gr_result = nullptr;
    auto               gr_status = getgrnam_r(group_name, &gr_data, gr_buffer, gr_buffer_size, &gr_result);

    if(gr_result == nullptr)
    {
        std::string error_message = std::string("getgrnam_r: Unable to determine group ID for group \"") + group_name + "\": ";
        if(gr_status != 0)
        {
            error_message += utils::errno_to_string(gr_status);
        }
        else
        {
            error_message += "No group exists with that name.";
        }
        throw std::runtime_error(error_message);
    }

    return gr_data.gr_gid;
}

std::string get_group_name(gid_t const group_id)
{
    // 16k as also used in linux manpage example
    constexpr size_t   gr_buffer_size = 16384u;
    char               gr_buffer[gr_buffer_size];
    group              gr_data;
    group            * gr_result = nullptr;
    auto               gr_status = getgrgid_r(group_id, &gr_data, gr_buffer, gr_buffer_size, &gr_result);

    if(gr_result == nullptr)
    {
        std::string error_message = std::string("getgrgid_r: Unable to determine group name for GID \"") + std::to_string(group_id) + "\": ";
        if(gr_status != 0)
        {
            error_message += utils::errno_to_string(gr_status);
        }
        else
        {
            error_message += "No group exists with that GID.";
        }
        throw std::runtime_error(error_message);
    }

    return gr_data.gr_name;
}

std::vector<std::string> get_user_group_names(std::string const &user_name)
{
    std::vector<std::string> result;
    gid_t static_groups[usual_group_count];
    gid_t *groups   = &static_groups[0];
    int group_count = usual_group_count;
    if(::getgrouplist(user_name.c_str(), 0, groups, &group_count) < 0)
    {
        if(group_count > max_group_count)
        {
            throw std::runtime_error("Unable to get groups: Too many groups for user " + user_name);
        }
        int const dynamic_group_size = group_count + 1024;
        groups      = new gid_t[dynamic_group_size]; // parasoft-suppress CERT_CPP-MEM52-a-1 "No valid situation on Linux where NULL is returned"
        group_count = dynamic_group_size;
        if(::getgrouplist(user_name.c_str(), 0, groups, &group_count) < 0)
        {
            delete [] groups;
            throw std::runtime_error("Unable to get groups for user " + user_name);
        }
    }
    for(int i = 0; i < group_count; ++i)
    {
        auto group_name = get_group_name(groups[i]);
        if(!group_name.empty())
        {
            result.emplace_back(group_name);
        }
    }

    if(groups != &static_groups[0])
    {
        delete[] groups;
    }

    return result;
}

} // Namespace privileges
} // Namespace wago


//---- End of source file ------------------------------------------------------
