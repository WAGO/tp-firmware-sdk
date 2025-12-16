// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-actions.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "switch_config_api.hpp"
#include "switch_config_output.hpp"
#include "switch_config_v0.hpp"
#include "switch_data_provider_mock.hpp"

namespace {
using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::wago::get_switch_config::option_map;
using ::wago::get_switch_config::switch_data_provider_mock;
using ::wago::libswitchconfig::status;
using ::wago::libswitchconfig::switch_config;
using ::wago::libswitchconfig::switch_type;

constexpr auto switch_config_v1_json_str =
    R"({"broadcast-protection":{"ports":[{"enabled":true,"name":"X1"}],"unit":"pps","value":1000},"multicast-protection":{"ports":[{"enabled":true,"name":"X2"}],"unit":"pps","value":2000},"port-mirror":{"destination":"X2","enabled":false,"source":"X1"},"version":1})";
constexpr auto switch_config_v2_json_str =
    R"({"broadcast-protection":{"ports":[{"enabled":true,"name":"X1"}],"unit":"pps","value":1000},"multicast-protection":{"ports":[{"enabled":true,"name":"X2"}],"unit":"pps","value":2000},"multicast-protection-enabled":true,"port-mirror":{"destination":"X2","enabled":false,"source":"X1"},"version":2})";
::std::vector<::std::string> split_lines(const ::std::string& data) {
  ::std::string line;
  ::std::vector<::std::string> lines;

  ::std::stringstream ss(data);
  while (getline(ss, line, '\n')) {
    lines.push_back(line);
  }

  return lines;
}

MATCHER_P(StrLengthIsLessOrEqualTo, max_length, "") {  // NOLINT clang-diagnostic-deprecated
  return arg.length() <= static_cast<size_t>(max_length);
}

::std::string read_backup_content(const switch_config& config, const option_map& options, const switch_type type) {
  switch_data_provider_mock provider_mock;

  // clang-format off
  EXPECT_CALL(provider_mock, get_switch_config(_))
    .WillOnce(DoAll(
      SetArgReferee<0>(config),
      Return(status{})
    ));

  EXPECT_CALL(provider_mock, get_switch_type(_))
    .WillOnce(DoAll(
      SetArgReferee<0>(type),
      Return(status{})
    ));
  // clang-format on

  testing::internal::CaptureStdout();
  print_backup_content(options, provider_mock);
  return testing::internal::GetCapturedStdout();
}

void expect_backup_content_is_printed(
    const wago::libswitchconfig::switch_config& config, const ::nlohmann::json& expected_json,
    const wago::get_switch_config::option_map& options = {},
    const wago::libswitchconfig::switch_type type      = wago::libswitchconfig::switch_type::UNKNOWN) {
  auto output = read_backup_content(config, options, type);

  auto lines = split_lines(output);
  EXPECT_THAT(lines, testing::Each(testing::StartsWith("switch.data=")));
  EXPECT_THAT(lines, testing::Each(StrLengthIsLessOrEqualTo(88)));

  ::boost::replace_all(output, "switch.data=", "");
  ::boost::replace_all(output, "\n", "");
  EXPECT_EQ(::nlohmann::json::parse(output), expected_json);
}

void expect_backup_content_v0_is_printed(
    const wago::libswitchconfig::switch_config& config, const switch_config_v0& original_config,
    const wago::get_switch_config::option_map& options = {},
    const wago::libswitchconfig::switch_type type      = wago::libswitchconfig::switch_type::UNKNOWN) {
  auto output = read_backup_content(config, options, type);
  auto lines  = split_lines(output);

  EXPECT_THAT(lines, testing::Contains(
                         testing::StartsWith("switch-port-mirror=" + std::to_string(original_config.port_mirror))));
  EXPECT_THAT(lines, testing::Contains(testing::StartsWith("switch-bcast-protect=" +
                                                           std::to_string(original_config.broadcast_protection))));
  EXPECT_THAT(lines, testing::Contains(testing::StartsWith("switch-rate-limit=disabled")));
}

}  // namespace

namespace nlohmann {
void PrintTo(const ::nlohmann::json& j, std::ostream* os);
void PrintTo(const ::nlohmann::json& j, std::ostream* os) {
  *os << j.dump();
}
}  // namespace nlohmann

namespace wago::get_switch_config {

using namespace wago::libswitchconfig;  // NOLINT google-build-using-namespace
using namespace ::testing;              // NOLINT google-build-using-namespace

TEST(config_to_json, print_json_config_v1) {
  switch_data_provider_mock provider_mock;

  auto expected_json = ::nlohmann::json::parse(switch_config_v1_json_str);

  // clang-format off
  switch_config config = {1, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}};

  EXPECT_CALL(provider_mock, get_switch_config(_))
  .WillOnce(DoAll(
    SetArgReferee<0>(config),
    Return(status{})
  ));
  // clang-format on

  testing::internal::CaptureStdout();
  print_json_config(option_map{}, provider_mock);
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_EQ(::nlohmann::json::parse(output), expected_json);
}

