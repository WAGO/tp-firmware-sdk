
// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switchconfig_validate.hpp"

#include <exception>
#include <nlohmann/json.hpp>
#include <string>

#include "iptool.hpp"
#include "switch_config_api.hpp"
#include "switchconfig_base.hpp"
#include "switchconfig_constants.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

bool is_valid_port_name(const ::std::string& name, const ::std::vector<::std::string>& ports) {
  bool is_valid = false;
  for (const auto& p : ports) {
    if (name == p) {
      is_valid = true;
    }
  }
  return is_valid;
}

bool is_storm_protection_unset(const storm_protection& sp) {
  return sp.value == 0;
}

}  // namespace

bool validate(const switch_config& config, const ::std::vector<::std::string>& port_names, switch_type st,
              ::std::stringstream& message) {
  auto is_valid         = true;
  auto supported_values = get_supported_values(st);

  auto bcast        = config.broadcast_protection;
  auto bcast_values = supported_values.broadcast_protection_values;
  if (!is_storm_protection_unset(bcast)) {
    for (const auto& p : bcast.ports) {
      if (!is_valid_port_name(p.name, port_names)) {
        is_valid = false;
        message << "Broadcast-protection: Invalid port name: " << p.name << ::std::endl;
      }
    }

    if (std::find(bcast_values.begin(), bcast_values.end(), bcast.value) == bcast_values.end()) {
      is_valid = false;
      message << "Broadcast-protection: Invalid value: " << bcast.value << ". Supported values are: ";
      for (const auto& v : bcast_values) {
        message << v << " ";
      }
      message << ::std::endl;
    }

    if (bcast.unit != get_storm_protection_unit(st)) {
      is_valid = false;
      message << "Broadcast-protection: Invalid unit. The unit " << get_storm_protection_unit(st)
              << " will be supported." << ::std::endl;
    }
  }

  auto mcast        = config.multicast_protection;
  auto mcast_values = supported_values.multicast_protection_values;
  if (!is_storm_protection_unset(mcast)) {
    for (const auto& p : mcast.ports) {
      if (!is_valid_port_name(p.name, port_names)) {
        is_valid = false;
        message << "Multicast-protection: Invalid port name: " << p.name << ::std::endl;
      }
    }

    if (std::find(mcast_values.begin(), mcast_values.end(), mcast.value) == mcast_values.end()) {
      is_valid = false;
      message << "Multicast-protection: Invalid value: " << mcast.value << ". Supported values are: ";
      for (const auto& v : mcast_values) {
        message << v << " ";
      }
      message << ::std::endl;
    }

    if (mcast.unit != get_storm_protection_unit(st)) {
      is_valid = false;
      message << "Multicast-protection: Invalid unit. The unit " << get_storm_protection_unit(st)
              << " will be supported." << ::std::endl;
    }
  }

  return is_valid;
}

bool validate(const port_mirror& pm, const ::std::vector<::std::string>& port_names, ::std::stringstream& message) {
  bool is_valid = true;
  if (!pm.enabled) {
    // We ignore the port names if mirroring is disabled.
    // In this case, empty (unset) names are also accepted.
    return is_valid;
  }
  if (!is_valid_port_name(pm.source, port_names)) {
    message << "Port-Mirroring: Invalid source port name: " << pm.source << ::std::endl;
    is_valid = false;
  }
  if (!is_valid_port_name(pm.destination, port_names)) {
    message << "Port-Mirroring: Invalid destination port name: " << pm.destination << ::std::endl;
    is_valid = false;
  }
  if (is_valid && pm.source == pm.destination) {
    message << "Port-Mirroring: Source and destination must not be the same." << ::std::endl;
    is_valid = false;
  }

  return is_valid;
}

status validate(const switch_config& config) {
  switch_type st;
  status s = get_switch_type(st);

  auto sw_config = clear_switch_unsopported_parameters(config, st);

  auto port_names = get_system_ports_as_label();

  if (s.ok()) {
    ::std::stringstream error_message;
    if (!validate(sw_config, port_names, st, error_message) ||
        !validate(sw_config.port_mirroring, port_names, error_message)) {
      s = status(status_code::VALIDATION_ERROR, error_message.str());
    }
  }
  return status{s.get_code(), s.to_string()};
}

}  // namespace wago::libswitchconfig