// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switch_config_handler.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

#include "switch_config_api.hpp"

namespace wago::config_switch {

using wago::libswitchconfig::switch_config;

namespace {

constexpr auto version              = "version";
constexpr auto broadcast_protection = "broadcast-protection";
constexpr auto multicast_protection = "multicast-protection";
constexpr auto multicast_protection_enabled = "multicast-protection-enabled";
constexpr auto port_mirror          = "port-mirror";

constexpr auto sp_ports = "ports";
constexpr auto sp_unit  = "unit";
constexpr auto sp_value = "value";

constexpr auto pm_enabled     = "enabled";
constexpr auto pm_source      = "source";
constexpr auto pm_destination = "destination";

constexpr auto p_name    = "name";
constexpr auto p_enabled = "enabled";

constexpr auto all_parameters = {"set", "restore"};

template <typename T>
using setter_func_map = ::std::map<::std::string, ::std::function<void(T&, const nlohmann::basic_json<>&)>>;

template <typename T>
void read_parameters_to(nlohmann::json const& parameters, setter_func_map<T> const& setter, T& to) {
  for (auto const& p : parameters.items()) {
    if (setter.count(p.key()) > 0) {
      setter.at(p.key())(to, p.value());
    } else {
      throw switch_config_exception(switch_config_error::invalid_parameter, "unknown json field: " + p.key());
    }
  }
}

void check_version_is_supported(const switch_config& config) {
  if (config.version > 2) {
    throw switch_config_exception(switch_config_error::invalid_parameter,
                                  "invalid version: " + ::std::to_string(static_cast<unsigned int>(config.version)));
  }
}

template <typename T>
T get_checked(const nlohmann::basic_json<>& value, const ::std::string& param_name) {
  int64_t tmp;
  value.get_to(tmp);
  if (tmp < 0 || tmp > ::std::numeric_limits<T>::max()) {
    throw switch_config_exception(switch_config_error::invalid_parameter,
                                  "The value of" + param_name + " must be in the range from 0 to 255");
  }

  return static_cast<T>(tmp);
}

void edit_port_parameters(libswitchconfig::port& p, const nlohmann::basic_json<>& json) {
  // clang-format off
  const setter_func_map<libswitchconfig::port> setter_funcs = {
      {p_enabled, [](libswitchconfig::port& pm, const nlohmann::basic_json<>& value) { value.get_to(pm.enabled); }},
      {p_name,    [](libswitchconfig::port& pm, const nlohmann::basic_json<>& value) { value.get_to(pm.name); }}};
  // clang-format on

  read_parameters_to(json, setter_funcs, p);
}

void read_port_parameters_to(libswitchconfig::storm_protection& sp, const nlohmann::basic_json<>& j_ports) {
  if (!j_ports.is_array()) {
    throw switch_config_exception(switch_config_error::invalid_parameter, "storm-protection must be an array");
  }

  for (auto const& j_port : j_ports) {
    auto port_it = j_port.find(p_name);
    if (port_it == j_port.end()) {
      throw switch_config_exception(switch_config_error::invalid_parameter, "port must contain a name");
    }
    auto port_name = *port_it;
    auto it        = std::find_if(sp.ports.begin(), sp.ports.end(), [&](const auto& p) { return p.name == port_name; });

    if (it != sp.ports.end()) {
      edit_port_parameters(*it, j_port);
    } else {
      auto new_port = libswitchconfig::port{};
      edit_port_parameters(new_port, j_port);
      sp.ports.push_back(new_port);
    }
  }
}

void edit_storm_protection_parameters(libswitchconfig::storm_protection& sp, const nlohmann::basic_json<>& json) {
  // clang-format off
  const setter_func_map<libswitchconfig::storm_protection> setter_funcs = {
    {sp_ports, [](libswitchconfig::storm_protection& storm_p, const nlohmann::basic_json<>& value) { read_port_parameters_to(storm_p, value); }},
    {sp_unit,  [](libswitchconfig::storm_protection& storm_p, const nlohmann::basic_json<>& value) { value.get_to(storm_p.unit); }},
    {sp_value, [](libswitchconfig::storm_protection& storm_p, const nlohmann::basic_json<>& value) { value.get_to(storm_p.value); }}};
  // clang-format on

  read_parameters_to(json, setter_funcs, sp);
}

void edit_port_mirror_parameters(switch_config& config, const nlohmann::basic_json<>& json) {
  // clang-format off
  const setter_func_map<libswitchconfig::port_mirror> setter_funcs = {
      {pm_enabled,     [](libswitchconfig::port_mirror& pm, const nlohmann::basic_json<>& value) { value.get_to(pm.enabled); }},
      {pm_source,      [](libswitchconfig::port_mirror& pm, const nlohmann::basic_json<>& value) { value.get_to(pm.source); }},
      {pm_destination, [](libswitchconfig::port_mirror& pm, const nlohmann::basic_json<>& value) { value.get_to(pm.destination); }}};
  // clang-format on

  read_parameters_to(json, setter_funcs, config.port_mirroring);
}

}  // namespace

void check_that_parameters_are_known(parameter_map const& parameters) {
  static const auto known_parameters = ::std::set<::std::string>{all_parameters.begin(), all_parameters.end()};
  for (auto& parameter : parameters) {
    if (known_parameters.count(parameter.first) == 0) {
      throw switch_config_exception(switch_config_error::invalid_parameter,
                                    "invalid command line parameter: " + parameter.first);
    }
  }
}

void edit_parameters(const ::std::string& json_string, switch_config& config) {
  // clang-format off
  const setter_func_map<switch_config> setter_funcs = {
      {version,         [](switch_config& c, const nlohmann::basic_json<>& value) { c.version = get_checked<uint8_t>(value, version); check_version_is_supported(c); }},
      {broadcast_protection,     [](switch_config& c, const nlohmann::basic_json<>& value) {edit_storm_protection_parameters(c.broadcast_protection, value); }},
      {multicast_protection,     [](switch_config& c, const nlohmann::basic_json<>& value) {edit_storm_protection_parameters(c.multicast_protection, value); }},
      {multicast_protection_enabled, [](switch_config& c, const nlohmann::basic_json<>& value) { value.get_to(c.multicast_protection_enabled); }},
      {port_mirror,              edit_port_mirror_parameters}
  };
  // clang-format on

  try {
    nlohmann::json j  = nlohmann::json::parse(json_string);
    switch_config tmp = config;
    read_parameters_to(j, setter_funcs, tmp);
    config = tmp;
  } catch (nlohmann::json::exception const& e) {
    throw switch_config_exception(switch_config_error::invalid_parameter, e.what());
  } catch (switch_config_exception const& e) {
    throw e;
  }
}

}  // namespace wago::config_switch
