// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switch_config_v0.hpp"


::std::string switch_config_v0::to_string() const {
    return "switch-port-mirror=" + ::std::to_string(port_mirror) +
           "\nswitch-bcast-protect=" + ::std::to_string(broadcast_protection) + "\nswitch-rate-limit=disabled";
  }
