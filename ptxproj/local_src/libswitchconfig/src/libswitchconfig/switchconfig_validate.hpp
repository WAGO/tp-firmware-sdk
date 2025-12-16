// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

bool validate(const switch_config& config, const ::std::vector<::std::string>& port_names, switch_type st,
              ::std::stringstream& message);
bool validate(const port_mirror& pm, const ::std::vector<::std::string>& port_names, ::std::stringstream& message);

}  // namespace wago::libswitchconfig