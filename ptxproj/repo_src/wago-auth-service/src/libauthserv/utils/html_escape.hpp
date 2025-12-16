//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Escape special html characters from strings.
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

///
/// \brief
///     escape html characters and convert linebreaks for use in htmltext
///
/// \param
///     text string that will be converted
///
/// \return
///     html save string with escaped characters (only textnode save)
///
//------------------------------------------------------------------------------
std::string html_escape(std::string text);

} // Namespace authserv
} // Namespace wago

//---- End of source file ------------------------------------------------------

