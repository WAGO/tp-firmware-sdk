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
///  \brief    System abstraction implementation for user permission.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_serv.hpp"

#include <wago/privileges/group.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace sal {

class wdx_permission_impl : public wdx_permission
{
public:
    wdx_permission_impl() noexcept = default;
    ~wdx_permission_impl() noexcept override = default;

    std::vector<std::string> get_user_groups(std::string const &user_name) const override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static wdx_permission_impl default_permission;

wdx_permission *wdx_permission::instance = &default_permission;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
std::vector<std::string> wdx_permission_impl::get_user_groups(std::string const &user_name) const
{
    return privileges::get_user_group_names(user_name);
}


} // Namespace sal
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
