//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Definitions specific to (inter-process) communication
///            for WAGO Parameter Service.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_DEFINITIONS_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_DEFINITIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {
//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
/// Group name for users able to access frontend socket for backend connection.
constexpr char const frontend_user_group[]    = "wdaadm";
/// Socket path for frontend connection.
constexpr char const frontend_domain_socket[] = "/run/paramd/wdaadm/frontend.socket";

/// Group name for users able to access file api socket for backend connection.
constexpr char const file_api_user_group[]    = "wdaadm";
/// Socket path for file api connection.
constexpr char const file_api_domain_socket[] = "/run/paramd/wdaadm/fileapi.socket";

/// Group name for users able to access backend socket for backend connection.
constexpr char const backend_user_group[]    = "wdxprov";
/// Socket path for backend connection.
constexpr char const backend_domain_socket[] = "/run/paramd/wdxprov/backend.socket";

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_DEFINITIONS_HPP_
//---- End of source file ------------------------------------------------------
