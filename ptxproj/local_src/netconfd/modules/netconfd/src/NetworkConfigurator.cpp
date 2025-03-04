// SPDX-License-Identifier: GPL-2.0-or-later

#include "NetworkConfigurator.hpp"

#include <cerrno>
#include <cstring>
#include <utility>

#include "BridgeManager.hpp"
#include "DBusHandlerRegistry.h"
#include "DipSwitch.hpp"
#include "DynamicIPClientAdministrator.hpp"
#include "EthernetInterfaceFactory.hpp"
#include "EventManager.hpp"
#include "IPManager.hpp"
#include "Logger.hpp"
#include "MacDistributor.hpp"
#include "NetlinkLink.hpp"
#include "NetDevManager.hpp"
#include "NetlinkAddressCache.hpp"
#include "NetlinkLinkCache.hpp"
#include "NetlinkMonitor.hpp"
#include "NetworkConfigBrain.hpp"
#include "PersistenceProvider.hpp"
#include "Redundancy.hpp"
#include "Server.h"
#include "UriEscape.hpp"
#include "HostnameManager.hpp"
#include "DeviceTypeLabel.hpp"
#include "HostnameController.hpp"

namespace netconf {

using namespace std::string_literals;

class NetworkConfiguratorImpl {
 public:
  explicit NetworkConfiguratorImpl(InterprocessCondition &start_condition, StartWithPortstate startWithPortState);
  virtual ~NetworkConfiguratorImpl() = default;

  NetworkConfiguratorImpl(const NetworkConfiguratorImpl&) = delete;
  NetworkConfiguratorImpl& operator=(const NetworkConfiguratorImpl&) = delete;
  NetworkConfiguratorImpl(const NetworkConfiguratorImpl&&) = delete;
  NetworkConfiguratorImpl& operator=(const NetworkConfiguratorImpl&&) = delete;

 private:
  const ::std::string persistence_file_path = "/etc/specific"s;
  const ::std::string DEV_DIP_SWITCH_VALUE = "/dev/dip-switch/value"s;

  NetlinkLink netlink_link_;
  NetlinkMonitor netlink_monitor_;
  ::std::shared_ptr<IInterfaceMonitor> interface_monitor_;
  ::std::shared_ptr<IIPMonitor> ip_monitor_;
  CommandExecutor command_executer_;
  DeviceTypeLabel device_type_label_;
  NetDevManager netdev_manager_;
  MacDistributor mac_distributor_;
  EventManager event_manager_;
  DipSwitch ip_dip_switch_;
  PersistenceProvider persistence_provider_;
  Redundancy stp_;
  BridgeManager bridge_manager_;
  EthernetInterfaceFactory ethernet_interface_factory_;
  InterfaceConfigManager interface_manager_;
  DynamicIPClientAdministrator dyn_ip_client_admin_;
  IPController ip_controller_;
  HostnameManager hostname_manager_;
  IPManager ip_manager_;
  NetworkConfigBrain network_config_brain_;

