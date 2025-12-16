//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    URL util collection.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_HTTP_URL_UTILS_HPP_
#define SRC_LIBWDXWDA_HTTP_URL_UTILS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

namespace wago {
namespace wdx {
namespace wda {
namespace http {
//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
std::string build_url_regex_pattern(std::string const &pattern_template,
                                    bool               allow_trailing_slash,
                                    bool               match_following);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char const parameter_marker          = ':';
constexpr char const parameter_marker_path     = '#';
// This may work on C++17
//constexpr char const parameter_markers[3]      = { parameter_marker, parameter_marker_path, '\0' };
constexpr char const * const parameter_markers = ":#";


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_HTTP_URL_UTILS_HPP_
//---- End of source file ------------------------------------------------------
