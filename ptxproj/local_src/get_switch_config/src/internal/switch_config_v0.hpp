// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cstdint>
#include <string>
struct switch_config_v0 {
  uint8_t port_mirror          = 0;
  uint32_t broadcast_protection = 0;

  ::std::string to_string() const;
};
