// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "devlink_micrel.hpp"

#include <string>

#include "program.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

::std::string build_devlink_set_cmd(::std::string parameter, ::std::string value) {
  return "/usr/sbin/devlink dev param set 'mdio_bus/gpio-0:00' name '" + parameter + "' cmode runtime value " + value;
}

status get_status_from_devtool_program(const program& programm) {
  return programm.get_result() == 0 ? status{} : status{status_code::SYSTEM_CALL_ERROR, programm.get_stderr()};
}

}  // namespace

status devlink_set_broadcast_rate_limit(::std::uint32_t rate) {
  auto cmd = build_devlink_set_cmd("bcast storm rate", ::std::to_string(rate));
  auto p   = program::execute(cmd);
  return get_status_from_devtool_program(p);
}

///usr/sbin/devlink dev param set 'mdio_bus/gpio-0:00' name 'mcast storm rate disable' cmode runtime value false
status devlink_set_multicast_rate_limit_enable(bool enable) {
  auto cmd = build_devlink_set_cmd("mcast storm rate disable", enable ? "false" : "true");
  auto p   = program::execute(cmd);
  return get_status_from_devtool_program(p);
}

status devlink_set_broadcast_storm_protection_enable(::std::uint32_t port_hw_id, bool enable) {
  ::std::string port = ::std::to_string(port_hw_id);
  auto cmd           = build_devlink_set_cmd("bcast storm protection enable port " + port, enable ? "true" : "false");
  auto p             = program::execute(cmd);
  return get_status_from_devtool_program(p);
}

status devlink_get_info(::std::string& info) {
  auto cmd = "/usr/sbin/devlink dev info";
  auto p   = program::execute(cmd);

  status s = get_status_from_devtool_program(p);
  if (s.ok()) {
    info = p.get_stdout();
  }
  return s;
}

}  // namespace wago::libswitchconfig
