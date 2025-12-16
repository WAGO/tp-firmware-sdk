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
///  \brief    Utility functions grouping operations that create or modify or evaluate vectors
///
///  \author   PEn : WAGO GmbH & Co. KG
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_UTILS_VECTOR_OPERATIONS_HPP_
#define SRC_LIBAUTHSERV_UTILS_VECTOR_OPERATIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

bool is_subset(std::vector<std::string> const &subset,
               std::vector<std::string> const &superset);

std::vector<std::string> split_string(std::string const &elements,
                                      char        const  splitter);

} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_UTILS_VECTOR_OPERATIONS_HPP_
//---- End of source file ------------------------------------------------------