TEST(config_to_json, print_json_config_v2) {
  switch_data_provider_mock provider_mock;

  auto expected_json = ::nlohmann::json::parse(switch_config_v2_json_str);

  // clang-format off
  switch_config config = {2, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}, true};

  EXPECT_CALL(provider_mock, get_switch_config(_))
  .WillOnce(DoAll(
    SetArgReferee<0>(config),
    Return(status{})
  ));
  // clang-format on

  testing::internal::CaptureStdout();
  print_json_config(option_map{}, provider_mock);
  std::string output = testing::internal::GetCapturedStdout();

  EXPECT_EQ(::nlohmann::json::parse(output), expected_json);
}

TEST(config_to_json, try_to_print_config_for_unknown_version) {
  ::std::uint8_t version = 5;
  switch_config config = {
      version, {{{true, "X1"}, {true, "X2"}}, "percent", 3}, {{{true, "X2"}}, "percent", 1}, {true, "X2", "X1"}, true};

  // clang-format off
  EXPECT_DEATH({
    switch_data_provider_mock provider_mock;
    ::testing::Mock::AllowLeak(&provider_mock);
    EXPECT_CALL(provider_mock, get_switch_config(_))
      .WillOnce(DoAll(
        SetArgReferee<0>(config),
        Return(status{})
      ));
    print_json_config(option_map{}, provider_mock);
  }, ".*Unknown config version.*");
  // clang-format on
}

TEST(config_to_json, format_json_for_backup) {
  auto expected_json   = ::nlohmann::json::parse(switch_config_v2_json_str);
  switch_config config = {2, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}, true};

  expect_backup_content_is_printed(config, expected_json);
}

TEST(config_to_json, format_json_for_backup_v1_with_targetversion) {
  switch_data_provider_mock provider_mock;

  auto expected_json = ::nlohmann::json::parse(switch_config_v1_json_str);

  option_map options   = {{action_option::targetversion, "V04.04.12"}};
  switch_config config = {1, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}};

  expect_backup_content_is_printed(config, expected_json, options, wago::libswitchconfig::switch_type::TI);
}

TEST(config_to_json, format_json_for_backup_of_ti_from_v1_to_v2) {
  switch_data_provider_mock provider_mock;

  auto expected_json = ::nlohmann::json::parse(switch_config_v2_json_str);
  // This has to be added for v2 format, but is ignored by ti config.
  expected_json["multicast-protection-enabled"] = false;

  switch_config config = {1, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}};

  expect_backup_content_is_printed(config, expected_json, {}, wago::libswitchconfig::switch_type::TI);
}

TEST(config_to_json, format_json_for_backup_of_ti_from_v2_to_v1) {
  switch_data_provider_mock provider_mock;

  auto expected_json = ::nlohmann::json::parse(switch_config_v1_json_str);

  option_map options   = {{action_option::targetversion, "V04.07.00"}};  // v1 format is used for this version
  switch_config config = {2, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}, true};

  expect_backup_content_is_printed(config, expected_json, options, wago::libswitchconfig::switch_type::TI);
}

TEST(config_to_json, format_json_for_backup_of_micrel_from_v2_to_v0) {
  switch_data_provider_mock provider_mock;

  uint8_t port_mirror           = 2;  // X2 --> X1
  uint32_t broadcast_protection = 3;
  auto expected_v0              = switch_config_v0{port_mirror, broadcast_protection};

  option_map options = {
      {action_option::targetversion, "V04.07.00"}};  // former config tool format is used for this version
  switch_config config = {
      2, {{{true, "X1"}, {true, "X2"}}, "percent", 3}, {{{true, "X2"}}, "percent", 1}, {true, "X2", "X1"}, true};

  expect_backup_content_v0_is_printed(config, expected_v0, options, wago::libswitchconfig::switch_type::MICREL);
}

TEST(config_to_json, ignore_backup_of_marvell_for_fw_prior_to_v2) {
  switch_data_provider_mock provider_mock;

  auto expected_json = ::nlohmann::json::parse(switch_config_v2_json_str);
  // This has to be added for v2 format, but is ignored by ti config.
  expected_json["multicast-protection-enabled"] = false;

  option_map options   = {{action_option::targetversion, "V04.07.00"}};
  switch_config config = {1, {{{true, "X1"}}, "pps", 1000}, {{{true, "X2"}}, "pps", 2000}, {false, "X1", "X2"}};

  auto output = read_backup_content(config, options, wago::libswitchconfig::switch_type::MARVELL);
  EXPECT_TRUE(output.empty());
}

TEST(config_to_json, execute_action_for_help) {
  parameter_list parameters = {action::help};
  option_map options;

  testing::internal::CaptureStdout();
  status s           = execute_action(parameters, options, switch_data_provider_mock{});
  std::string output = testing::internal::GetCapturedStdout();

  ASSERT_THAT(s, Eq(status{}));
  EXPECT_THAT(output, HasSubstr("-c|--config"));
  EXPECT_THAT(output, HasSubstr("-b|--backup"));
  EXPECT_THAT(output, HasSubstr("-s|--supported-values"));
  EXPECT_THAT(output, HasSubstr("-h|--help"));

  EXPECT_THAT(output, HasSubstr("-t|--backup-targetversion"));
  EXPECT_THAT(output, HasSubstr("-p|--pretty"));
}

TEST(config_to_json, execute_action_with_unsupported_parameter_count) {
  parameter_list parameters = {action::help, action::json_config};

  status s = execute_action(parameters, option_map{}, switch_data_provider_mock{});

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));
}

}  // namespace wago::get_switch_config
