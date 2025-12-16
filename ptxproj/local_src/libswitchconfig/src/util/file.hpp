// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

status write(std::stringstream &ss, ::std::string &&path);

status read(std::stringstream &ss, ::std::string &&path);

}