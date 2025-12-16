// Copyright (c) 2025 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "restore.hpp"

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "switch_config_api.hpp"
#include "switch_data_provider_i.hpp"
#include "switch_data_provider_mock.hpp"
#include "test_helper.hpp"

namespace wago::config_switch {

using namespace wago::libswitchconfig;  // NOLINT google-build-using-namespace
using namespace ::testing;              // NOLINT google-build-using-namespace

class RestoreTest : public ::testing::Test {
 protected:
  switch_data_provider_mock data_provider;

 public:
  status get_default_switch_config_micrel(switch_config& config) {
    config.version                      = 2;
    config.broadcast_protection         = {{{true, "X1"}, {true, "X2"}}, "percent", 1};
    config.multicast_protection         = {{{false, "X1"}, {false, "X2"}}, "percent", 1};
    config.port_mirroring               = {false, "X1", "X2"};
    config.multicast_protection_enabled = false;
    return status{};
  }
};

TEST_F(RestoreTest, get_switch_config_from_v2_backup_file_content) {
  const ::std::string backup_content_v2 =
      R"(switch.data={"broadcast-protection":{"ports":[{"enabled":true,"name":"X1"},{"enabled":fa
switch.data=lse,"name":"X2"}],"unit":"pps","value":8000},"multicast-protection":{"ports"
switch.data=:[{"enabled":false,"name":"X1"},{"enabled":true,"name":"X2"}],"unit":"pps","
switch.data=value":2000},"multicast-protection-enabled":true,"port-mirror":{"destination
switch.data=":"X2","enabled":false,"source":"X1"},"version":2})";

  wago::libswitchconfig::switch_config config;
  auto status = get_switch_config_from_backup(data_provider, config, backup_content_v2);

  ASSERT_TRUE(status.ok()) << status.to_string();

  storm_protection bp = {{{true, "X1"}, {false, "X2"}}, "pps", 8000};
  storm_protection mp = {{{false, "X1"}, {true, "X2"}}, "pps", 2000};
  port_mirror pm      = {false, "X1", "X2"};

  EXPECT_THAT(config.broadcast_protection, bp);
  EXPECT_THAT(config.multicast_protection, mp);
  EXPECT_THAT(config.port_mirroring, pm);
  EXPECT_THAT(config.multicast_protection_enabled, true);
  EXPECT_THAT(config.version, 2);
}

TEST_F(RestoreTest, get_switch_config_from_v1_backup_file_content) {
  const ::std::string backup_content_v1 =
      R"(switch.data={"broadcast-protection":{"ports":[{"enabled":true,"name":"X1"},{"enabled":fa
switch.data=lse,"name":"X2"}],"unit":"pps","value":8000},"multicast-protection":{"ports"
switch.data=:[{"enabled":false,"name":"X1"},{"enabled":true,"name":"X2"}],"unit":"pps","
switch.data=value":2000},"port-mirror":{"destination":"X2","enabled":false,"source":"X1"
switch.data=},"version":1})";

  wago::libswitchconfig::switch_config config;
  auto status = get_switch_config_from_backup(data_provider, config, backup_content_v1);

  ASSERT_TRUE(status.ok()) << status.to_string();

  storm_protection bp = {{{true, "X1"}, {false, "X2"}}, "pps", 8000};
  storm_protection mp = {{{false, "X1"}, {true, "X2"}}, "pps", 2000};
  port_mirror pm      = {false, "X1", "X2"};

  EXPECT_THAT(config.broadcast_protection, bp);
  EXPECT_THAT(config.multicast_protection, mp);
  EXPECT_THAT(config.port_mirroring, pm);
  EXPECT_THAT(config.multicast_protection_enabled, false);
  EXPECT_THAT(config.version, 1);
}

TEST_F(RestoreTest, get_switch_config_from_v0_backup_file_content_for_micrel) {
  EXPECT_CALL(data_provider, get_default_switch_config(::testing::_))
      .WillOnce(::testing::Invoke(this, &RestoreTest::get_default_switch_config_micrel));

  EXPECT_CALL(data_provider, get_switch_type(::testing::_)).WillOnce(Invoke([](switch_type& st) {
    st = switch_type::MICREL;
    return status{};
  }));

  const ::std::string backup_content_v0 =
      R"(switch-port-mirror=1
switch-bcast-protect=3
switch-rate-limit=disabled)";

  wago::libswitchconfig::switch_config config;
  auto status = get_switch_config_from_backup(data_provider, config, backup_content_v0);

  ASSERT_TRUE(status.ok()) << status.to_string();

  storm_protection bp = {{{true, "X1"}, {true, "X2"}}, "percent", 3};
  storm_protection mp = {{{false, "X1"}, {false, "X2"}}, "percent", 1};  // Should be the value provided by default config.
  port_mirror pm      = {true, "X1", "X2"};

  EXPECT_THAT(config.broadcast_protection, bp);
  EXPECT_THAT(config.multicast_protection, mp);
  EXPECT_THAT(config.port_mirroring, pm);
  EXPECT_THAT(config.multicast_protection_enabled, false);  // Default value for v0
  EXPECT_THAT(config.version, 2);
}

TEST_F(RestoreTest, try_to_get_switch_config_from_v0_backup_file_content_for_non_micrel) {
  EXPECT_CALL(data_provider, get_switch_type(::testing::_)).WillOnce(Invoke([](switch_type& st) {
    st = switch_type::TI;
    return status{};
  }));

  const ::std::string backup_content_v0 =
      R"(switch-port-mirror=1
switch-bcast-protect=3
switch-rate-limit=disabled)";

  wago::libswitchconfig::switch_config config;
  auto status = get_switch_config_from_backup(data_provider, config, backup_content_v0);

  ASSERT_FALSE(status.ok()) << status.to_string();
  ASSERT_THAT(status.to_string(), HasSubstr("No valid switch data found in backup"));
}

TEST_F(RestoreTest, try_to_get_switch_data_from_incomplete_v0_backup) {
  EXPECT_CALL(data_provider, get_default_switch_config(::testing::_))
      .WillRepeatedly(::testing::Invoke(this, &RestoreTest::get_default_switch_config_micrel));

  const ::std::string backup_content_v0_missing_port_mirror =
      R"(switch-bcast-protect=3
  switch-rate-limit=disabled)";

  wago::libswitchconfig::switch_config config;
  auto status = get_switch_data_v0_from_backup(data_provider, config, backup_content_v0_missing_port_mirror);

  ASSERT_FALSE(status.ok()) << status.to_string();
  ASSERT_THAT(status.to_string(), HasSubstr("Missing required parameters in v0 backup content"));

  const ::std::string backup_content_v0_missing_port_mirror_value =
      R"(switch-port-mirror=
  switch-bcast-protect=3
  switch-rate-limit=disabled)";

  status = get_switch_data_v0_from_backup(data_provider, config, backup_content_v0_missing_port_mirror_value);

  ASSERT_FALSE(status.ok()) << status.to_string();
  ASSERT_THAT(status.to_string(), HasSubstr("Missing required parameters in v0 backup content"));
}

}  // namespace wago::config_switch