  dbus::Server dbus_server_;
  dbus::DBusHandlerRegistry dbus_handler_registry_;
  ::UriEscape uri_escape_;
};

NetworkConfiguratorImpl::NetworkConfiguratorImpl(InterprocessCondition &start_condition,
                                                 StartWithPortstate startWithPortState)
    : interface_monitor_ { static_cast<::std::shared_ptr<IInterfaceMonitor>>(netlink_monitor_.Add<NetlinkLinkCache>()) },
      ip_monitor_ { static_cast<::std::shared_ptr<IIPMonitor>>(netlink_monitor_.Add<NetlinkAddressCache>()) },
      device_type_label_ { command_executer_ },
      netdev_manager_ { interface_monitor_, event_manager_, netlink_link_},
      mac_distributor_ { device_type_label_.GetMac(), device_type_label_.GetMacCount(), netdev_manager_ },
      ip_dip_switch_ { DEV_DIP_SWITCH_VALUE },
      persistence_provider_ { persistence_file_path, ip_dip_switch_, static_cast<uint32_t>(netdev_manager_.GetNetDevs({DeviceType::Port}).size()) },
      bridge_manager_ { netdev_manager_, mac_distributor_, stp_, device_type_label_.GetOrderNumber() },
      interface_manager_ { netdev_manager_, persistence_provider_, ethernet_interface_factory_ },
      dyn_ip_client_admin_ {device_type_label_.GetOrderNumber() },
      hostname_manager_{device_type_label_.GetMac()},
      ip_manager_ { event_manager_, persistence_provider_, netdev_manager_, ip_dip_switch_, interface_manager_,
          dyn_ip_client_admin_, ip_controller_, ip_monitor_, hostname_manager_},
      network_config_brain_ { bridge_manager_, bridge_manager_, ip_manager_, event_manager_,
          persistence_provider_, ip_dip_switch_, interface_manager_, netdev_manager_, hostname_manager_ } {

  event_manager_.RegisterNetworkInformation(persistence_provider_, ip_manager_, interface_manager_, netdev_manager_);

  dbus_server_.AddInterface(dbus_handler_registry_);

  dbus_handler_registry_.RegisterSetBridgeConfigHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: SetBridgeConfig: " + data);
    data = this->uri_escape_.Unescape(data);
    return this->network_config_brain_.SetBridgeConfig(data);
  });

  dbus_handler_registry_.RegisterGetBridgeConfigHandler([this](std::string &data) -> std::string {
    auto status = this->network_config_brain_.GetBridgeConfig(data);
    LOG_DEBUG("DBUS Req: GetBridgeConfig: " + data);
    return status;
  });

  dbus_handler_registry_.RegisterGetDeviceInterfacesHandler([this](std::string &data) -> std::string {
    auto status = this->network_config_brain_.GetInterfaceInformation(data);
    LOG_DEBUG("DBUS Req: GetInterfaceInformation: " + data);
    return status;
  });

  dbus_handler_registry_.RegisterSetInterfaceConfigHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: SetInterfaceConfig " + data);
    data = this->uri_escape_.Unescape(data);
    return this->network_config_brain_.SetInterfaceConfig(data);
  });

  dbus_handler_registry_.RegisterGetInterfaceConfigHandler([this](std::string &data) -> ::std::string {
    auto status = this->network_config_brain_.GetInterfaceConfig(data);
    LOG_DEBUG("DBUS Req: GetInterfaceConfig" + data);
    return status;
  });

  dbus_handler_registry_.RegisterGetInterfaceStatusesHandler([this](std::string &data) -> ::std::string {
    auto status = this->network_config_brain_.GetInterfaceStatuses(data);
    LOG_DEBUG("DBUS Req: GetInterfaceStatuses" + data);
    return status;
  });

  // Backup and Restore API
  dbus_handler_registry_.RegisterGetBackupParamCountHandler([this]() -> std::string {
    auto data = this->network_config_brain_.GetBackupParamterCount();
    LOG_DEBUG("DBUS Req: GetBackupParamCount: " + data);
    return data;
  });

  dbus_handler_registry_.RegisterBackupHandler([this](std::string file_path, std::string targetversion) {
    LOG_DEBUG("DBUS Req: Backup: " + file_path + "; targetversion=" + targetversion);
    return this->network_config_brain_.Backup(file_path, targetversion);
  });

  dbus_handler_registry_.RegisterRestoreHandler([this](std::string file_path) {
    LOG_DEBUG("DBUS Req: Restore: " + file_path);
    return this->network_config_brain_.Restore(file_path);
  });

  // IP config
  dbus_handler_registry_.RegisterSetAllIPConfigsHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: SetAllIPConfigs : " + data);
    data = this->uri_escape_.Unescape(data);
    return this->network_config_brain_.SetAllIPConfigs(data);
  });

  dbus_handler_registry_.RegisterSetIPConfigHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: SetIPConfig : " + data);
    data = this->uri_escape_.Unescape(data);
    return this->network_config_brain_.SetIPConfig(data);
  });

  dbus_handler_registry_.RegisterGetAllIPConfigsHandler([this](std::string &data) -> std::string {
    auto status = this->network_config_brain_.GetAllIPConfigs(data);
    LOG_DEBUG("DBUS Req: GetAllIPConfigs: " + data);
    return status;
  });

  dbus_handler_registry_.RegisterGetAllCurrentIPConfigsHandler([this](std::string &data) -> std::string {
    auto status = this->network_config_brain_.GetCurrentIPConfigs(data);
    LOG_DEBUG("DBUS Req: GetAllCurrentIPConfigs: " + data);
    return status;
  });

  dbus_handler_registry_.RegisterTempFixIpHandler([this]() -> ::std::string {
    LOG_DEBUG("DBUS Req: SetTemporaryFixIp");
    return this->network_config_brain_.SetTemporaryFixIp();
  });

  dbus_handler_registry_.RegisterGetDipSwitchConfigHandler([this](std::string &data) -> std::string {
    auto status = this->network_config_brain_.GetDipSwitchConfig(data);
    LOG_DEBUG("DBUS Req: GetDipSwitchConfig" + data);
    return status;
  });

  dbus_handler_registry_.RegisterSetDipSwitchConfigHandler([this](std::string data_in) {
    LOG_DEBUG("DBUS Req: SetDipSwitchConfig");
    auto status = this->network_config_brain_.SetDipSwitchConfig(data_in);
    return status;
  });

  // events
  dbus_handler_registry_.RegisterDynamicIPEventHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: ReceiveDynamicIPEvent" + data);
    auto status = this->network_config_brain_.ReceiveDynamicIPEvent(data);
    return status;
  });

  dbus_handler_registry_.RegisterReloadHostConfEventHandler([this]() {
    LOG_DEBUG("DBUS Req: RegisterReloadHostConfEventHandler");
    auto status = this->network_config_brain_.ReceiveReloadHostConfEvent();
    return status;
  });

  // interface
  dbus_handler_registry_.RegisterAddInterfaceHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: AddInterface: " + data);
    data = this->uri_escape_.Unescape(data);
    return this->network_config_brain_.AddInterface(data);
  });

  dbus_handler_registry_.RegisterDeleteInterfaceHandler([this](std::string data) {
    LOG_DEBUG("DBUS Req: DeleteInterface: " + data);
    data = this->uri_escape_.Unescape(data);
    return this->network_config_brain_.DeleteInterface(data);
  });

  LogInfo("NetworkConfigurator completed DBUS registration");
  start_condition.Notify();

  network_config_brain_.Start(startWithPortState);
  LogInfo("NetworkConfigurator ready");
}

NetworkConfigurator::NetworkConfigurator(InterprocessCondition &start_condition,
                                         StartWithPortstate startWithPortState) {
  network_configurator_ = ::std::make_unique<NetworkConfiguratorImpl>(start_condition, startWithPortState);
}

NetworkConfigurator::~NetworkConfigurator() {
  if (network_configurator_) {
    network_configurator_.reset();
  }
}

}  // namespace netconf
