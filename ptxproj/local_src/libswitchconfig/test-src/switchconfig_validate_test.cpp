// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

#include "switch_config_api.hpp"
#include "switchconfig_validate.hpp"

namespace wago::libswitchconfig {
namespace {

::std::vector<::std::string> two_port_device = {"X1", "X2"};

switch_config create_switch_config(const ::std::uint8_t version, const storm_protection& broadcast_protection,
                                   const storm_protection& multicast_protection, const port_mirror& port_mirroring) {
  switch_config config;

  config.version              = version;
  config.broadcast_protection = broadcast_protection;
  config.multicast_protection = multicast_protection;
  config.port_mirroring       = port_mirroring;

  return config;
}
}  // namespace

TEST(switch_config_validation, switch_config_is_equal) {
  switch_config config_left = create_switch_config(1,
    {{{true, "X1"}, {true, "X2"}}, "pps", 1000},
                           {{{false, "X1"}, {false, "X2"}}, "pps", 2000},
    {false, "X1", "X2"});

  switch_config config_right = create_switch_config(1,
    {{{true, "X1"}, {true, "X2"}}, "pps", 1000},
                           {{{false, "X1"}, {false, "X2"}}, "pps", 2000},
    {false, "X1", "X2"});

  EXPECT_THAT(config_left, testing::Eq(config_right));
}

TEST(switch_config_validation, do_deep_copy) {
  const switch_config config = create_switch_config(1,
    {{{true, "X1"}, {true, "X2"}}, "pps", 1000},
                           {{{false, "X1"}, {false, "X2"}}, "pps", 2000},
    {false, "X1", "X2"});

  auto config_copy = config;

  config_copy.broadcast_protection.value = 1234;
  config_copy.multicast_protection.ports.at(0).enabled = true;
  config_copy.port_mirroring.source = "X11";

  EXPECT_NE(config_copy, config);
}

TEST(switch_config_validation, port_mirroring_is_valid) {
  port_mirror pm = {false, "X1", "X2"};

  std::stringstream error_message;
  bool is_valid = validate(pm, two_port_device, error_message);

  EXPECT_TRUE(is_valid);
  EXPECT_THAT(error_message.str(), testing::IsEmpty());
}

TEST(switch_config_validation, port_mirroring_same_port) {
  port_mirror pm = {true, "X1", "X1"};

  std::stringstream error_message;
  bool is_valid = validate(pm, two_port_device, error_message);

  EXPECT_FALSE(is_valid);
  EXPECT_THAT(error_message.str(), "Port-Mirroring: Source and destination must not be the same.\n");
}

TEST(switch_config_validation, port_mirroring_invalid_port) {
  port_mirror pm = {true, "X1", "X3"};

  std::stringstream error_message;
  bool is_valid = validate(pm, two_port_device, error_message);

  EXPECT_FALSE(is_valid);
  EXPECT_THAT(error_message.str(), "Port-Mirroring: Invalid destination port name: X3\n");
}

TEST(switch_config_validation, storm_protection_invalid_port_name) {
  switch_config sc = create_switch_config(
      1, {{{true, "X1"}, {true, "X20"}}, "pps", 1000},
      {{{false, "X1"}, {false, "X2"}}, "pps", 2000},
      {false, "X1", "X2"});

  std::stringstream error_message;
  bool is_valid = validate(sc, two_port_device, switch_type::TI, error_message);

  EXPECT_FALSE(is_valid);
  EXPECT_THAT(error_message.str(), "Broadcast-protection: Invalid port name: X20\n");
}

TEST(switch_config_validation, storm_protection_invalid_value) {
  switch_config sc = create_switch_config(
      1, {{{true, "X1"}, {true, "X2"}}, "percent", 110},
      {{}, "", 0},
      {false, "X1", "X2"});

  std::stringstream error_message;
  bool is_valid = validate(sc, two_port_device, switch_type::MICREL, error_message);

  EXPECT_FALSE(is_valid);
  EXPECT_THAT(error_message.str(), "Broadcast-protection: Invalid value: 110. Supported values are: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 \n");
}

TEST(switch_config_validation, storm_protection_ignore_unset_config) {
  switch_config sc = create_switch_config(
      1, {{}, "", 0},
      {{}, "", 0},
      {false, "X1", "X2"});

  std::stringstream error_message;
  bool is_valid = validate(sc, two_port_device, switch_type::MICREL, error_message);

  EXPECT_TRUE(is_valid);
}

TEST(switch_config_validation, storm_protection_units) {
  switch_config sc = create_switch_config(
      1, {{{true, "X1"}, {true, "X2"}}, "MyOwnUnit", 1000},
      {{{false, "X1"}, {false, "X2"}}, "pps", 2000},
      {false, "X1", "X2"});

  std::stringstream error_message;
  bool is_valid = validate(sc, two_port_device, switch_type::TI, error_message);

  EXPECT_FALSE(is_valid);
  EXPECT_THAT(error_message.str(), "Broadcast-protection: Invalid unit. The unit pps will be supported.\n");
}

}  // namespace wago::libswitchconfig
