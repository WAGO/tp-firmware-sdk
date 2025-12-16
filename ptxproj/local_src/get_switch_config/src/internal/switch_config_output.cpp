// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#include "switch_config_output.hpp"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <regex>
#include <set>
#include <string>

#include "switch_config_api.hpp"
#include "switch_config_v0.hpp"
#include "write_last_error.hpp"

using json = nlohmann::json;

namespace wago::libswitchconfig {
void to_json(nlohmann::json& j, const ::wago::libswitchconfig::supported_values& sv);
void to_json(nlohmann::json& j, const ::wago::libswitchconfig::supported_values& sv) {
  j = nlohmann::json{{"version", sv.version},
                     {"broadcast-protection", sv.broadcast_protection_values},
                     {"multicast-protection", sv.multicast_protection_values}};
}
}  // namespace wago::libswitchconfig

namespace {
using wago::libswitchconfig::switch_config;
using wago::libswitchconfig::switch_type;

void show_help() {
  const ::std::string help_text = {R"(
      Usage: get_switch_config [--backup [--backup-targetversion=<fw-version>] | --pretty ]

      Print the switch configuration.

      -c|--config                              Print the network switch configuration.
      -b|--backup                              Output the configuration in backup file format.
      -s|--supported-values                    Output the supported values.
      -h|--help                                Print this help text.

      -t|--backup-targetversion=<fw-version>   Request a specific target firmware version, e.g. 04.05.00(27)
      -p|--pretty                              Output the configuration in json format that is more readable for humans.
      )"};

  ::std::cout << help_text << ::std::endl;
}

void print_json(const json& j, const bool pretty) {
  if (pretty) {
    ::std::cout << std::setw(4) << j;
  } else {
    ::std::cout << j;
  }
}

void print_backup_content_json(const ::std::string& data) {
  const ::std::string key           = "switch.data";
  const int chars_per_line_         = 88;
  const size_t value_chars_per_line = chars_per_line_ - (key.length() + 1);

  std::stringstream ss;
  auto lines = static_cast<uint32_t>(::std::floor(data.size() / value_chars_per_line));
  for (uint32_t line = 0; line < lines; line++) {
    ::std::string substring = data.substr(line * value_chars_per_line, value_chars_per_line);
    ss << key << '=' << substring << '\n';
  }

  auto remaining_chars = data.size() % value_chars_per_line;
  if (remaining_chars > 0) {
    ::std::string substring = data.substr(lines * value_chars_per_line, data.size());
    ss << key << '=' << substring << '\n';
  }

  ::std::cout << ss.str();
}

::wago::libswitchconfig::status supported_values_to_json(nlohmann::json& j,
                                                         const ::wago::libswitchconfig::supported_values& sv) {
  ::wago::libswitchconfig::status s{};
  if (sv.version <= 2) {
    j = sv;
  } else {
    s = ::wago::libswitchconfig::status{::wago::libswitchconfig::status_code::UNKNOWN_CONFIG_VERSION,
                                        "Unknown config version: " + std::to_string(sv.version)};
  }

  return s;
}

uint8_t get_port_mirror_v0(const switch_config& config) {
  if (!config.port_mirroring.enabled) {
    return 0;
  }

  if (config.port_mirroring.source == "X1") {
    return 1;
  }

  if (config.port_mirroring.source == "X2") {
    return 2;
  }
  return 0;
}

// Get the broadcast protection value for the original Micrel switch configuration format.
uint32_t get_broadcast_protection_v0(const switch_config& config) {
  if (config.broadcast_protection.ports.empty() || config.broadcast_protection.ports.size() != 2) {
    wago::get_switch_config::exit_with_error(wago::get_switch_config::get_switch_config_error::internal_error,
                                             "Broadcast protection downgrade to v0 format requires two ports.");
  }

  if (config.broadcast_protection.ports[0].enabled && config.broadcast_protection.ports[1].enabled) {
    if (config.broadcast_protection.unit == "percent") {
      return ::std::min<uint32_t>(5, config.broadcast_protection.value);
    }

    wago::get_switch_config::exit_with_error(wago::get_switch_config::get_switch_config_error::internal_error,
                                             "Broadcast protection downgrade to v0 format requires unit 'percent'.");
  }
  return 0;
}

switch_config_v0 create_switch_config_v0(switch_config& config) {
  switch_config_v0 config_v0;
  config_v0.port_mirror          = get_port_mirror_v0(config);
  config_v0.broadcast_protection = get_broadcast_protection_v0(config);
  return config_v0;
}

void print_backup_content_v0(switch_config& config) {
  switch_config_v0 config_v0 = create_switch_config_v0(config);

  ::std::cout << config_v0.to_string() << '\n';
}

void print_backup_content_v1(switch_config& config) {
  json j;
  config.version = 1;  // Ensure the version is set to 1 for v1 format

  wago::libswitchconfig::status s = wago::libswitchconfig::switch_config_to_json(j, config);

  if (s.ok()) {
    ::std::string data = j.dump();
    print_backup_content_json(data);
  } else {
    exit_with_error(wago::get_switch_config::get_switch_config_error::internal_error, s.to_string());
  }
}

void print_backup_content_v2(wago::libswitchconfig::switch_config& config) {
  json j;
  config.version = 2;  // Ensure the version is set to 2 for v2 format

  wago::libswitchconfig::status s = wago::libswitchconfig::switch_config_to_json(j, config);
  if (s.ok()) {
    ::std::string data = j.dump();
    print_backup_content_json(data);
  } else {
    exit_with_error(wago::get_switch_config::get_switch_config_error::internal_error, s.to_string());
  }
}

bool version_is_greater_than_or_equal(const ::std::string& v1, const ::std::string& v2) {
  std::regex version_regex(R"(^(\d{2})\.(\d{2})\.(\d{2})\((\d{2})\))");
  std::smatch v1_match, v2_match;

  if (std::regex_match(v1, v1_match, version_regex) && std::regex_match(v2, v2_match, version_regex)) {
    int v1_major = std::stoi(v1_match[1]);
    int v1_minor = std::stoi(v1_match[2]);
    int v2_major = std::stoi(v2_match[1]);
    int v2_minor = std::stoi(v2_match[2]);

    if (v1_major < v2_major) {
      return false;
    }

    if (v1_major > v2_major) {
      return true;
    }

    return v1_minor >= v2_minor;
  }
  return false;  // invalid format
}

::std::string get_config_version(const ::std::string& target_version, const switch_type& sw_type) {
  ::std::string version = "ignore";

  if (version_is_greater_than_or_equal(target_version, "04.08.00(00)")) {
    version = "2";
  } else {
    if (sw_type == switch_type::MICREL) {
      version = "0";  // Use v0 format for MICREL switches
    } else if (sw_type == switch_type::TI) {
      version = "1";  // Use v1 format for TI switches
    }
    // Ignore Marvell switches, as they are not supported in FWs before 04.08.00(00), FW30
  }
  return version;
}

}  // namespace

