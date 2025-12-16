// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <gmock/gmock.h>

#include "switch_config_api.hpp"
#include "switch_data_provider_i.hpp"

namespace wago::get_switch_config {

class switch_data_provider_mock : public switch_data_provider_i {
 public:
  MOCK_CONST_METHOD1(get_switch_config, ::wago::libswitchconfig::status(::wago::libswitchconfig::switch_config &));
  MOCK_CONST_METHOD1(get_supported_values,
                     ::wago::libswitchconfig::status(::wago::libswitchconfig::supported_values &));
  MOCK_CONST_METHOD1(get_switch_type, ::wago::libswitchconfig::status(::wago::libswitchconfig::switch_type &));
};

}  // namespace wago::get_switch_config
