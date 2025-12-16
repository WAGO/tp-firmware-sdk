// Copyright (c) 2023 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace wago::libswitchconfig {

struct port {
  bool operator==(const port& other) const {
    return enabled == other.enabled && name == other.name;
  }

  bool operator!=(const port& other) const {
    return !operator==(other);
  }

  bool enabled = false;
  ::std::string name;
};

struct storm_protection {
  bool operator==(const storm_protection& other) const {
    return ports == other.ports && unit == other.unit && value == other.value;
  }

  bool operator!=(const storm_protection& other) const {
    return !operator==(other);
  }

  ::std::vector<port> ports;
  ::std::string unit;
  ::std::uint32_t value = 0;
};

struct port_mirror {
  bool operator==(const port_mirror& other) const {
    return enabled == other.enabled && source == other.source && destination == other.destination;
  }

  bool operator!=(const port_mirror& other) const {
    return !operator==(other);
  }

  bool enabled = false;
  ::std::string source;
  ::std::string destination;
};

struct switch_config {
  bool operator==(const switch_config& other) const {
    return version == other.version && broadcast_protection == other.broadcast_protection &&
           multicast_protection == other.multicast_protection && port_mirroring == other.port_mirroring &&
           multicast_protection_enabled == other.multicast_protection_enabled;
  }

  bool operator!=(const switch_config& other) const {
    return !operator==(other);
  }

  ::std::uint8_t version = 0;
  storm_protection broadcast_protection;
  storm_protection multicast_protection;
  port_mirror port_mirroring;

  // only micrel switch specific parameter
  bool multicast_protection_enabled = false;
};

struct supported_values {
  ::std::uint8_t version = 0;
  ::std::vector<::std::uint32_t> broadcast_protection_values;
  ::std::vector<::std::uint32_t> multicast_protection_values;
};

enum class status_code : ::std::uint32_t {
  OK                      = 0,
  WRONG_PARAMETER_PATTERN = 1,
  SYSTEM_CALL_ERROR       = 2,
  JSON_PARSE_ERROR        = 3,
  UNKNOWN_CONFIG_VERSION  = 4,
  VALIDATION_ERROR        = 5
};

class status {
 public:
  status() = default;
  status(status_code code, ::std::string&& msg) : s_code{code}, s_msg{msg} {
  }
  virtual ~status()                      = default;
  status(const status& other)            = default;
  status(status&& other)                 = default;
  status& operator=(const status& other) = default;
  status& operator=(status&& other)      = default;

  ::std::string to_string() const {
    return s_msg;
  }

  bool ok() const {
    return s_code == status_code::OK;
  }
  bool operator==(const status& other) const {
    return s_code == other.s_code && s_msg == other.s_msg;
  }

  status_code get_code() const {
    return s_code;
  }

 private:
  status_code s_code = status_code::OK;
  ::std::string s_msg;
};

enum class switch_type : ::std::uint32_t {
  UNKNOWN = 0,
  MICREL  = 1,
  MARVELL  = 2,
  TI      = 3,
};

status get_switch_type(switch_type& st);

status switch_config_to_json(::nlohmann::json& j, const switch_config& config);
status switch_config_from_json(switch_config& config, const ::nlohmann::json& j);

status read_switch_config(switch_config& config);
status write_switch_config(const switch_config& config);

status apply_switch_config(const switch_config& config);
status get_default_switch_config(switch_config& config);

status get_supported_values(supported_values& sv);

status validate(const switch_config& config);

}  // namespace wago::libswitchconfig
