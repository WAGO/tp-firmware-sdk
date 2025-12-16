// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <gmock/gmock.h>

#include "switch_data_provider_i.hpp"

namespace wago::config_switch {

class switch_data_provider_mock : public switch_data_provider_i {
 public:
  MOCK_CONST_METHOD1(get_switch_type, ::wago::libswitchconfig::status(::wago::libswitchconfig::switch_type &));
  MOCK_CONST_METHOD1(read_switch_config, ::wago::libswitchconfig::status(::wago::libswitchconfig::switch_config &));
  MOCK_CONST_METHOD1(write_switch_config,
                     ::wago::libswitchconfig::status(const ::wago::libswitchconfig::switch_config &));
  MOCK_CONST_METHOD1(apply_switch_config,
                     ::wago::libswitchconfig::status(const ::wago::libswitchconfig::switch_config &));
  MOCK_CONST_METHOD1(get_default_switch_config,
                     ::wago::libswitchconfig::status(::wago::libswitchconfig::switch_config &));
  MOCK_CONST_METHOD1(validate, ::wago::libswitchconfig::status(const ::wago::libswitchconfig::switch_config &));
};

}  // namespace wago::config_switch
