// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <fstream>
#include "iptool.hpp"
#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

void log_message(::std::string text) {
  ::std::ofstream last_error("/tmp/libswitchlog.txt", ::std::ios::app);
  if (last_error.good()) {
    last_error << text << std::endl;  // Add newline for better formatting
    last_error.flush();
    last_error.close();
  }
}

}  // namespace wago::libswitchconfig