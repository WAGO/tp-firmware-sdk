// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "tc.hpp"

#include <cassert>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>

#include "program.hpp"
#include "switch_config_api.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {

namespace {

::std::string build_tc_ingress_rate_cmd(const ::std::string& dst_mac, const ::std::string& rate, rate_type rt) {
  // TODO (Team) For Bytes per second use "rate" instead of "pkts_rate".
  //       https://man7.org/linux/man-pages/man8/tc-police.8.html

  if (rt == rate_type::PKTS_RATE) {
    // for micrel packages per second use:
    return " ingress flower skip_sw dst_mac " + dst_mac + " action police pkts_rate " + rate + " pkts_burst 1 drop";
  } else if (rt == rate_type::RATE) {
    // for marvell the rate is specified in MBits
    return " ingress flower skip_sw dst_mac " + dst_mac + " action police rate " + rate + "mbit burst 1 drop";
  } else {
    assert("TC-Tool: Invalid rate type");
    return "";
  }
}

::std::string build_tc_add_filter_cmd(const ::std::string& port_name, const ::std::string& filter_cmd) {
  return "/usr/sbin/tc filter add dev " + port_name + " " + filter_cmd;
}

::std::string build_tc_del_filter_cmd(const ::std::string& port_name, const ::std::string& pref,
                                      const ::std::string& filter) {
  return "/usr/sbin/tc filter del dev " + port_name + " pref " + pref + " " + filter;
}

::std::string build_tc_del_filter_cmd(const ::std::string& port_name, const ::std::string& filter) {
  return "/usr/sbin/tc filter del dev " + port_name + " " + filter;
}

::std::string build_tc_change_filter_cmd(const ::std::string& port_name, const ::std::string& pref,
                                         const ::std::string& handle, const std::string& filter_cmd) {
  return "/usr/sbin/tc filter replace dev " + port_name + " pref " + pref + " handle " + handle + " " + filter_cmd;
}

status get_status_from_tc_program(const program& programm) {
  return programm.get_result() == 0 ? status{} : status{status_code::SYSTEM_CALL_ERROR, programm.get_stderr()};
}

status tc_show_filter(const ::std::string& port_name, const ::std::string& filter, nlohmann::json& ingress_filter) {
  auto cmd = "/usr/sbin/tc -json filter show dev " + port_name + " " + filter;
  auto p   = program::execute(cmd);
  if (p.get_result() == 0) {
    ingress_filter = nlohmann::json::parse(p.get_stdout());
    return {};
  }

  return status{status_code::SYSTEM_CALL_ERROR, p.get_stderr()};
}

}  // namespace

::std::optional<tc_filter_ref> get_mirror_filter_ref(const nlohmann::json& filters) {
  tc_filter_ref ref;
  for (auto const& filter : filters) {
    if (!filter.contains("pref")) {
      continue;
    }

    if (filter.contains("options")) {
      if (filter.contains("kind") && filter.at("kind") != "matchall") {
        continue;
      }

      auto const& options = filter.at("options");

      if (!options.contains("handle")) {
        continue;
      }

      if (!options.contains("actions")) {
        continue;
      }

      auto const& actions = options.at("actions");
      for (auto const& action : actions) {
        if (!action.contains("kind") && action.at("kind") != "mirred") {
          continue;
        }

        if (!action.contains("mirred_action") && action.at("mirred_action") != "mirror") {
          continue;
        }

        if (action.contains("control_action")) {
          auto const& control_action = action.at("control_action");
          if (control_action.contains("type")) {
            auto const& type = control_action.at("type");
            if (type == "pipe") {
              filter.at("pref").get_to(ref.pref);
              options.at("handle").get_to(ref.handle);
              return ref;
            }
          }
        }
      }
    }
  }
  return ::std::nullopt;
}

