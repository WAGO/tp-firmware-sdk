// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

enum class rate_type : ::std::uint32_t {
  UNKNOWN   = 0,
  RATE      = 1,
  PKTS_RATE = 2,
};

struct tc_filter_ref {
  int handle = 0;
  int pref   = 0;
};

status tc_add_qdisc_clsact(const ::std::string& port_name);
status tc_delete_qdisc_clsact(const ::std::string& port_name);
status tc_show_qdisc(const ::std::string& port_name, nlohmann::json& qdiscs);
bool tc_has_qdisc_clsact(const nlohmann::json& qdiscs);

status tc_show_egress_filter(const ::std::string& port_name, nlohmann::json& ingress_filter);
status tc_show_ingress_filter(const ::std::string& port_name, nlohmann::json& ingress_filter);

::std::optional<tc_filter_ref> get_mirror_filter_ref(const nlohmann::json& filter);
::std::optional<tc_filter_ref> get_ingress_ratelimit_filter_ref(const nlohmann::json& ingress_filter,
                                                                const ::std::string& dst_mac);

status tc_change_ingress_rate_filter(const tc_filter_ref& filter_ref, const ::std::string& port_name,
                                     const ::std::string& value, const ::std::string& dst_mac, rate_type st);
status tc_add_ingress_rate_filter(const ::std::string& port_name, const ::std::string& value,
                                  const ::std::string& dst_mac, rate_type st);

status tc_delete_egress_filter(const ::std::string& port_name);
status tc_delete_ingress_filter(const ::std::string& port_name);
status tc_delete_egress_filter(const tc_filter_ref& filter_ref, const ::std::string& port_name);
status tc_delete_ingress_filter(const tc_filter_ref& filter_ref, const ::std::string& port_name);

status tc_add_mirror(const ::std::string& src_port_name, const ::std::string& dst_port_name,
                     const ::std::string& direction, switch_type switch_type);

}  // namespace wago::libswitchconfig
