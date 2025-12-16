
// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switchconfig_constants.hpp"

#include <nlohmann/json.hpp>
#include <string>

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

::std::string to_port_label(const ::std::string& system_name) {
  if (system_name.rfind("eth", 0) == 0) {
    return system_name.substr(3);
  }
  return system_name;
}

::std::string to_system_name(const ::std::string& port_label) {
  if (port_label.rfind("ethX", 0) == 0) {
    return port_label;
  }
  return "eth" + port_label;
}

::std::string get_storm_protection_unit(switch_type sw_type) {
  if (sw_type == switch_type::MICREL) {
    return "percent";
  } else if (sw_type == switch_type::TI) {
    return "pps";
  } else if (sw_type == switch_type::MARVELL) {
    return "Mbits";
  } else {
    return "";
  }
}

//TODO (Team): we have to check min max and default values.

::std::uint32_t get_broadcast_protection_default_value(switch_type st) {
  ::std::map<switch_type, ::std::uint32_t> values = {
      {switch_type::MICREL, 1}, {switch_type::TI, 20000}, {switch_type::MARVELL, 1}};
  return values[st];
}

::std::uint32_t get_multicast_protection_default_value(switch_type st) {
  ::std::map<switch_type, ::std::uint32_t> values = {{switch_type::TI, 20000}, {switch_type::MARVELL, 1}};
  return values[st];
}

// The port to HWID mapping is currently only required for the Micrel 2 port devices.
// The CC100 is an exception, as it doesn't have the consistent port-to-ID mapping. It's reversed.
// ONLY MICREL 2-Port:
::std::map<::std::string, ::std::uint32_t> get_ethport_hwid_mapping() {
  return {{"ethX1", 1}, {"ethX2", 0}, {"cpu", 2}};
}
::std::map<::std::string, ::std::uint32_t> get_ethport_hwid_mapping_cc100() {
  return {{"ethX1", 0}, {"ethX2", 1}, {"cpu", 2}};
}

supported_values get_supported_values(switch_type st) {
  supported_values sv{};
  sv.version = switch_config_version;

  if (st == switch_type::MICREL) {
    for (::std::uint32_t i = 1; i <= 20; i += 1) {
      sv.broadcast_protection_values.push_back(i);
    }
  }

  if (st == switch_type::TI) {
    for (::std::uint32_t i = 1000; i < 10000; i += 1000) {
      sv.broadcast_protection_values.push_back(i);
      sv.multicast_protection_values.push_back(i);
    }

    for (::std::uint32_t i = 10000; i <= 100000; i += 10000) {
      sv.broadcast_protection_values.push_back(i);
      sv.multicast_protection_values.push_back(i);
    }

    sv.broadcast_protection_values.push_back(250000);
    sv.multicast_protection_values.push_back(250000);
  }

  if (st == switch_type::MARVELL) {
    for (::std::uint32_t i = 1; i <= 20; i += 1) {
      sv.broadcast_protection_values.push_back(i);
      sv.multicast_protection_values.push_back(i);
    }
  }

  return sv;
}

}  // namespace wago::libswitchconfig
