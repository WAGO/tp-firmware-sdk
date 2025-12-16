// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "switchconfig_base.hpp"

namespace wago::libswitchconfig {
namespace {

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

TEST(port_system_name_conversion, convert_port_names) {
  switch_config config = create_switch_config(1, {{{true, "X1"}, {true, "X2"}}, "pps", 1000},
                                              {{{true, "X1"}}, "pps", 4321}, {false, "X1", "X2"});

  switch_config expected_config = create_switch_config(1, {{{true, "ethX1"}, {true, "ethX2"}}, "pps", 1000},
                                                       {{{true, "ethX1"}}, "pps", 4321}, {false, "ethX1", "ethX2"});

  convert_to_system_port_names(config);

  EXPECT_THAT(config, expected_config);
}

TEST(port_system_name_conversion, convert_port_names_ignore_eth_prefix) {
  switch_config config = create_switch_config(1, {{{true, "X1"}, {true, "ethX2"}}, "pps", 1000},
                                              {{{true, "ethX1"}}, "pps", 4321}, {false, "X1", "ethX2"});

  switch_config expected_config = create_switch_config(1, {{{true, "ethX1"}, {true, "ethX2"}}, "pps", 1000},
                                                       {{{true, "ethX1"}}, "pps", 4321}, {false, "ethX1", "ethX2"});

  convert_to_system_port_names(config);

  EXPECT_THAT(config, expected_config);
}

}  // namespace wago::libswitchconfig
