// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switchconfig_apply.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "devlink_micrel.hpp"
#include "iptool.hpp"
#include "switch_config_api.hpp"
#include "switchconfig_base.hpp"
#include "switchconfig_constants.hpp"
#include "tc.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

status add_qdisc_clsact(const ::std::string& port) {
  nlohmann::json qdiscs;
  auto status = tc_show_qdisc(port, qdiscs);

  if (!status.ok()) {
    return status;
  }

  if (!tc_has_qdisc_clsact(qdiscs)) {
    return tc_add_qdisc_clsact(port);
  }

  return {};
}

status add_qdisc_clsact(const ::std::vector<::std::string> ports) {
  for (auto const& port : ports) {
    auto status = add_qdisc_clsact(port);
    if (!status.ok()) {
      return status;
    }
  }
  return {};
}

status delete_qdisc_clsact(const ::std::string& port) {
  nlohmann::json qdiscs;
  auto status = tc_show_qdisc(port, qdiscs);

  if (!status.ok()) {
    return status;
  }

  if (tc_has_qdisc_clsact(qdiscs)) {
    return tc_delete_qdisc_clsact(port);
  }

  return {};
}

status delete_qdisc_clsact(const ::std::vector<::std::string> ports) {
  for (auto const& port : ports) {
    auto status = delete_qdisc_clsact(port);
    if (!status.ok()) {
      return status;
    }
  }
  return {};
}

status apply_protection(const port& p, const ::std::string& value, const ::std::string& dst_mac, switch_type st) {
  nlohmann::json ingress_filters;

  // for MARVELL use rate_type::RATE and vor TI use rate_type::PKTS_RATE
  rate_type rt = (st == switch_type::TI) ? rate_type::PKTS_RATE : rate_type::RATE;

  auto status = tc_show_ingress_filter(p.name, ingress_filters);
  if (status.ok()) {
    auto existing_filter_ref = get_ingress_ratelimit_filter_ref(ingress_filters, dst_mac);
    if (p.enabled) {
      status = existing_filter_ref
                   ? tc_change_ingress_rate_filter(existing_filter_ref.value(), p.name, value, dst_mac, rt)
                   : tc_add_ingress_rate_filter(p.name, value, dst_mac, rt);
    } else {
      if (existing_filter_ref) {
        status = tc_delete_ingress_filter(existing_filter_ref.value(), p.name);
      }
    }
  }

  return status;
}

status remove_port_mirror(const ::std::string& port_name) {
  nlohmann::json filter;
  auto status = tc_show_ingress_filter(port_name, filter);
  if (!status.ok()) {
    return status;
  }
  auto filter_ref = get_mirror_filter_ref(filter);
  if (filter_ref) {
    status = tc_delete_ingress_filter(filter_ref.value(), port_name);
    if (!status.ok()) {
      return status;
    }
  }

  status = tc_show_egress_filter(port_name, filter);
  if (status.ok()) {
    filter_ref = get_mirror_filter_ref(filter);
    if (filter_ref) {
      return tc_delete_egress_filter(filter_ref.value(), port_name);
    }
  }

  return {};
}

status apply_port_mirroring(const port_mirror port_mirror, switch_type switch_type) {
  status status = tc_add_mirror(port_mirror.source, port_mirror.destination, "egress", switch_type);
  if (!status.ok()) {
    return status;
  }

  return tc_add_mirror(port_mirror.source, port_mirror.destination, "ingress", switch_type);
}

status apply_port_mirroring(const port_mirror& port_mirror, const ::std::vector<system_port>& system_ports, switch_type switch_type) {
  status status{};
  if (port_mirror.enabled) {
    status = apply_port_mirroring(port_mirror, switch_type);
  } else {
    for (auto const& p : system_ports) {
      if (status.ok()) {
        status = remove_port_mirror(p.name);
      }
    }
  }

  return status;
}

::std::uint32_t convert_micrel_bcast_rate_to_hw(::std::uint32_t rate_percent) {
  constexpr ::std::uint32_t max_rate_value = 2047;  // the broadcast rate with is 11 bits
  constexpr ::std::uint32_t broadcast_storm_value =
      9969;  // 148,800 frames * 67 ms / 100; taken from the switch documentation

  double rate = (static_cast<double>(rate_percent) * broadcast_storm_value) / 100.0;
  if (rate > max_rate_value) {
    rate = max_rate_value;
  }
  return static_cast<::std::uint32_t>(std::floor(rate));
}

status apply_config_micrel(switch_config& sw_config) {
  auto rate = convert_micrel_bcast_rate_to_hw(sw_config.broadcast_protection.value);

  status status = devlink_set_broadcast_rate_limit(rate);

  if (status.ok()) {
    status = devlink_set_multicast_rate_limit_enable(sw_config.multicast_protection_enabled);
  }

  ::std::map<::std::string, ::std::uint32_t> port_hw_mapping;
  if (status.ok()) {
    status = get_port_hw_mapping(port_hw_mapping);
  }

  //The driver enables protection for the switch port connected to the CPU by default.
  //We don't want this because we suspect it could cause problems with protocols like EtherCAT that send telegrams via broadcast.
  if (status.ok()) {
    status = devlink_set_broadcast_storm_protection_enable(port_hw_mapping["cpu"], false);
  }

  for (const auto& p : sw_config.broadcast_protection.ports) {
    if (status.ok()) {
      status = devlink_set_broadcast_storm_protection_enable(port_hw_mapping[p.name], p.enabled);
    }
  }

  return status;
}

status apply_config(switch_config& sw_config, switch_type st) {
  status status;

  for (auto const& p : sw_config.broadcast_protection.ports) {
    status = apply_protection(p, ::std::to_string(static_cast<size_t>(sw_config.broadcast_protection.value)),
                              broadcast_mac, st);
    if (!status.ok()) {
      return status;
    }
  }

  for (auto const& p : sw_config.multicast_protection.ports) {
    status = apply_protection(p, ::std::to_string(static_cast<size_t>(sw_config.multicast_protection.value)),
                              multicast_mac, st);
    if (!status.ok()) {
      return status;
    }
  }

  return status;
}

status configure_qdisc(switch_config& sw_config, ::std::vector<system_port>& system_ports) {
  // porst for which a qdisq is set up
  auto qdisc_ports = ports_requires_qdisc(sw_config);
  // porst for which a qdisq is removed
  auto no_qdisc_ports = ports_requires_qdisc_not(system_ports, qdisc_ports);

  status status = delete_qdisc_clsact(no_qdisc_ports);
  if (!status.ok()) {
    return status;
  }

  return add_qdisc_clsact(qdisc_ports);
}

}  // namespace

status apply_switch_config(const switch_config& config) {
  switch_type st;
  status s = get_switch_type(st);

  switch_config sw_config;
  if(s.ok()){
    sw_config = clear_switch_unsopported_parameters(config, st);
    convert_to_system_port_names(sw_config);
  }

  ::std::vector<system_port> system_ports;
  if (s.ok()) {
    s = get_system_ports(system_ports);
  }

  if (s.ok()) {
    s = configure_qdisc(sw_config, system_ports);
  }
  if (s.ok()) {
    if(st == switch_type::MICREL){
      s = apply_config_micrel(sw_config);
    }
    if (st == switch_type::TI || st == switch_type::MARVELL) {
      s = apply_config(sw_config, st);
    }
  }

  if (s.ok()) {
    s = apply_port_mirroring(sw_config.port_mirroring, system_ports, st);
  }

  return status{s.get_code(), s.to_string()};
}

}  // namespace wago::libswitchconfig
