// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "iptool.hpp"

#include <iterator>
#include <nlohmann/json.hpp>
#include <string>

#include "program.hpp"
#include "switch_config_api.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

status get_config(nlohmann::json& ports) {
  auto cmd = "/usr/sbin/ip -json link show";
  auto p   = program::execute(cmd);
  if (p.get_result() == 0) {
    ports = nlohmann::json::parse(p.get_stdout());
    return {};
  }

  return status{status_code::SYSTEM_CALL_ERROR, p.get_stderr()};
}

}  // namespace

void from_json(const json& j, system_port& port) {
  j.at("ifname").get_to(port.name);
  j.at("operstate").get_to(port.state);
}

status get_system_ports_from_json(::std::vector<system_port>& ports, const json& j) {
  status s{};

  try {
    ports = j.template get<::std::vector<system_port>>();
  } catch (const ::std::exception& e) {
    s = status{status_code::JSON_PARSE_ERROR, "Json parse error: " + ::std::string(e.what())};
  }

  return s;
}

status get_system_ports(::std::vector<system_port>& ports) {
  status s{};
  nlohmann::json ports_json;
  ::std::vector<system_port> all_ports;

  s = get_config(ports_json);

  if (s.ok()) {
    s = get_system_ports_from_json(all_ports, ports_json);
  }

  if (s.ok()) {
    ::std::copy_if(all_ports.begin(), all_ports.end(), std::back_inserter(ports),
        [](const auto& p){return p.name.rfind("ethX", 0) == 0;}
    );
  }

  return s;
}

}  // namespace wago::libswitchconfig