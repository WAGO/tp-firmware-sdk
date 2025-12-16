// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <switch_config_api.hpp>
#include <exception>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <system_error>

namespace wago::config_switch {

using parameter_map = std::unordered_map<::std::string, ::std::string>;

void check_that_parameters_are_known(parameter_map const& parameters);

void edit_parameters(const ::std::string& json_string, wago::libswitchconfig::switch_config& config);

enum class switch_config_error {
  invalid_parameter = 1,
  missing_parameter = 2,
};
class switch_config_exception final : public ::std::runtime_error {
 public:
  switch_config_exception(switch_config_error switch_config_error, ::std::string const& what)
      : ::std::runtime_error(what), error_(switch_config_error) {
  }
  switch_config_exception(const switch_config_exception&)            = default;
  switch_config_exception& operator=(const switch_config_exception&) = default;
  switch_config_exception(switch_config_exception&&)                 = default;
  switch_config_exception& operator=(switch_config_exception&&)      = default;

  ~switch_config_exception() final = default;

  switch_config_error error() const {
    return error_;
  }

 private:
  switch_config_error error_;
};

}  // namespace wago::snmp_config
