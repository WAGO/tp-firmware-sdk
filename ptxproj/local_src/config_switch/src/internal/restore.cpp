// Copyright (c) 2025 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "restore.hpp"

#include <fstream>
#include <nlohmann/json.hpp>

#include "switch_config_api.hpp"
#include "switch_data_provider_i.hpp"

namespace wago::config_switch {

using wago::libswitchconfig::switch_config;

wago::libswitchconfig::status read_backup_file(::std::string &backup_content, const ::std::string &backup_file_path) {
  std::ifstream backup_file(backup_file_path);
  if (backup_file.is_open()) {
    backup_content = {(std::istreambuf_iterator<char>(backup_file)), std::istreambuf_iterator<char>()};
    backup_file.close();
    if (!backup_content.empty()) {
      return ::wago::libswitchconfig::status{};
    }
    return ::wago::libswitchconfig::status{libswitchconfig::status_code{1},
                                           "Backup file is empty: " + backup_file_path};
  }
  return ::wago::libswitchconfig::status{libswitchconfig::status_code{1},
                                         "Could not open backup file: " + backup_file_path};
}

wago::libswitchconfig::status get_switch_data_from_backup(wago::libswitchconfig::switch_config &config,
                                                          const ::std::string &backup_content) {
  std::istringstream iss(backup_content);
  std::string line;
  const std::string prefix = "switch.data=";
  std::string json_data;

  while (std::getline(iss, line)) {
    if (line.rfind(prefix, 0) == 0) {
      json_data += line.substr(prefix.size());
    }
  }

  if (json_data.empty()) {
    return ::wago::libswitchconfig::status{wago::libswitchconfig::status_code{1},
                                           "No valid switch data found in backup"};
  }

  nlohmann::json j = nlohmann::json::parse(json_data);
  return wago::libswitchconfig::switch_config_from_json(config, j);
}

// Micrel v0 config only
void set_port_mirror_parameters(switch_config &config, uint8_t port_mirror) {
  config.port_mirroring.enabled = (port_mirror != 0);
  if (config.port_mirroring.enabled) {
    config.port_mirroring.source      = (port_mirror == 1) ? std::string{"X1"} : std::string{"X2"};
    config.port_mirroring.destination = (port_mirror == 1) ? std::string{"X2"} : std::string{"X1"};
  }
}

// Micrel v0 config only
void set_broadcast_protection_parameters(switch_config &config, uint32_t bcast_protect) {

  for (auto & port : config.broadcast_protection.ports) {
    port.enabled = (bcast_protect > 0);
  }

  config.broadcast_protection.value = (bcast_protect > 0) ? bcast_protect : 1;
  config.broadcast_protection.unit  = "percent";
  config.broadcast_protection.ports.clear();
  config.broadcast_protection.ports.push_back({bcast_protect > 0, "X1"});
  config.broadcast_protection.ports.push_back({bcast_protect > 0, "X2"});
}

wago::libswitchconfig::status get_switch_data_v0_from_backup(const switch_data_provider_i &data_provider,
                                                             wago::libswitchconfig::switch_config &config,
                                                             const ::std::string &backup_content) {
  std::istringstream iss(backup_content);
  std::string line;
  std::string port_mirror_value;
  std::string bcast_protect_value;

  auto status = data_provider.get_default_switch_config(config);
  if (!status.ok()) {
    return status;
  }

  while (std::getline(iss, line)) {
    const std::string port_mirror_prefix   = "switch-port-mirror=";
    const std::string bcast_protect_prefix = "switch-bcast-protect=";

    if (line.rfind(port_mirror_prefix, 0) == 0) {
      port_mirror_value = line.substr(port_mirror_prefix.size());
    } else if (line.rfind(bcast_protect_prefix, 0) == 0) {
      bcast_protect_value = line.substr(bcast_protect_prefix.size());
    }

    if (!port_mirror_value.empty() && !bcast_protect_value.empty()) {
      break;  // Both values found, no need to continue reading
    }
  }

  if (port_mirror_value.empty() || bcast_protect_value.empty()) {
    return ::wago::libswitchconfig::status{wago::libswitchconfig::status_code{1},
                                           "Missing required parameters in v0 backup content"};
  }

  set_port_mirror_parameters(config, static_cast<uint8_t>(std::stoi(port_mirror_value)));
  set_broadcast_protection_parameters(config, static_cast<uint32_t>(std::stoi(bcast_protect_value)));
  config.version = 2;
  return ::wago::libswitchconfig::status{};
}

wago::libswitchconfig::status get_switch_config_from_backup(
    const ::wago::config_switch::switch_data_provider_i &data_provider, wago::libswitchconfig::switch_config &config,
    const ::std::string &backup_content) {
  auto status = get_switch_data_from_backup(config, backup_content);
  // for Micrel switches with FW<30, the backup configuration is not yet stored in JSON format and must be converted
  if (!status.ok()) {
    wago::libswitchconfig::switch_type st;
    auto s = data_provider.get_switch_type(st);
    if (s.ok() && st == wago::libswitchconfig::switch_type::MICREL) {
      status = get_switch_data_v0_from_backup(data_provider, config, backup_content);
    }
  }

  return status;
}

}  // namespace wago::config_switch
