// Copyright (c) 2025 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "write_last_error.hpp"

#include <fstream>


namespace wago::get_switch_config {

void write_last_error(::std::string const& text) {
  ::std::ofstream last_error;
  last_error.open("/tmp/last_error.txt");
  if (last_error.good()) {
    last_error << text;
    last_error.flush();
    last_error.close();
  }
}

}  // namespace wago::get_switch_config