namespace wago::get_switch_config {
using libswitchconfig::switch_type;

wago::libswitchconfig::status parse_commandline_args(const ::std::vector<::std::string>& args,
                                                     parameter_list& parameters, option_map& options) {
  wago::libswitchconfig::status s{};

  // actions
  ::std::set<::std::string> config_args{"-c", "--config"};
  ::std::set<::std::string> backup_args{"-b", "--backup"};
  ::std::set<::std::string> supported_values{"-s", "--supported-values"};
  ::std::set<::std::string> help_args{"-h", "--help"};
  // options
  ::std::set<::std::string> backup_targetversion_args{"-t=", "--backup-targetversion="};
  ::std::set<::std::string> pretty_args{"-p", "--pretty"};

  if (args.size() > 2) {
    return {wago::libswitchconfig::status_code::WRONG_PARAMETER_PATTERN, "Invalid parameter count."};
  }

  for (const auto& arg : args) {
    action action_arg;

    if (arg.rfind("-t=", 0) == 0 || arg.rfind("--backup-targetversion=", 0) == 0) {
      auto pos = arg.find('=');
      if (pos + 1 < arg.length()) {
        ::std::string version = arg.substr(pos + 1);
        if (is_valid_fw_version(version)) {
          options.emplace(action_option::targetversion, version);
          continue;
        }
        s = {wago::libswitchconfig::status_code::WRONG_PARAMETER_PATTERN, "Invalid targetversion value"};
      } else {
        s = {wago::libswitchconfig::status_code::WRONG_PARAMETER_PATTERN, "Missing targetversion value"};
      }
    } else if (config_args.count(arg) > 0) {
      action_arg = action::json_config;
    } else if (backup_args.count(arg) > 0) {
      action_arg = action::backup;
    } else if (supported_values.count(arg) > 0) {
      action_arg = action::supported_values;
    } else if (help_args.count(arg) > 0) {
      action_arg = action::help;
    } else if (pretty_args.count(arg) > 0) {
      options.emplace(action_option::pretty, "");
      continue;
    } else {
      s = {wago::libswitchconfig::status_code::WRONG_PARAMETER_PATTERN, "Unknown parameter"};
    }

    if (s.ok()) {
      if (::std::find(parameters.cbegin(), parameters.cend(), action_arg) == parameters.cend()) {
        parameters.emplace_back(action_arg);
      } else {
        s = {wago::libswitchconfig::status_code::WRONG_PARAMETER_PATTERN, "Found duplicate parameter"};
      }
    }

    if (!s.ok()) {
      break;
    }
  }

  if (parameters.empty()) {
    parameters.emplace_back(action::help);
  }

  return s;
}

libswitchconfig::status execute_action(const parameter_list& parameters, const option_map& options,
                                       const switch_data_provider_i& provider) {
  if (parameters.size() != 1) {
    return libswitchconfig::status{libswitchconfig::status_code::WRONG_PARAMETER_PATTERN, "Invalid parameter count."};
  }

  action_map action_funcs = {
      {action::json_config, [](const option_map& om, const switch_data_provider_i& p) { print_json_config(om, p); }},
      {action::backup, [](const option_map& om, const switch_data_provider_i& p) { print_backup_content(om, p); }},
      {action::supported_values,
       [](const option_map& om, const switch_data_provider_i& p) { print_supported_values(om, p); }},
      {action::help, [](const option_map&, const switch_data_provider_i&) { show_help(); }}};

  action selected_action = parameters.front();
  action_funcs.at(selected_action)(options, provider);

  return libswitchconfig::status{};
}

[[noreturn]] void exit_with_error(get_switch_config_error code, ::std::string const& text) {
  write_last_error(text);
  std::cerr << text << std::endl;
  ::std::exit(static_cast<int>(code));
}

void clear_last_error() {
  write_last_error(::std::string{});
}

void print_json_config(const option_map& options, const switch_data_provider_i& provider) {
  json j;
  libswitchconfig::switch_config config;

  libswitchconfig::status s = provider.get_switch_config(config);
  if (s.ok()) {
    s = libswitchconfig::switch_config_to_json(j, config);
  }

  if (s.ok()) {
    print_json(j, options.count(action_option::pretty) > 0);
  } else {
    exit_with_error(get_switch_config_error::internal_error, s.to_string());
  }
}

void print_backup_content(const option_map& options, const switch_data_provider_i& provider) {
  switch_type sw_type = switch_type::UNKNOWN;
  libswitchconfig::switch_config config;

  libswitchconfig::status s = provider.get_switch_config(config);

  if (s.ok()) {
    s = provider.get_switch_type(sw_type);
  }

  if (s.ok()) {
    ::std::string target_version = "04.08.00(00)";  // version 2 format is first supported in FW 04.08.00(00)
    ::std::string version        = "2";
    if (options.count(action_option::targetversion) > 0) {
      target_version = options.at(action_option::targetversion);
    }
    version = get_config_version(target_version, sw_type);

    if (version == "0") {
      // Original Micrel switch config
      print_backup_content_v0(config);
    } else if (version == "1") {
      // TI switch config
      print_backup_content_v1(config);
    } else if (version == "2") {
      print_backup_content_v2(config);
    } else if (version == "ignore") {
      // Do nothing; We ignore Marvell switch configs before 04.08.00(00), FW30.
    } else {
      exit_with_error(get_switch_config_error::internal_error, "Unknown config version");
    }
  } else {
    exit_with_error(get_switch_config_error::internal_error, s.to_string());
  }
}

void print_supported_values(const option_map& options, const switch_data_provider_i& provider) {
  nlohmann::json supported_values_json;
  ::wago::libswitchconfig::supported_values sv;

  libswitchconfig::status s = provider.get_supported_values(sv);

  if (s.ok()) {
    s = supported_values_to_json(supported_values_json, sv);
  } else {
    exit_with_error(get_switch_config_error::internal_error, s.to_string());
  }

  if (s.ok()) {
    print_json(supported_values_json, options.count(action_option::pretty) > 0);
  } else {
    exit_with_error(get_switch_config_error::internal_error, s.to_string());
  }
}

bool is_valid_fw_version(const ::std::string& version) {
  return std::regex_match(version, std::regex(R"(^[0-9]{2}\.[0-9]{2}\.[0-9]{2}\([0-9]{2}\))"));
}

}  // namespace wago::get_switch_config
