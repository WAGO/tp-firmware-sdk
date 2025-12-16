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
///  \brief    Functions for combined parameter filtering.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_PARAMETER_FILTER_HPP_
#define SRC_LIBWDXWDA_REST_PARAMETER_FILTER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/requests.hpp"

#include <string>
#include <map>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

// Signature of a filter extractor function
//   returns true, if required filter names matches and filter value applicable
using filter_extractor = bool(*)(wdx::parameter_filter       &filter,
                                 std::string           const &required_filter_name,
                                 std::string           const &filter_name,
                                 std::string           const &filter_value);


bool combine_with_beta_filter(wdx::parameter_filter       &filter,
                              std::string           const &required_filter_name,
                              std::string           const &filter_name,
                              std::string           const &filter_value);

bool combine_with_deprecated_filter(wdx::parameter_filter       &filter,
                                    std::string           const &required_filter_name,
                                    std::string           const &filter_name,
                                    std::string           const &filter_value);

bool combine_with_writeable_filter(wdx::parameter_filter       &filter,
                                   std::string           const &required_filter_name,
                                   std::string           const &filter_name,
                                   std::string           const &filter_value);

bool combine_with_userSetting_filter(wdx::parameter_filter       &filter,
                                     std::string           const &required_filter_name,
                                     std::string           const &filter_name,
                                     std::string           const &filter_value);

bool combine_with_device_filter(wdx::parameter_filter       &filter,
                                std::string           const &required_filter_name,
                                std::string           const &filter_name,
                                std::string           const &filter_value);

bool combine_with_path_filter(wdx::parameter_filter       &filter,
                              std::string           const &required_filter_name,
                              std::string           const &filter_name,
                              std::string           const &filter_value);

// Extract a combination of applicable filters from given query filters
wdx::parameter_filter extract_filters_from_query(std::map<std::string, filter_extractor> const &filter_extractors_by_filter_name,
                                                 std::map<std::string, std::string>      const &filter_query_parameters);


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_PARAMETER_FILTER_HPP_
//---- End of source file ------------------------------------------------------
