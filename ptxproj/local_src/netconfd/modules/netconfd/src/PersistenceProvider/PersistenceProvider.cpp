// SPDX-License-Identifier: GPL-2.0-or-later

#include "PersistenceProvider.hpp"

#include "TypesHelper.hpp"

namespace netconf {

PersistenceProvider::PersistenceProvider(const ::std::string &persistence_path, DipSwitch &dip_switch,
                                         uint32_t device_port_count)
    : backup_restore_ { file_editor_, 75 },
      restore_legacy_ { file_editor_, device_port_count },
      persistence_executor_ { persistence_path, file_editor_, backup_restore_, restore_legacy_, dip_switch } {
}

Status PersistenceProvider::Write(const BridgeConfig &config) {
  return persistence_executor_.Write(config);
}

Status PersistenceProvider::Read(BridgeConfig &config) {
  return persistence_executor_.Read(config);
}

Status PersistenceProvider::Write(const IPConfigs &configs) {
  return persistence_executor_.Write(configs);
}

Status PersistenceProvider::Read(IPConfigs &configs) {
  Status status = persistence_executor_.Read(configs);
  RemoveUnnecessaryIPParameter(configs);
  return status;
}

Status PersistenceProvider::Read(BridgeConfig& config, IPConfigs& configs, Interfaces& interfaces) {
  return persistence_executor_.Read(config, configs, interfaces);
}

Status PersistenceProvider::Backup(const std::string &file_path, const std::string &targetversion) {
  return persistence_executor_.Backup(file_path, targetversion);
}

Status PersistenceProvider::Restore(const ::std::string &file_path, BridgeConfig &bridge_config, IPConfigs &ip_configs,
                                    InterfaceConfigs &interface_configs, DipSwitchIpConfig &dip_switch_config,
                                    Interfaces &interfaces) {
  return persistence_executor_.Restore(file_path, bridge_config, ip_configs, interface_configs, dip_switch_config, interfaces);
}

uint32_t PersistenceProvider::GetBackupParameterCount() const {
  return persistence_executor_.GetBackupParameterCount();
}

Status PersistenceProvider::Read(InterfaceConfigs &port_configs) {
  return persistence_executor_.Read(port_configs);
}

Status PersistenceProvider::Write(const InterfaceConfigs &port_configs) {
  return persistence_executor_.Write(port_configs);
}

Status PersistenceProvider::Read(DipSwitchIpConfig &config) {
  return persistence_executor_.Read(config);
}

Status PersistenceProvider::Write(const DipSwitchIpConfig &config) {
  return persistence_executor_.Write(config);
}

Status PersistenceProvider::Write(const Interfaces &config) {
  return persistence_executor_.Write(config);
}
void PersistenceProvider::Read(Interfaces &config) {
  persistence_executor_.Read(config);
}

}  // namespace netconf
