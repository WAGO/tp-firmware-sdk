
// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switchconfig_defaults.hpp"

#include <cassert>
#include <nlohmann/json.hpp>
#include <string>

#include "iptool.hpp"
#include "switch_config_api.hpp"
#include "switchconfig_base.hpp"
#include "switchconfig_constants.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

void set_device_specific_storm_protection(storm_protection& sp, ::std::uint32_t default_value, ::std::string& unit,
                                          const ::std::vector<system_port>& system_ports, bool ports_enabled) {
  auto ports = system_ports;
  for (const auto& p : ports) {
    sp.ports.push_back({ports_enabled, to_port_label(p.name)});
  }

  sp.unit  = unit;
  sp.value = default_value;
}

}  // namespace

void get_default_switch_config(switch_config& config, switch_type st, const ::std::vector<system_port>& system_ports) {
  // The default configuration provides the following (taken from the YBP product requirements):
  // - Broadcast protection enabled with default rate (switch dependent, value not specified in the requirements)
  // - Multicast protection disabled
  // - Port mirroring disabled

  config.version        = switch_config_version;
  config.port_mirroring = {false, "X1", "X2"};

  config.multicast_protection_enabled = false;

  auto unit = get_storm_protection_unit(st);

  auto bcast_default = get_broadcast_protection_default_value(st);
  if(st == switch_type::TI){
    // The TI switch enforces its protection very strictly, resulting in broadcast telegrams being discarded immediately.
    // Short-term bursts are thus immediately suppressed, which can cause problems for the customer.
    // (Leaky bucket behavior is not supported.)
    // Therefore, broadcast protection is switched off by default for the TI switch.
    set_device_specific_storm_protection(config.broadcast_protection, bcast_default, unit, system_ports, false);
  } else {
    set_device_specific_storm_protection(config.broadcast_protection, bcast_default, unit, system_ports, true);
  }

  if(st != switch_type::MICREL) {
    auto mcast_default = get_multicast_protection_default_value(st);
    set_device_specific_storm_protection(config.multicast_protection, mcast_default, unit, system_ports, false);
  }
}

status get_default_switch_config(switch_config& config) {
  ::std::vector<system_port> system_ports;
  status s = get_system_ports(system_ports);

  switch_type st;
  if (s.ok()) {
    s = get_switch_type(st);
  }

  if (s.ok()) {
    get_default_switch_config(config, st, system_ports);
  }
  return status{s.get_code(), "Get default config error: " + s.to_string()};
}

status get_supported_values(supported_values& sv) {
  switch_type st;
  status s = get_switch_type(st);

  if (s.ok()) {
    sv = get_supported_values(st);
  }
  return status{s.get_code(), "Get supported values error: " + s.to_string()};
}

}  // namespace wago::libswitchconfig
