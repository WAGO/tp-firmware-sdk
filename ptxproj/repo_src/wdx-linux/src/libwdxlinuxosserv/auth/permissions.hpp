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
///  \brief    Permissions definition for WDx permissions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_AUTH_PERMISSIONS_HPP_
#define SRC_LIBWDXLINUXOSSERV_AUTH_PERMISSIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/permissions_i.hpp>

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace auth {

class permissions : public permissions_i
{
public:
    permissions() = default;
    WC_INTERFACE_IMPL_CLASS(permissions)

public:
    user_permissions get_user_permissions(std::string const &user_name) noexcept override;
    std::string get_permission_name(std::string const &feature_name,
                                    permissions_i::types const  type) const noexcept override;
};


} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_AUTH_PERMISSIONS_HPP_
//---- End of source file ------------------------------------------------------
