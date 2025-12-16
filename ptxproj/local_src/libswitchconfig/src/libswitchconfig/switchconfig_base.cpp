// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switchconfig_base.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

#include "devlink_micrel.hpp"
#include "iptool.hpp"
#include "program.hpp"
#include "switch_config_api.hpp"
#include "switchconfig_constants.hpp"
#include "tc.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

::std::vector<::std::string> subtract(const ::std::vector<::std::string>& all,
                                      const ::std::vector<::std::string>& part) {
  ::std::vector<::std::string> difference;

  for (const auto& p : all) {
    auto it = std::find(part.cbegin(), part.cend(), p);
    if (it == part.end()) {
      difference.push_back(p);
    }
  }
  return difference;
}

}  // namespace

bool contains(const ::std::vector<::std::string>& values, const ::std::string& value) {
  auto it = ::std::find(values.cbegin(), values.cend(), value);
  return (it != values.cend());
}

::std::vector<::std::string> ports_requires_qdisc(const switch_config& config) {
  ::std::set<::std::string> ports;

  for (const auto& p : config.broadcast_protection.ports) {
    if (p.enabled) {
      ports.insert(p.name);
    }
  }

  for (const auto& p : config.multicast_protection.ports) {
    if (p.enabled) {
      ports.insert(p.name);
    }
  }

  if (config.port_mirroring.enabled) {
    ports.insert(config.port_mirroring.source);
  }

  return {ports.begin(), ports.end()};
}

::std::vector<::std::string> ports_requires_qdisc_not(const ::std::vector<system_port>& system_ports,
                                                  const ::std::vector<::std::string>& qdisc_ports) {
  ::std::vector<::std::string> port_names;
  ::std::transform(system_ports.begin(), system_ports.end(), ::std::back_inserter(port_names),
                   [](const system_port& sp) { return sp.name; });

  return subtract(port_names, qdisc_ports);
}

status get_port_hw_mapping(::std::map<::std::string, ::std::uint32_t>& port_hw_mapping) {
  auto cmd = "/etc/config-tools/get_typelabel_value -a";
  auto p   = program::execute(cmd);
  status s = p.get_result() == 0 ? status{} : status{status_code::SYSTEM_CALL_ERROR, p.get_stderr()};
  if (!s.ok()) {
    return s;
  }
  ::std::string typelabel = p.get_stdout();

  if (typelabel.find("MARKING=CC") != ::std::string::npos) {
    port_hw_mapping = get_ethport_hwid_mapping_cc100();
  }
  else {
    port_hw_mapping = get_ethport_hwid_mapping();
  }
  return s;
}

::std::vector<::std::string> get_system_ports_as_label() {
  ::std::vector<system_port> system_ports;
  get_system_ports(system_ports);

  ::std::vector<::std::string> port_names;
  ::std::transform(system_ports.begin(), system_ports.end(), ::std::back_inserter(port_names),
                   [](const system_port& sp) { return to_port_label(sp.name); });
  return port_names;
}

void convert_to_system_port_names(switch_config& config) {
  config.port_mirroring.source      = to_system_name(config.port_mirroring.source);
  config.port_mirroring.destination = to_system_name(config.port_mirroring.destination);
  for (auto& p : config.broadcast_protection.ports) {
    p.name = to_system_name(p.name);
  }
  for (auto& p : config.multicast_protection.ports) {
    p.name = to_system_name(p.name);
  }
}

switch_config clear_switch_unsopported_parameters(const switch_config& config, switch_type st) {
  switch_config filtered_config = config;
  if (st == switch_type::MICREL) {
    // MICREL does not support port specific multicast protection.
    // it only supports a global multicast protection enabled.
    filtered_config.multicast_protection = {};
  } else if (st == switch_type::TI || st == switch_type::MARVELL) {
    // TI and MARVEL does not support global multicast protection enabled.
    // it only supports port specific multicast protection.
    filtered_config.multicast_protection_enabled = false;
  }
  return filtered_config;
}

status get_switch_type(switch_type& st) {
  ::std::string info;
  st       = switch_type::UNKNOWN;
  status s = devlink_get_info(info);
  if (!s.ok()) {
    return s;
  }

  if (info.find(switch_name_micrel) != ::std::string::npos) {
    st = switch_type::MICREL;
  } else if (info.find(switch_name_ti) != ::std::string::npos) {
    st = switch_type::TI;
  } else if (info.find(switch_name_marvel) != ::std::string::npos) {
    st = switch_type::MARVELL;
  }

  if (st == switch_type::UNKNOWN) {
    return status{status_code::SYSTEM_CALL_ERROR, "Unknown switch type."};
  }
  return status{};
}

}  // namespace wago::libswitchconfig
