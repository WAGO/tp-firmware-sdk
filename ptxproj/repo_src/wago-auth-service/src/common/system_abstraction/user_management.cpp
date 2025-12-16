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
///  \brief    System abstraction implementation for user management.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wago/privileges/group.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace sal {

class user_management_impl : public user_management
{
public:
    user_management_impl() noexcept = default;
    ~user_management_impl() noexcept override = default;

    std::string getgroupname(gid_t gid) const noexcept override;
    std::vector<std::string> get_groups(std::string const &user) const override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static user_management_impl default_user_management;

user_management *user_management::instance = &default_user_management;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
std::string user_management_impl::getgroupname(gid_t gid) const noexcept
{
    std::string group_name;
    try
    {
        group_name = privileges::get_group_name(gid);
    }
    catch(...) { }// parasoft-suppress CERT_CPP-ERR56-b-2 "No further error handling needed"
    
    return group_name;
}

std::vector<std::string> user_management_impl::get_groups(std::string const &user) const
{
    return privileges::get_user_group_names(user);
}


} // Namespace sal
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
