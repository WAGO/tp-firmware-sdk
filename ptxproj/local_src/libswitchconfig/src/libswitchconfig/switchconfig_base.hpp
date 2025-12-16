// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "iptool.hpp"
#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

::std::vector<::std::string> get_system_ports_as_label();
switch_config clear_switch_unsopported_parameters(const switch_config& config, switch_type st);
void convert_to_system_port_names(switch_config& config);
bool contains(const ::std::vector<::std::string>& values, const ::std::string& value);

status get_port_hw_mapping(::std::map<::std::string, ::std::uint32_t>& port_hw_mapping);

::std::vector<::std::string> ports_requires_qdisc_not(const ::std::vector<system_port>& system_ports,
                                                  const ::std::vector<::std::string>& filtered_ports);
::std::vector<::std::string> ports_requires_qdisc(const switch_config& config);

}  // namespace wago::libswitchconfig