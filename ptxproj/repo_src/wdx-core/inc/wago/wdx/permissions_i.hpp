//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2021 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO parameter service permissions interface.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_PERMISSIONS_I_HPP_
#define INC_WAGO_WDX_PERMISSIONS_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "responses.hpp"

#include <wc/structuring.h>

#include <string>
#include <set>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {

/// Response for user permissions request sent to permissions_i interface.
struct user_permissions : public response
{
    /// User name of user this permissions grant for
    std::string           user_name;

    /// List of feature names with read permission
    std::set<std::string> read_permissions;

    /// List of feature names with write permission
    std::set<std::string> write_permissions;

    using response::response;

    /// Constructor for user permissions response.
    ///
    /// \param user_name_for_permissions
    ///   The user name for which the permissions are granted.
    /// \param user_read_permissions
    ///   he set of feature names with read permission.
    /// \param user_write_permissions
    ///   The set of feature names with write permission.
    user_permissions(std::string           user_name_for_permissions,
                     std::set<std::string> user_read_permissions,
                     std::set<std::string> user_write_permissions)
    : response(status_codes::success)
    , user_name(std::move(user_name_for_permissions))
    , read_permissions(std::move(user_read_permissions))
    , write_permissions(std::move(user_write_permissions))
    { }
};

/// This interface is used to determine the permissions of a user for accessing
/// features of the parameter service.
class permissions_i
{
    WC_INTERFACE_CLASS(permissions_i)

public:
    /// Enumeration of possible permission types
    enum types
    {
        readonly,
        readwrite
    };

    /// Determine permissions for a given user.
    ///
    /// \param user_name
    ///   User name to determine permissions for.
    ///
    /// \return
    ///   Permissions response for requested user.
    virtual user_permissions get_user_permissions(std::string const &user_name) noexcept = 0;

    /// Get permission name for a given feature and permission type.
    ///
    /// \param feature
    ///   Feature name to determine permission name for.
    ///
    /// \param type
    ///   Permission type to determine permission name for.
    ///
    /// \return
    ///   Permission name for given feature/type.
    ///   Empty value on any error.
    virtual std::string get_permission_name(std::string const &feature,
                                            types       const  type) const noexcept = 0;
};


} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_PERMISSIONS_I_HPP_
//---- End of source file ------------------------------------------------------
