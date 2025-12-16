// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <string>
#include "switch_config_api.hpp"


namespace wago::libswitchconfig {

status devlink_set_broadcast_rate_limit(::std::uint32_t rate);
status devlink_set_multicast_rate_limit_enable(bool enable);
status devlink_set_broadcast_storm_protection_enable(::std::uint32_t port, bool enable);
status devlink_get_info(::std::string& info);

}  // namespace wago::libswitchconfig
