// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "switch_data_provider_i.hpp"

namespace wago::get_switch_config {

class switch_data_provider : public switch_data_provider_i {
 public:
  switch_data_provider()           = default;
  ~switch_data_provider() override = default;

  switch_data_provider(switch_data_provider &&)                 = delete;
  switch_data_provider(const switch_data_provider &)            = delete;
  switch_data_provider &operator=(switch_data_provider &&)      = delete;
  switch_data_provider &operator=(const switch_data_provider &) = delete;

  ::wago::libswitchconfig::status get_switch_config(::wago::libswitchconfig::switch_config &config) const override;
  ::wago::libswitchconfig::status get_supported_values(::wago::libswitchconfig::supported_values &sv) const override;
  ::wago::libswitchconfig::status get_switch_type(::wago::libswitchconfig::switch_type &sw_type) const override;
};

}  // namespace wago::get_switch_config
