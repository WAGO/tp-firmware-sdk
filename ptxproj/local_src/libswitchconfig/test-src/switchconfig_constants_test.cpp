// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include "switch_config_api.hpp"
#include "switchconfig_constants.hpp"

namespace wago::libswitchconfig {


TEST(switch_config_defaults, get_supported_values_ti) {

  supported_values sv = get_supported_values(switch_type::TI);

  EXPECT_EQ(sv.version, 2);
  EXPECT_EQ(sv.broadcast_protection_values.size(), 20);
  EXPECT_EQ(sv.broadcast_protection_values.at(0), 1000);
  EXPECT_EQ(sv.broadcast_protection_values.at(4), 5000);
  EXPECT_EQ(sv.broadcast_protection_values.at(9), 10000);
  EXPECT_EQ(sv.broadcast_protection_values.at(18), 100000);
  EXPECT_EQ(sv.broadcast_protection_values.at(19), 250000);

  EXPECT_EQ(sv.multicast_protection_values.size(), 20);
  EXPECT_EQ(sv.multicast_protection_values.at(0), 1000);
  EXPECT_EQ(sv.multicast_protection_values.at(4), 5000);
  EXPECT_EQ(sv.multicast_protection_values.at(9), 10000);
  EXPECT_EQ(sv.multicast_protection_values.at(18), 100000);
  EXPECT_EQ(sv.multicast_protection_values.at(19), 250000);
}

TEST(switch_config_defaults, get_supported_values_micrel) {

  supported_values sv = get_supported_values(switch_type::MICREL);

  EXPECT_EQ(sv.version, 2);
  EXPECT_EQ(sv.broadcast_protection_values.size(), 20);
  EXPECT_EQ(sv.broadcast_protection_values.at(0), 1);
  EXPECT_EQ(sv.broadcast_protection_values.at(4), 5);
  EXPECT_EQ(sv.broadcast_protection_values.at(9), 10);
  EXPECT_EQ(sv.broadcast_protection_values.at(19), 20);

  EXPECT_EQ(sv.multicast_protection_values.size(), 0);
}

TEST(switch_config_defaults, get_supported_values_marvell) {

  supported_values sv = get_supported_values(switch_type::MARVELL);

  EXPECT_EQ(sv.version, 2);
  EXPECT_EQ(sv.broadcast_protection_values.size(), 20);
  EXPECT_EQ(sv.broadcast_protection_values.at(0), 1);
  EXPECT_EQ(sv.broadcast_protection_values.at(4), 5);
  EXPECT_EQ(sv.broadcast_protection_values.at(8), 9);
  EXPECT_EQ(sv.broadcast_protection_values.at(9), 10);
  EXPECT_EQ(sv.broadcast_protection_values.at(19), 20);

  EXPECT_EQ(sv.multicast_protection_values.size(), 20);
  EXPECT_EQ(sv.multicast_protection_values.at(0), 1);
  EXPECT_EQ(sv.multicast_protection_values.at(4), 5);
  EXPECT_EQ(sv.multicast_protection_values.at(8), 9);
  EXPECT_EQ(sv.multicast_protection_values.at(9), 10);
  EXPECT_EQ(sv.multicast_protection_values.at(19), 20);
}

}  // namespace wago::libswitchconfig