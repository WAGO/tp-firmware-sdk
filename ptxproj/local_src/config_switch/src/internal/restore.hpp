// Copyright (c) 2025 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "switch_config_api.hpp"
#include "switch_data_provider_i.hpp"

namespace wago::config_switch {

using wago::libswitchconfig::switch_config;

void set_port_mirror_parameters(switch_config &config, uint8_t port_mirror);
void set_broadcast_protection_parameters(switch_config &config, uint32_t bcast_protect);
wago::libswitchconfig::status get_switch_data_from_backup(wago::libswitchconfig::switch_config &config,
                                                          const ::std::string &backup_content);
wago::libswitchconfig::status get_switch_data_v0_from_backup(
    const ::wago::config_switch::switch_data_provider_i &data_provider, wago::libswitchconfig::switch_config &config,
    const ::std::string &backup_content);
wago::libswitchconfig::status read_backup_file(::std::string &backup_content, const ::std::string &backup_file_path);
wago::libswitchconfig::status get_switch_config_from_backup(
    const ::wago::config_switch::switch_data_provider_i &data_provider, wago::libswitchconfig::switch_config &config,
    const ::std::string &backup_content);

}  // namespace wago::config_switch
