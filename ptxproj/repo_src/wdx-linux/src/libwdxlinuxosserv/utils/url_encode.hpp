//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Encode strings for URLs.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_UTILS_URL_ENCODE_HPP_
#define SRC_LIBWDXLINUXOSSERV_UTILS_URL_ENCODE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

std::string encode_url(std::string const &raw_text);
std::string decode_url(std::string const &encoded_text);

} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_UTILS_URL_ENCODE_HPP_
//---- End of source file ------------------------------------------------------
