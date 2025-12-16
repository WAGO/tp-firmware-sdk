// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <nlohmann/json.hpp>

#include "switch_config_api.hpp"

namespace wago::libswitchconfig {

::std::string bool_to_string(bool value);
::std::string bool_to_string(bool value) {
  return (value) ? "true" : "false";
}

void PrintTo(const port& p, std::ostream* os);
void PrintTo(const port& p, std::ostream* os) {
  *os << "(" << p.name << ": " << bool_to_string(p.enabled) << ")";
}

void PrintTo(const ::std::vector<port>& ports, std::ostream* os);
void PrintTo(const ::std::vector<port>& ports, std::ostream* os) {
  *os << "(";
  for (const auto& p : ports) {
    PrintTo(p, os);
  }
  *os << ")";
}

void PrintTo(const storm_protection& sp, std::ostream* os);
void PrintTo(const storm_protection& sp, std::ostream* os) {
  *os << "(";
  PrintTo(sp.ports, os);
  *os << ", " << sp.value << ", " << sp.unit << ")";
}

void PrintTo(const port_mirror& pm, std::ostream* os);
void PrintTo(const port_mirror& pm, std::ostream* os) {
  *os << "(" << bool_to_string(pm.enabled) << ", " << pm.source << ", " << pm.destination << ")";
}

void PrintTo(const switch_config& sc, std::ostream* os);
void PrintTo(const switch_config& sc, std::ostream* os) {
  *os << "(bc: ";
  PrintTo(sc.broadcast_protection, os);
  *os << ", mc: ";
  PrintTo(sc.multicast_protection, os);
  *os << ", pm: ";
  PrintTo(sc.port_mirroring, os);
  *os << ", version: " << ::std::to_string(static_cast<unsigned int>(sc.version)) << ")";
}

}  // namespace wago::libswitchconfig
