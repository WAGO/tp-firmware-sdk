// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <switch_config_api.hpp>

namespace wago::get_switch_config {

class switch_data_provider_i {
 public:
  switch_data_provider_i()          = default;
  virtual ~switch_data_provider_i() = default;

  switch_data_provider_i(switch_data_provider_i &&)                 = delete;
  switch_data_provider_i(const switch_data_provider_i &)            = delete;
  switch_data_provider_i &operator=(switch_data_provider_i &&)      = delete;
  switch_data_provider_i &operator=(const switch_data_provider_i &) = delete;

  virtual ::wago::libswitchconfig::status get_switch_config(::wago::libswitchconfig::switch_config &config) const   = 0;
  virtual ::wago::libswitchconfig::status get_supported_values(::wago::libswitchconfig::supported_values &sv) const = 0;
  virtual ::wago::libswitchconfig::status get_switch_type(::wago::libswitchconfig::switch_type &sw_type) const      = 0;
};

}  // namespace wago::get_switch_config
