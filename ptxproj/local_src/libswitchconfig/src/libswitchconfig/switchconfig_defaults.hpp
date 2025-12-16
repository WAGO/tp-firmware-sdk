// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <string>

#include "switch_config_api.hpp"
#include "iptool.hpp"

namespace wago::libswitchconfig {

void get_default_switch_config(switch_config& config, switch_type st, const ::std::vector<system_port>& system_ports);
void get_supported_values(supported_values& sv, switch_type st);

}  // namespace wago::libswitchconfig