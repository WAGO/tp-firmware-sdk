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
///  \brief    Implementation of group management functions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "group_management.hpp"
#include "system_abstraction.hpp"

#include <algorithm>
#include <memory>

#include <wc/assertion.h>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
std::vector<std::string> get_groups(std::string const &user) noexcept
{
    try
    {
        return sal::user_management::get_instance().get_groups(user);
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, e.what());
        return { };
    }
}

bool is_authorized_for_scope(std::string              const &user,
                             std::vector<std::string> const &scopes)
{
    auto groups = get_groups(user);

    bool result = true;
    for(auto const &scope : scopes)
    {
        std::string scope_group_name(scope);
        std::replace(scope_group_name.begin(), scope_group_name.end(), ':', '_');
        // Check if user is member of corresponding group for scope (currently same name by convention)
        if(std::find(groups.begin(), groups.end(), scope_group_name) == groups.end())
        {
            result = false;
            break;
        }
    }

    return result;
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