::std::optional<tc_filter_ref> get_ingress_ratelimit_filter_ref(const nlohmann::json& ingress_filter,
                                                                const ::std::string& dst_mac) {
  tc_filter_ref ref;
  for (auto const& filter : ingress_filter) {
    if (!filter.contains("pref")) {
      continue;
    }

    if (filter.contains("options")) {
      if (filter.contains("kind") && filter.at("kind") != "flower") {
        continue;
      }

      auto const& options = filter.at("options");

      if (!options.contains("handle")) {
        continue;
      }

      if (!options.contains("actions")) {
        continue;
      }

      if (!options.contains("keys") || !options.at("keys").contains("dst_mac")) {
        continue;
      }

      if (options.at("keys").at("dst_mac") != dst_mac) {
        continue;
      }

      auto const& actions = options.at("actions");
      for (auto const& action : actions) {
        if (action.contains("control_action")) {
          auto const& control_action = action.at("control_action");
          if (control_action.contains("type")) {
            auto const& type = control_action.at("type");
            if (type == "drop") {
              filter.at("pref").get_to(ref.pref);
              options.at("handle").get_to(ref.handle);
              return ref;
            }
          }
        }
      }
    }
  }
  return ::std::nullopt;
}

status tc_show_qdisc(const ::std::string& port_name, nlohmann::json& qdiscs) {
  auto cmd = "/usr/sbin/tc -json qdisc show dev " + port_name;
  auto p   = program::execute(cmd);
  if (p.get_result() == 0) {
    qdiscs = nlohmann::json::parse(p.get_stdout());
    return {};
  }

  return status{status_code::SYSTEM_CALL_ERROR, p.get_stderr()};
}

status tc_add_qdisc_clsact(const ::std::string& port_name) {
  auto cmd = "/usr/sbin/tc qdisc add dev " + port_name + " clsact";
  auto p   = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_delete_qdisc_clsact(const ::std::string& port_name) {
  auto cmd = "/usr/sbin/tc qdisc del dev " + port_name + " clsact";
  auto p   = program::execute(cmd);

  return get_status_from_tc_program(p);
}

bool tc_has_qdisc_clsact(const nlohmann::json& qdiscs) {
  for (auto const& qdisc : qdiscs) {
    if (qdisc.contains("kind") && qdisc.at("kind") == "clsact") {
      return true;
    }
  }
  return false;
}

status tc_show_egress_filter(const ::std::string& port_name, nlohmann::json& ingress_filter) {
  return tc_show_filter(port_name, "egress", ingress_filter);
}

status tc_show_ingress_filter(const ::std::string& port_name, nlohmann::json& ingress_filter) {
  return tc_show_filter(port_name, "ingress", ingress_filter);
}

status tc_change_ingress_rate_filter(const tc_filter_ref& filter_ref, const ::std::string& port_name,
                                     const ::std::string& value, const ::std::string& dst_mac, rate_type rt) {
  auto pref   = ::std::to_string(filter_ref.pref);
  auto handle = ::std::to_string(filter_ref.handle);
  auto cmd    = build_tc_change_filter_cmd(port_name, pref, handle, build_tc_ingress_rate_cmd(dst_mac, value, rt));
  auto p      = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_add_ingress_rate_filter(const ::std::string& port_name, const ::std::string& value,
                                  const ::std::string& dst_mac, rate_type rt) {
  auto cmd = build_tc_add_filter_cmd(port_name, build_tc_ingress_rate_cmd(dst_mac, value, rt));
  auto p = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_delete_ingress_filter(const ::std::string& port_name) {
  auto cmd = build_tc_del_filter_cmd(port_name, "ingress");
  auto p   = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_delete_egress_filter(const ::std::string& port_name) {
  auto cmd = build_tc_del_filter_cmd(port_name, "egress");
  auto p   = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_delete_egress_filter(const tc_filter_ref& filter_ref, const ::std::string& port_name) {
  auto cmd = build_tc_del_filter_cmd(port_name, ::std::to_string(filter_ref.pref), "egress");
  auto p   = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_delete_ingress_filter(const tc_filter_ref& filter_ref, const ::std::string& port_name) {
  auto cmd = build_tc_del_filter_cmd(port_name, ::std::to_string(filter_ref.pref), "ingress");
  auto p   = program::execute(cmd);

  return get_status_from_tc_program(p);
}

status tc_add_mirror(const ::std::string& src_port_name, const ::std::string& dst_port_name,
                     const ::std::string& direction, switch_type switch_type) {
  auto skip_sw = "";
  if (switch_type != switch_type::TI) {
    skip_sw = "skip_sw ";
  }
  auto cmd =
      build_tc_add_filter_cmd(src_port_name, direction + " matchall " + skip_sw + "action mirred egress mirror dev " + dst_port_name);
  auto p = program::execute(cmd);

  return get_status_from_tc_program(p);
}

}  // namespace wago::libswitchconfig
