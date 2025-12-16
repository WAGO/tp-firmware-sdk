// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include "switch_config_api.hpp"
#include "switchconfig_defaults.hpp"

namespace wago::libswitchconfig {


TEST(switch_config_defaults, get_default_ti_switch_config) {

  ::std::vector<system_port> system_ports {{"ethX1", "up"}, {"ethX2", "up"}};
  supported_values sv;
  switch_config sc;
  get_default_switch_config(sc, switch_type::TI, system_ports);

  EXPECT_EQ(sc.version, 2);
  EXPECT_EQ(sc.port_mirroring.enabled, false);
  EXPECT_EQ(sc.port_mirroring.source, "X1");
  EXPECT_EQ(sc.port_mirroring.destination, "X2");

  EXPECT_EQ(sc.broadcast_protection.value, 20000);
  EXPECT_EQ(sc.broadcast_protection.unit, "pps");
  EXPECT_EQ(sc.broadcast_protection.ports.size(), 2);
  EXPECT_EQ(sc.broadcast_protection.ports.at(0).name, "X1");
  EXPECT_EQ(sc.broadcast_protection.ports.at(0).enabled, false);
  EXPECT_EQ(sc.broadcast_protection.ports.at(1).name, "X2");
  EXPECT_EQ(sc.broadcast_protection.ports.at(1).enabled, false);

  EXPECT_EQ(sc.multicast_protection.value, 20000);
  EXPECT_EQ(sc.multicast_protection.unit, "pps");
  EXPECT_EQ(sc.multicast_protection.ports.size(), 2);
  EXPECT_EQ(sc.multicast_protection.ports.at(0).name, "X1");
  EXPECT_EQ(sc.multicast_protection.ports.at(0).enabled, false);
  EXPECT_EQ(sc.multicast_protection.ports.at(1).name, "X2");
  EXPECT_EQ(sc.multicast_protection.ports.at(1).enabled, false);

  EXPECT_EQ(sc.multicast_protection_enabled, false);

}

TEST(switch_config_defaults, get_default_marvell_switch_config) {

  ::std::vector<system_port> system_ports {{"ethX1", "up"}, {"ethX2", "up"}, {"ethX11", "up"}, {"ethX12", "up"}};
  supported_values sv;
  switch_config sc;
  get_default_switch_config(sc, switch_type::MARVELL, system_ports);

  EXPECT_EQ(sc.version, 2);
  EXPECT_EQ(sc.port_mirroring.enabled, false);
  EXPECT_EQ(sc.port_mirroring.source, "X1");
  EXPECT_EQ(sc.port_mirroring.destination, "X2");

  EXPECT_EQ(sc.broadcast_protection.value, 1);
  EXPECT_EQ(sc.broadcast_protection.unit, "Mbits");
  EXPECT_EQ(sc.broadcast_protection.ports.size(), 4);
  EXPECT_EQ(sc.broadcast_protection.ports.at(0).name, "X1");
  EXPECT_EQ(sc.broadcast_protection.ports.at(0).enabled, true);
  EXPECT_EQ(sc.broadcast_protection.ports.at(1).name, "X2");
  EXPECT_EQ(sc.broadcast_protection.ports.at(1).enabled, true);
  EXPECT_EQ(sc.broadcast_protection.ports.at(2).name, "X11");
  EXPECT_EQ(sc.broadcast_protection.ports.at(2).enabled, true);
  EXPECT_EQ(sc.broadcast_protection.ports.at(3).name, "X12");
  EXPECT_EQ(sc.broadcast_protection.ports.at(3).enabled, true);

  EXPECT_EQ(sc.multicast_protection.value, 1);
  EXPECT_EQ(sc.multicast_protection.unit, "Mbits");
  EXPECT_EQ(sc.multicast_protection.ports.size(), 4);
  EXPECT_EQ(sc.multicast_protection.ports.at(0).name, "X1");
  EXPECT_EQ(sc.multicast_protection.ports.at(0).enabled, false);
  EXPECT_EQ(sc.multicast_protection.ports.at(1).name, "X2");
  EXPECT_EQ(sc.multicast_protection.ports.at(1).enabled, false);
  EXPECT_EQ(sc.multicast_protection.ports.at(2).name, "X11");
  EXPECT_EQ(sc.multicast_protection.ports.at(2).enabled, false);
  EXPECT_EQ(sc.multicast_protection.ports.at(3).name, "X12");
  EXPECT_EQ(sc.multicast_protection.ports.at(3).enabled, false);

  EXPECT_EQ(sc.multicast_protection_enabled, false);

}

TEST(switch_config_defaults, get_default_micrel_switch_config) {

  ::std::vector<system_port> system_ports {{"ethX1", "up"}, {"ethX2", "up"}};
  supported_values sv;
  switch_config sc;
  get_default_switch_config(sc, switch_type::MICREL, system_ports);

  EXPECT_EQ(sc.version, 2);
  EXPECT_EQ(sc.port_mirroring.enabled, false);
  EXPECT_EQ(sc.port_mirroring.source, "X1");
  EXPECT_EQ(sc.port_mirroring.destination, "X2");

  EXPECT_EQ(sc.broadcast_protection.value, 1);
  EXPECT_EQ(sc.broadcast_protection.unit, "percent");
  EXPECT_EQ(sc.broadcast_protection.ports.size(), 2);
  EXPECT_EQ(sc.broadcast_protection.ports.at(0).name, "X1");
  EXPECT_EQ(sc.broadcast_protection.ports.at(0).enabled, true);
  EXPECT_EQ(sc.broadcast_protection.ports.at(1).name, "X2");
  EXPECT_EQ(sc.broadcast_protection.ports.at(1).enabled, true);

  EXPECT_EQ(sc.multicast_protection.value, 0);
  EXPECT_EQ(sc.multicast_protection.unit, "");
  EXPECT_EQ(sc.multicast_protection.ports.size(), 0);

  EXPECT_EQ(sc.multicast_protection_enabled, false);

}

}  // namespace wago::libswitchconfig