// Copyright (c) 2025 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "write_last_error.hpp"
#include <iostream>

namespace wago::get_switch_config {

void write_last_error(::std::string const& text) {
  ::std::cerr << text;
}

}  // namespace wago::get_switch_config
