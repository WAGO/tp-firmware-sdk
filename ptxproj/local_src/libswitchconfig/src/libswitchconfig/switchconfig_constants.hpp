// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <string>

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

// Version history:
// 1: Initial version with basic switch configuration.
// 2: Added multicast_protection_enabled parameter to switch_config.
constexpr auto switch_config_version1 = 1;
constexpr auto switch_config_version = 2;

constexpr auto broadcast_mac                  = "ff:ff:ff:ff:ff:ff";
constexpr auto multicast_mac                  = "01:00:00:00:00:00/01:00:00:00:00:00";
constexpr auto switch_config_persistence_path = "/etc/specific/switchconfig.json";

constexpr auto switch_name_ti     = "am65-cpsw-nuss";
constexpr auto switch_name_micrel = "ksz8863-switch";
constexpr auto switch_name_marvel = "mv88e6085";

::std::string to_port_label(const ::std::string& system_name);
::std::string to_system_name(const ::std::string& port_label);

::std::string get_storm_protection_unit(switch_type sw_type);

::std::uint32_t get_broadcast_protection_default_value(switch_type sw_type);
::std::uint32_t get_multicast_protection_default_value(switch_type sw_type);

::std::map<::std::string, ::std::uint32_t> get_ethport_hwid_mapping();
::std::map<::std::string, ::std::uint32_t> get_ethport_hwid_mapping_cc100();

supported_values get_supported_values(switch_type st);

}  // namespace wago::libswitchconfig