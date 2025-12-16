// Copyright (c) 2025 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-actions.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
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

constexpr auto supported_values_json_str =
    R"({"broadcast-protection":[1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,20000,30000,40000,50000,60000,70000,80000,90000,100000,200000,300000,400000,500000],"multicast-protection":[1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,20000,30000,40000,50000,60000,70000,80000,90000,100000,200000,300000,400000,500000],"version":2})";
constexpr auto supported_values_micrel_json_str =
    R"({"broadcast-protection":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20],"multicast-protection":[],"version":2})";

MATCHER_P(StrLengthIsLessOrEqualTo, max_length, "") {  // NOLINT clang-diagnostic-deprecated
  return arg.length() <= static_cast<size_t>(max_length);
}

::std::string read_supported_values(wago::libswitchconfig::supported_values& sv) {
  switch_data_provider_mock provider_mock;
  // clang-format off
  EXPECT_CALL(provider_mock, get_supported_values(_))
    .WillOnce(DoAll(
      SetArgReferee<0>(sv),
      Return(status{})
    ));
  // clang-format on

  testing::internal::CaptureStdout();
  print_supported_values(option_map{}, provider_mock);
  std::string output = testing::internal::GetCapturedStdout();
  return output;
}

}  // namespace
namespace wago::get_switch_config {

using namespace wago::libswitchconfig;  // NOLINT google-build-using-namespace
using namespace ::testing;              // NOLINT google-build-using-namespace

TEST(supported_values, print_supported_values) {
  supported_values sv;
  sv.version                     = 2;
  sv.broadcast_protection_values = {1000,  2000,  3000,   4000,   5000,   6000,   7000,  8000,
                                    9000,  10000, 20000,  30000,  40000,  50000,  60000, 70000,
                                    80000, 90000, 100000, 200000, 300000, 400000, 500000};
  sv.multicast_protection_values = {1000,  2000,  3000,   4000,   5000,   6000,   7000,  8000,
                                    9000,  10000, 20000,  30000,  40000,  50000,  60000, 70000,
                                    80000, 90000, 100000, 200000, 300000, 400000, 500000};
  auto expected_json             = ::nlohmann::json::parse(supported_values_json_str);

  auto output = read_supported_values(sv);

  EXPECT_EQ(::nlohmann::json::parse(output), expected_json);
}

TEST(supported_values, print_supported_values_micrel) {
  switch_data_provider_mock provider_mock;
  supported_values sv;
  sv.version                     = 2;
  sv.broadcast_protection_values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  sv.multicast_protection_values = {};
  auto expected_json             = ::nlohmann::json::parse(supported_values_micrel_json_str);

  auto output = read_supported_values(sv);

  EXPECT_EQ(::nlohmann::json::parse(output), expected_json);
}

TEST(supported_values, try_to_print_supported_values_for_unknown_version) {
  supported_values sv;
  sv.version                     = 5;
  sv.broadcast_protection_values = {1000,  2000,  3000,   4000,   5000,   6000,   7000,  8000,
                                    9000,  10000, 20000,  30000,  40000,  50000,  60000, 70000,
                                    80000, 90000, 100000, 200000, 300000, 400000, 500000};
  sv.multicast_protection_values = {1000,  2000,  3000,   4000,   5000,   6000,   7000,  8000,
                                    9000,  10000, 20000,  30000,  40000,  50000,  60000, 70000,
                                    80000, 90000, 100000, 200000, 300000, 400000, 500000};

  // clang-format off
  EXPECT_DEATH({
    switch_data_provider_mock provider_mock;
    ::testing::Mock::AllowLeak(&provider_mock);
    EXPECT_CALL(provider_mock, get_supported_values(_))
      .WillOnce(DoAll(
        SetArgReferee<0>(sv),
        Return(status{})
      ));
    print_supported_values(option_map{}, provider_mock);
  }, "Unknown config version: 5");
  // clang-format on
}

TEST(supported_values, provider_returns_error) {

  // clang-format off
  EXPECT_DEATH({
    switch_data_provider_mock provider_mock;
    ::testing::Mock::AllowLeak(&provider_mock);
    EXPECT_CALL(provider_mock, get_supported_values(_))
      .WillOnce(Return(status{status_code{1}, "provider error"})
      );
    print_supported_values(option_map{}, provider_mock);
  }, "provider error");
  // clang-format on
}

}  // namespace wago::get_switch_config
