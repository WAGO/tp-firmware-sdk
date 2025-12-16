// Copyright (c) 2024 WAGO GmbH & Co. KG
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <functional>
#include <nlohmann/json_fwd.hpp>

#include "switch_config_api.hpp"
#include "switch_data_provider_i.hpp"

namespace wago::get_switch_config {

enum class get_switch_config_error { parameter_parse_error = 1, internal_error = 2 };

enum class action { json_config, backup, help, supported_values };

enum class action_option { pretty, targetversion };

using parameter_list = ::std::vector<action>;
using option_map     = ::std::map<action_option, ::std::string>;
using action_map     = ::std::map<action, ::std::function<void(const option_map&, const switch_data_provider_i&)>>;

::wago::libswitchconfig::status get_switch_config_as_json(const ::wago::libswitchconfig::switch_config& c,
                                                          ::nlohmann::json& j);
::wago::libswitchconfig::status parse_commandline_args(const ::std::vector<::std::string>& args,
                                                       parameter_list& parameters, option_map& options);
::wago::libswitchconfig::status execute_action(const parameter_list& parameters, const option_map& options,
                                               const switch_data_provider_i& provider);
[[noreturn]] void exit_with_error(get_switch_config_error code, ::std::string const& text);
void clear_last_error();
void print_backup_content(const option_map& options, const switch_data_provider_i& provider);
void print_supported_values(const option_map& options, const switch_data_provider_i& provider);
void generate_output(const ::std::function<void(const ::nlohmann::json& j)>& func);
void print_json_config(const wago::get_switch_config::option_map& options,
                       const wago::get_switch_config::switch_data_provider_i& provider);
bool is_valid_fw_version(const ::std::string& version);

}  // namespace wago::get_switch_config
