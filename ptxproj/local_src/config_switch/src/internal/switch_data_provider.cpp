// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switch_data_provider.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <string>

namespace wago::libswitchconfig {

using wago::libswitchconfig::status;
using wago::libswitchconfig::switch_config;
using wago::libswitchconfig::switch_type;

void to_json(nlohmann::json& j, const ::wago::libswitchconfig::supported_values& sv);
void to_json(nlohmann::json& j, const ::wago::libswitchconfig::supported_values& sv) {
  j = nlohmann::json{{"version", sv.version},
                     {"broadcast-protection", sv.broadcast_protection_values},
                     {"multicast-protection", sv.multicast_protection_values}};
}
}  // namespace wago::libswitchconfig

namespace wago::config_switch {

status switch_data_provider::get_switch_type(switch_type& sw_type) const {
  return ::wago::libswitchconfig::get_switch_type(sw_type);
}

status switch_data_provider::read_switch_config(switch_config& config) const {
  return ::wago::libswitchconfig::read_switch_config(config);
}

status switch_data_provider::write_switch_config(const switch_config& config) const {
  return ::wago::libswitchconfig::write_switch_config(config);
}

status switch_data_provider::apply_switch_config(const switch_config& config) const {
  return ::wago::libswitchconfig::apply_switch_config(config);
}

status switch_data_provider::get_default_switch_config(switch_config& config) const {
  return ::wago::libswitchconfig::get_default_switch_config(config);
}

status switch_data_provider::validate(const switch_config& config) const {
  return ::wago::libswitchconfig::validate(config);
}

}  // namespace wago::config_switch
