// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switch_data_provider.hpp"

#include <algorithm>
#include <nlohmann/json.hpp>
#include <string>

namespace wago::get_switch_config {

::wago::libswitchconfig::status switch_data_provider::get_switch_config(
    ::wago::libswitchconfig::switch_config& config) const {
  return ::wago::libswitchconfig::read_switch_config(config);
}

::wago::libswitchconfig::status switch_data_provider::get_supported_values(::wago::libswitchconfig::supported_values& sv) const {
  return ::wago::libswitchconfig::get_supported_values(sv);
}

::wago::libswitchconfig::status switch_data_provider::get_switch_type(
    ::wago::libswitchconfig::switch_type& sw_type) const {
  return ::wago::libswitchconfig::get_switch_type(sw_type);
}
}  // namespace wago::get_switch_config
