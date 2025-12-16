//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO Parameter Service client option structure.
///
///  \author   MRö: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_CLIENT_CLIENT_OPTION_HPP_
#define INC_WAGO_WDX_LINUXOS_CLIENT_CLIENT_OPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

//------------------------------------------------------------------------------
/// \brief Client option structure
//------------------------------------------------------------------------------
struct client_option final
{
    /// \brief Single character as short option
    char         const  short_option;
    /// \brief Short text without spaces as long option
    char const * const  long_option;
    /// \brief Short help text for option
    char const * const  help_text;
    /// \brief Variable to store option argument value
    std::string        &value;
};



} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_CLIENT_CLIENT_OPTION_HPP_
//---- End of source file ------------------------------------------------------
