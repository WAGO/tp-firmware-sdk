
// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <exception>
#include <nlohmann/json.hpp>
#include <string>

#include "alphanum.hpp"
#include "file.hpp"
#include "switch_config_api.hpp"
#include "switchconfig_base.hpp"
#include "switchconfig_constants.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

void sort_storm_protection_ports(storm_protection& protection) {
  // clang-format off
    auto natural_less = [](const port& lhs, const port& rhs) {
       return doj::alphanum_comp(lhs.name, rhs.name) < 0;
    };

    ::std::sort(protection.ports.begin(), protection.ports.end(), natural_less);
  // clang-format on
}

}  // namespace

status read_switch_config(switch_config& config) {
  status s{};
  ::std::stringstream ss;

  s = read(ss, switch_config_persistence_path);
  if (s.ok()) {
    try {
      json j = json::parse(ss);
      s      = switch_config_from_json(config, j);
    } catch (const ::std::exception& e) {
      s = {status_code::JSON_PARSE_ERROR, "Failed to parse json: " + ::std::string(e.what())};
    }
  }

  return status{s.get_code(), s.to_string()};
}

status write_switch_config(const switch_config& config) {
  switch_type st;
  status s = get_switch_type(st);

  auto sw_config = clear_switch_unsopported_parameters(config, st);

  if(s.ok()){
    s = validate(sw_config);
  }

  if (s.ok()) {
    sort_storm_protection_ports(sw_config.broadcast_protection);
    sort_storm_protection_ports(sw_config.multicast_protection);

    json j;
    switch_config_to_json(j, sw_config);
    std::stringstream ss;
    ss << std::setw(4) << j;
    s = write(ss, switch_config_persistence_path);
  }

  return status{s.get_code(), s.to_string()};
}
}  // namespace wago::libswitchconfig