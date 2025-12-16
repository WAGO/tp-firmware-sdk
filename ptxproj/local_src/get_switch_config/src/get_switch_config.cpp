// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0


#include <nlohmann/json.hpp>
#include <switch_config_output.hpp>
#include <switch_data_provider.hpp>

int main(int argc, const char* argv[]) {
  ::std::vector<::std::string> args(argv + 1, argv + argc);  // Ignore the program name.
  wago::get_switch_config::parameter_list parameters;
  wago::get_switch_config::option_map options;

  wago::get_switch_config::clear_last_error();

  ::wago::libswitchconfig::status s = parse_commandline_args(args, parameters, options);

  if (s.ok()) {
    wago::get_switch_config::switch_data_provider data_provider{};
    s = execute_action(parameters, options, data_provider);
  } else {
    exit_with_error(wago::get_switch_config::get_switch_config_error::parameter_parse_error, s.to_string());
  }

  if (!s.ok()) {
    exit_with_error(wago::get_switch_config::get_switch_config_error::parameter_parse_error, s.to_string());
  }

  return 0;
}