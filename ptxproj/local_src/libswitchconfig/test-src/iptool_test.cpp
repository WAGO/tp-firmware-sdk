// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <vector>

#include "iptool.hpp"
#include "switch_config_api.hpp"

namespace {
constexpr auto ethX1_json_str =
    R"([{"ifindex":3,"ifname":"ethX1","flags":["BROADCAST","MULTICAST"],"mtu":1500,"qdisc":"mq","master":"br0","operstate":"DOWN","linkmode":"DEFAULT","group":"default","txqlen":1000,"link_type":"ether","address":"00:30:de:60:14:ff","broadcast":"ff:ff:ff:ff:ff:ff","permaddr":"0a:1c:b0:82:d7:26"}])";

}  // namespace

namespace wago::libswitchconfig {

TEST(ip_tool_json_parse_test, convert_ip_tool_from_json) {
  nlohmann::json j = nlohmann::json::parse(ethX1_json_str);
  ::std::vector<system_port> configs;
  auto status = get_system_ports_from_json(configs, j);

  ASSERT_TRUE(status.ok());
  ASSERT_THAT(configs.at(0).name, testing::Eq("ethX1"));
  ASSERT_THAT(configs.at(0).state, testing::Eq("DOWN"));
}

}  // namespace wago::libswitchconfig