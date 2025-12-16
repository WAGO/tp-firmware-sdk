// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <switch_config_api.hpp>

namespace wago::config_switch {

class switch_data_provider_i {
 public:
  switch_data_provider_i()          = default;
  virtual ~switch_data_provider_i() = default;

  switch_data_provider_i(switch_data_provider_i &&)                 = delete;
  switch_data_provider_i(const switch_data_provider_i &)            = delete;
  switch_data_provider_i &operator=(switch_data_provider_i &&)      = delete;
  switch_data_provider_i &operator=(const switch_data_provider_i &) = delete;

  // virtual ::wago::libswitchconfig::status config_switch(::wago::libswitchconfig::switch_config &config) const = 0;
  // virtual ::wago::libswitchconfig::status get_supported_values(nlohmann::json &j) const                           = 0;

  virtual ::wago::libswitchconfig::status get_switch_type(::wago::libswitchconfig::switch_type &sw_type) const    = 0;
  virtual ::wago::libswitchconfig::status read_switch_config(::wago::libswitchconfig::switch_config &config) const = 0;
  virtual ::wago::libswitchconfig::status write_switch_config(const ::wago::libswitchconfig::switch_config &config) const = 0;
  virtual ::wago::libswitchconfig::status apply_switch_config(const ::wago::libswitchconfig::switch_config &config) const = 0;
  virtual ::wago::libswitchconfig::status get_default_switch_config(::wago::libswitchconfig::switch_config &config) const = 0;
  virtual ::wago::libswitchconfig::status validate(const ::wago::libswitchconfig::switch_config &config) const = 0;

};

}  // namespace wago::config_switch
