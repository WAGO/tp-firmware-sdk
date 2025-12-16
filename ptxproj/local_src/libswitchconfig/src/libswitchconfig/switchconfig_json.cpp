
// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <exception>
#include <nlohmann/json.hpp>
#include <string>

#include "gtest/gtest.h"
#include "libswitchconfig/switchconfig_constants.hpp"
#include "switch_config_api.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

void to_json(json& j, const port& p);
void to_json(json& j, const port& p) {
  j = json{{"enabled", p.enabled}, {"name", p.name}};
}

void to_json(json& j, const storm_protection& protection);
void to_json(json& j, const storm_protection& protection) {
  j = json{{"ports", protection.ports}, {"unit", protection.unit}, {"value", protection.value}};
}

void to_json(json& j, const port_mirror& mirror);
void to_json(json& j, const port_mirror& mirror) {
  j = json{{"enabled", mirror.enabled}, {"source", mirror.source}, {"destination", mirror.destination}};
}

void to_json(json& j, const switch_config& config);
void to_json(json& j, const switch_config& config) {
  if (config.version == switch_config_version1) {
    j = json{{"version", config.version},
             {"broadcast-protection", config.broadcast_protection},
             {"multicast-protection", config.multicast_protection},
             {"port-mirror", config.port_mirroring}};
  } else if (config.version == switch_config_version) {
    j = json{{"version", config.version},
             {"broadcast-protection", config.broadcast_protection},
             {"multicast-protection", config.multicast_protection},
             {"port-mirror", config.port_mirroring},
             {"multicast-protection-enabled", config.multicast_protection_enabled}};
  } else {
    throw std::runtime_error("Unknown config version. only versions <= " + ::std::to_string(switch_config_version) +
                             " are supported!");
  }
}

void from_json(const json& j, port& p);
void from_json(const json& j, port& p) {
  j.at("enabled").get_to(p.enabled);
  j.at("name").get_to(p.name);
}

void from_json(const json& j, storm_protection& protection);
void from_json(const json& j, storm_protection& protection) {
  j.at("ports").get_to(protection.ports);
  j.at("unit").get_to(protection.unit);
  j.at("value").get_to(protection.value);
}

void from_json(const json& j, port_mirror& mirror);
void from_json(const json& j, port_mirror& mirror) {
  j.at("enabled").get_to(mirror.enabled);
  j.at("source").get_to(mirror.source);
  j.at("destination").get_to(mirror.destination);
}

void from_json(const json& j, switch_config& config);
void from_json(const json& j, switch_config& config) {
  j.at("version").get_to(config.version);
  j.at("broadcast-protection").get_to(config.broadcast_protection);
  j.at("multicast-protection").get_to(config.multicast_protection);
  j.at("port-mirror").get_to(config.port_mirroring);

  // Parameter 'multicast-protection-enabled' added in version 2.
  // Optional for update compatibility reasons.
  auto mc = j.find("multicast-protection-enabled") != j.end();
  if (mc) {
    j.at("multicast-protection-enabled").get_to(config.multicast_protection_enabled);
  }
}

status switch_config_to_json(json& j, const switch_config& config) {
  status s{};
  try {
    j = config;
  } catch (const ::std::exception& e) {
    s = status{status_code::JSON_PARSE_ERROR, "Json convert error: " + ::std::string(e.what())};
  }
  return s;
}

status switch_config_from_json(switch_config& config, const json& j) {
  status s{};

  try {
    auto tmp = j.template get<switch_config>();

    if (tmp.version <= switch_config_version) {
      config = tmp;
    } else {
      s = status{status_code::UNKNOWN_CONFIG_VERSION, "Unknown config version. only versions <= " +
                                                          ::std::to_string(switch_config_version) + " are supported!"};
    }
  } catch (const ::std::exception& e) {
    s = status{status_code::JSON_PARSE_ERROR, "Json parse error: " + ::std::string(e.what())};
  }

  return s;
}
}  // namespace wago::libswitchconfig