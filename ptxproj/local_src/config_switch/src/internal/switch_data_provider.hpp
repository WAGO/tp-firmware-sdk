// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "switch_data_provider_i.hpp"

namespace wago::config_switch {

using wago::libswitchconfig::status;
using wago::libswitchconfig::switch_config;
using wago::libswitchconfig::switch_type;

class switch_data_provider : public switch_data_provider_i {
 public:
  switch_data_provider()           = default;
  ~switch_data_provider() override = default;

  switch_data_provider(switch_data_provider &&)                 = delete;
  switch_data_provider(const switch_data_provider &)            = delete;
  switch_data_provider &operator=(switch_data_provider &&)      = delete;
  switch_data_provider &operator=(const switch_data_provider &) = delete;

  status get_switch_type(switch_type &sw_type) const override;
  status read_switch_config(switch_config &config) const override;
  status write_switch_config(const switch_config &config) const override;
  status apply_switch_config(const switch_config &config) const override;
  status get_default_switch_config(switch_config &config) const override;
  status validate(const switch_config &config) const override;
};

}  // namespace wago::config_switch
