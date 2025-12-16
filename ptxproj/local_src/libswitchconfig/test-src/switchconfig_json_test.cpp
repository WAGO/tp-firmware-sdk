// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <nlohmann/json.hpp>

#include "switch_config_api.hpp"
#include "switchconfig_constants.hpp"

namespace wago::libswitchconfig {
namespace {

switch_config create_switch_config(const ::std::uint8_t version, const storm_protection& broadcast_protection,
                                   const storm_protection& multicast_protection, const port_mirror& port_mirroring,
                                   bool multicast_protection_enabled) {
  switch_config config;

  config.version                      = version;
  config.broadcast_protection         = broadcast_protection;
  config.multicast_protection         = multicast_protection;
  config.port_mirroring               = port_mirroring;
  config.multicast_protection_enabled = multicast_protection_enabled;

  return config;
}

}  // namespace

nlohmann::json expected_json = nlohmann::json::parse(R"(
{
"broadcast-protection": {
    "ports": [
        {
            "enabled": true,
            "name": "X1"
        },
        {
            "enabled": false,
            "name": "X2"
        }
    ],
    "unit": "pps",
    "value": 1500
},
"multicast-protection": {
    "ports": [
        {
            "enabled": false,
            "name": "X1"
        },
        {
            "enabled": true,
            "name": "X2"
        }
    ],
    "unit": "pps",
    "value": 1000
},
"port-mirror": {
    "destination": "X2",
    "enabled": false,
    "source": "X1"
},
"version": 2,
"multicast-protection-enabled": true
})");

TEST(switch_config_json_test, convert_switch_config_to_json) {
  storm_protection bcast            = {{{true, "X1"}, {false, "X2"}}, "pps", 1500};
  storm_protection mcast            = {{{false, "X1"}, {true, "X2"}}, "pps", 1000};
  port_mirror port_mirror           = {false, "X1", "X2"};
  bool multicast_protection_enabled = true;

  switch_config swconfig =
      create_switch_config(switch_config_version, bcast, mcast, port_mirror, multicast_protection_enabled);

  nlohmann::json j{};
  auto status = switch_config_to_json(j, swconfig);

  ASSERT_TRUE(status.ok());
  ASSERT_THAT(j.dump(), expected_json.dump());
}

TEST(switch_config_json_test, convert_json_to_switch_config) {
  storm_protection bcast            = {{{true, "X1"}, {false, "X2"}}, "pps", 1500};
  storm_protection mcast            = {{{false, "X1"}, {true, "X2"}}, "pps", 1000};
  port_mirror port_mirror           = {false, "X1", "X2"};
  bool multicast_protection_enabled = true;
  switch_config expected_swconfig =
      create_switch_config(switch_config_version, {bcast}, {mcast}, port_mirror, multicast_protection_enabled);

  switch_config swconfig;
  auto status = switch_config_from_json(swconfig, expected_json);

  ASSERT_TRUE(status.ok());
  ASSERT_THAT(swconfig, expected_swconfig);
}

TEST(switch_config_json_test, try_to_convert_switch_config_of_unknown_version) {
  storm_protection bcast            = {{{true, "X1"}, {false, "X2"}}, "pps", 1500};
  storm_protection mcast            = {{{false, "X1"}, {true, "X2"}}, "pps", 1000};
  port_mirror port_mirror           = {false, "X1", "X2"};
  bool multicast_protection_enabled = true;

  switch_config swconfig = create_switch_config(10, bcast, mcast, port_mirror, multicast_protection_enabled);

  nlohmann::json j{};
  auto status = switch_config_to_json(j, swconfig);

  ASSERT_THAT(status.get_code(), status_code::JSON_PARSE_ERROR);
}

TEST(switch_config_json_test, try_to_convert_incomplete_json) {
  nlohmann::json invalid_config = nlohmann::json::parse(
      R"(
{
    "broadcast-protection": [
        {
            "port": "X1"
        }
    ],
    "multicast-protection": [
        {
            "port": "X1",
            "value": "1000"
        }
    ],
    "port-mirror": {
        "destination": "X2",
        "enabled": false,
        "source": "X1"
    },
    "version": 2
}
)");

  switch_config swconfig;
  auto status = switch_config_from_json(swconfig, invalid_config);

  ASSERT_THAT(status.get_code(), status_code::JSON_PARSE_ERROR);
}

TEST(switch_config_json_test, try_to_convert_json_with_unknown_version) {
  switch_config swconfig;
  nlohmann::json unknown_version_json_str = nlohmann::json::parse(
      R"(
      {
    "broadcast-protection": {
        "ports": [],
        "unit": "",
        "value": 1000
    },
    "multicast-protection": {
        "ports": [],
        "unit": "",
        "value": 2000
    },
    "port-mirror": {
        "destination": "X2",
        "enabled": false,
        "source": "X1"
    },
    "multicast-protection-enabled": true,
    "version": 11
  })");
  auto status = switch_config_from_json(swconfig, unknown_version_json_str);

  ASSERT_THAT(status.get_code(), status_code::UNKNOWN_CONFIG_VERSION);
}

TEST(switch_config_json_test, convert_json_v1_to_switch_config) {
  switch_config swconfig;
  nlohmann::json config = nlohmann::json::parse(
      R"(
      {
    "broadcast-protection": {
        "ports": [],
        "unit": "",
        "value": 1000
    },
    "multicast-protection": {
        "ports": [],
        "unit": "",
        "value": 2000
    },
    "port-mirror": {
        "destination": "X2",
        "enabled": false,
        "source": "X1"
    },
    "version": 1
  })");
  auto status = switch_config_from_json(swconfig, config);

  ASSERT_THAT(status.get_code(), status_code::OK);
}

TEST(switch_config_json_test, convert_switch_config_v1_to_json) {
  switch_config swconfig;
  swconfig.version              = switch_config_version1;
  swconfig.broadcast_protection = storm_protection{{{true, "X1"}, {false, "X2"}}, "pps", 1500};
  swconfig.multicast_protection = storm_protection{{{false, "X1"}, {true, "X2"}}, "pps", 1000};
  swconfig.port_mirroring       = port_mirror{false, "X1", "X2"};

  nlohmann::json j{};
  auto status = switch_config_to_json(j, swconfig);

  nlohmann::json expected_json_v1 = nlohmann::json::parse(R"(
{
"broadcast-protection": {
    "ports": [
        {
            "enabled": true,
            "name": "X1"
        },
        {
            "enabled": false,
            "name": "X2"
        }
    ],
    "unit": "pps",
    "value": 1500
},
"multicast-protection": {
    "ports": [
        {
            "enabled": false,
            "name": "X1"
        },
        {
            "enabled": true,
            "name": "X2"
        }
    ],
    "unit": "pps",
    "value": 1000
},
"port-mirror": {
    "destination": "X2",
    "enabled": false,
    "source": "X1"
},
"version": 1
})");

  ASSERT_TRUE(status.ok());
  ASSERT_THAT(j.dump(), expected_json_v1.dump());
}
}  // namespace wago::libswitchconfig