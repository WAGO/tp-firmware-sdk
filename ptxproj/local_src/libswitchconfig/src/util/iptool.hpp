// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

struct system_port {
  ::std::string name;
  ::std::string state;
};

status get_system_ports(::std::vector<system_port>& ports);

status get_system_ports_from_json(::std::vector<system_port>& ports, const ::nlohmann::json& j);

}  // namespace wago::libswitchconfig
