// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ostream>
#include <switch_config_api.hpp>

#include "switch_config_output.hpp"

namespace wago::get_switch_config {

using namespace wago::libswitchconfig;  // NOLINT google-build-using-namespace
using namespace ::testing;              // NOLINT google-build-using-namespace

MATCHER_P(ContainsKey, key, "Map contains key") { // NOLINT clang-diagnostic-deprecated
  return arg.count(key) >= 1;
}

MATCHER_P2(ContainsValue, key, value, "Map contains value") { // NOLINT clang-diagnostic-deprecated
  if (arg.count(key) >= 1) {
    return arg.at(key) == value;
  };
  return false;
}

class parse_cmdline : public ::testing::Test {
  void TearDown() override {
    parameters.clear();
    options.clear();
  }

 protected:
  parameter_list parameters;
  option_map options;
};

TEST_F(parse_cmdline, request_config_as_json_string) {
  ::std::vector< ::std::string> args = {"--config"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.ok(), Eq(true));
  EXPECT_THAT(parameters, SizeIs(1));
  EXPECT_THAT(parameters, Contains(action::json_config));
  EXPECT_THAT(options, SizeIs(0));
}

TEST_F(parse_cmdline, request_config_as_pretty_json_string) {
  ::std::vector< ::std::string> args = {"--config", "--pretty"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.ok(), Eq(true));
  EXPECT_THAT(parameters, SizeIs(1));
  EXPECT_THAT(parameters, Contains(action::json_config));
  EXPECT_THAT(options, SizeIs(1));
  EXPECT_THAT(options, ContainsKey(action_option::pretty));
}

TEST_F(parse_cmdline, request_backup_string) {
  ::std::vector< ::std::string> args = {"--backup"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.ok(), Eq(true));
  EXPECT_THAT(parameters, SizeIs(1));
  EXPECT_THAT(parameters, Contains(action::backup));
  EXPECT_THAT(options, SizeIs(0));
}

TEST_F(parse_cmdline, request_supported_values_string) {
  ::std::vector< ::std::string> args = {"--supported-values"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.ok(), Eq(true));
  EXPECT_THAT(parameters, SizeIs(1));
  EXPECT_THAT(parameters, Contains(action::supported_values));
  EXPECT_THAT(options, SizeIs(0));
}

TEST_F(parse_cmdline, request_backup_targetversion_string) {
  ::std::vector< ::std::string> args = {"--backup", "--backup-targetversion=04.04.00(26)"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.ok(), Eq(true));
  EXPECT_THAT(parameters, SizeIs(1));
  EXPECT_THAT(parameters, Contains(action::backup));
  EXPECT_THAT(options, SizeIs(1));
  EXPECT_THAT(options, ContainsKey(action_option::targetversion));
  EXPECT_THAT(options, ContainsValue(action_option::targetversion, "04.04.00(26)"));
}

TEST_F(parse_cmdline, reject_too_many_parameters) {
  ::std::vector< ::std::string> args = {"--backup", "--help", "--config"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));
}

TEST_F(parse_cmdline, reject_duplicate_parameters) {
  ::std::vector< ::std::string> args = {"--backup", "--backup"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));
}

TEST_F(parse_cmdline, reject_unknown_parameters) {
  ::std::vector< ::std::string> args = {"--foo", "--help"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));

  parameters.clear();
  ::std::vector< ::std::string> args_reordered = {"--help", "--foo"};
  s                                            = parse_commandline_args(args_reordered, parameters, options);

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));
}

TEST_F(parse_cmdline, reject_missing_targetversion_parameter) {
  ::std::vector< ::std::string> args = {"--backup", "--backup-targetversion="};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));
}

TEST_F(parse_cmdline, reject_invalid_targetversion_parameter) {
  ::std::vector< ::std::string> args = {"--backup", "--backup-targetversion=Test"};
  auto s                             = parse_commandline_args(args, parameters, options);

  ASSERT_THAT(s.get_code(), Eq(status_code::WRONG_PARAMETER_PATTERN));
}

}  // namespace wago::get_switch_config
