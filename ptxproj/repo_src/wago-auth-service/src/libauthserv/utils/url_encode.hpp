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
///  \brief    Encode strings for URLs.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_UTILS_URL_ENCODE_HPP_
#define SRC_LIBAUTHSERV_UTILS_URL_ENCODE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

std::string encode_url(std::string const &text);
std::string decode_url(std::string const &encoded_text);

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_UTILS_URL_ENCODE_HPP_
//---- End of source file ------------------------------------------------------
