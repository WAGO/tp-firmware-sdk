//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Settings store for WAGO auth service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "settings_store.hpp"
#include "system_abstraction.hpp"
#include "utils/vector_operations.hpp"
#include "errno_utils.hpp"
#include "definitions.hpp"

#include <wc/log.h>
#include <wc/preprocessing.h>

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {

settings_store_i::global_setting find_global_setting(std::string const &key_name);
char const *get_global_name(settings_store_i::global_setting key);
bool validate_global_config(settings_store_i::global_config_map &config_settings_in_out,
                            settings_store_i::configuration_error_map &error_messages_out,
                            bool correct_wrong_values = false);

#define              CONFIG_FOLDER_PATH                    "/etc/authd"
#define              CONFIG_FILE_NAME                      "authd.conf"
#define              RESOURCESERVER_CONFIG_FOLDER          "resource_servers"
#define              CLIENT_CONFIG_FOLDER                  "clients"
constexpr char const main_config_file_path[]               = CONFIG_FOLDER_PATH "/" CONFIG_FILE_NAME;
constexpr char const resource_servers_config_folder_path[] = CONFIG_FOLDER_PATH "/" RESOURCESERVER_CONFIG_FOLDER;
constexpr char const clients_config_folder_path[]          = CONFIG_FOLDER_PATH "/" CLIENT_CONFIG_FOLDER;
#undef               CONFIG_FOLDER_PATH
#undef               CONFIG_FILE_NAME
#undef               RESOURCESERVER_CONFIG_FOLDER
#undef               CLIENT_CONFIG_FOLDER

std::mutex load_config_mutex; //noexcept constructor
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

settings_store::settings_store() :
    comments_regex(
        "(#.*)", std::regex::optimize
    ),
    whitespace_regex(
        "(^\\s+)|(\\s+$)", std::regex::optimize
    ),
    configfile_regex(
        "^[A-Za-z0-9]*\\.conf$", std::regex::optimize
    ),
    configfile_editor_regex (
        "^~[A-Za-z0-9]*\\.conf$", std::regex::optimize
    ),
    configfile_path_extract_id_regex (
        "(^.*/)|(\\.conf$)", std::regex::optimize
    )
{
    std::lock_guard<std::mutex> lock_guard(load_config_mutex);
    load_config(false);
}

settings_store::~settings_store() noexcept = default;

void settings_store::reload_config() noexcept
{
    global_config_map global_settings_backup;
    std::vector<oauth_client> clients_backup;
    std::vector<oauth_resource_server> resource_servers_backup;
    std::vector<std::string> scopes_backup;
    std::lock_guard<std::mutex> lock_guard(load_config_mutex);

    {
        std::lock_guard<std::mutex> lock_guard_global_settings(global_settings_mutex);
        global_settings_cache.swap(global_settings_backup);
    }
    client_list.swap(clients_backup);
    resource_server_list.swap(resource_servers_backup);
    scopes_list.swap(scopes_backup);

    try
    {
        load_config();
    }
    catch (std::exception const &ex)
    {
        wc_log(log_level_t::fatal, "Can't load new config (still using old values from memory): " + std::string(ex.what()));
        //revert
        {
            std::lock_guard<std::mutex> lock_guard_global_settings(global_settings_mutex);
            global_settings_cache.swap(global_settings_backup);
        }
        client_list.swap(clients_backup);
        resource_server_list.swap(resource_servers_backup);
        scopes_list.swap(scopes_backup);
    }

}

void settings_store::load_config(bool strict_errors)
{
    load_global_config(strict_errors);
    // load client configs
    parse_all_files_to_array(client_list, clients_config_folder_path, &settings_store::create_client, strict_errors);
    // load resource server configs
    parse_all_files_to_array(resource_server_list, resource_servers_config_folder_path, &settings_store::create_resource_server, strict_errors);

    // cache all scopes from resource servers
    for (settings_store_i::oauth_resource_server const &resource_server: resource_server_list)
    {
        if(resource_server.scopes.empty())
        {
            scopes_list.push_back(resource_server.id);
        }
        for(std::string const &scope: resource_server.scopes)
        {
            scopes_list.push_back(resource_server.id + ":" + scope);
        }
    }
}

void settings_store::load_global_config(bool strict_errors)
{
    global_config_map result;
    // load main config
    try
    {
        auto file_stream = sal::filesystem::get_instance().open_stream(main_config_file_path, std::ios_base::in);
        for(std::string line; get_setting_line(*file_stream, line); )
        {
            std::string key, value;
            split_key_value(line, key, value);
            try
            {
                // preload config values
                result.emplace(find_global_setting(key), value);
            }
            catch (std::invalid_argument const &) // thrown by find_global_setting when the key is invalid
            {
                wc_log(log_level_t::warning, "Unknown setting \"" + key + "\" in config file (" + main_config_file_path + ")");
            }
        }
    }
    catch (std::runtime_error &err) // file not accessible
    {
        std::string const message = std::string("Couldn't open config \"") + main_config_file_path + "\": " + err.what();
        if (strict_errors) throw std::runtime_error(message);
        wc_log(log_level_t::error, message);
    }

    // apply default and log info if config value is missing
    for(size_t i = 0; i < WC_ARRAY_LENGTH(global_setting_defaults); ++i)
    {
        auto setting = static_cast<global_setting>(i); //parasoft-suppress CERT_CPP-INT50-a-3 "Enum values of status codes are explicitly designed to be searched. Values are defined without gap and with an explicit end marker."
        if (result.count(setting) == 0)
        {
            result.emplace(setting, global_setting_defaults[i]);
            std::string info_message = std::string("Config value for \"") + get_global_name(setting) + "\" not specified in file \"" + main_config_file_path + "\". "
                                          "Using default value \"" + global_setting_defaults[i] + "\" instead.";
            wc_log(log_level_t::info, info_message);
        }
    }

    configuration_error_map error_messages;
    validate_global_config(result, error_messages, !strict_errors);
    if (!error_messages.empty())
    {
        std::string error_message;
        for(auto const &entry: error_messages)
        {
            error_message += std::string(global_setting_names[entry.first]) + " has an invalid value: " + entry.second + "\n";
        }
        if (strict_errors) { throw std::runtime_error(error_message); }
        wc_log(log_level_t::warning, error_message);
    }
    {
        std::lock_guard<std::mutex> lock_guard(global_settings_mutex);
        global_settings_cache = result;
    }
}

bool settings_store::set_global_config(global_config_map const &new_values, configuration_error_map &error_message_out)
{
    global_config_map new_config = new_values;
    // insert old values for missing keys
    new_config.insert(global_settings_cache.begin(), global_settings_cache.end());

    if (validate_global_config(new_config, error_message_out))
    {
        for (auto const &entry: new_config)
        {
            write_global_setting(entry.first, entry.second);
        }
        return true;
    }
    return false;
}


std::string const &settings_store::get_global_setting(global_setting key) const
{
    WC_ASSERT(key >= 0 && key < WC_ARRAY_LENGTH(settings_store_i::global_setting_names));
    std::lock_guard<std::mutex> lock_guard(global_settings_mutex);
    return global_settings_cache.at(key);
}

void settings_store::write_global_setting(global_setting key,
                                          std::string const &value)
{
    auto key_name = get_global_name(key); // checks if argument is valid
    auto main_config_file_temp_path = std::string(main_config_file_path) + ".tmp";

    auto &sal_filesystem = sal::filesystem::get_instance();
    auto temp_file_stream = sal_filesystem.open_stream(main_config_file_temp_path, std::ios_base::out);
    bool replaced_existing_setting = false;
    try
    {
        auto file_stream = sal_filesystem.open_stream(main_config_file_path, std::ios_base::in);
        for(std::string line; get_setting_line(*file_stream, line); )
        {
            std::string original_line = line;
            line.erase(std::remove_if(line.begin(), line.end(), ::isblank), line.end());
            auto key_value_separator_position = line.find('=');
            if(    (key_value_separator_position != std::string::npos)
                && (key_name == line.substr(0, key_value_separator_position)))
            {
                replaced_existing_setting = true;
                *temp_file_stream << key_name << " = " << value << std::endl;
            }
            else
            {
                *temp_file_stream << original_line << std::endl;
            }
        }

        // append non-existing key/value pair to config
        if(!replaced_existing_setting)
        {
            *temp_file_stream << key_name << " = " << value << std::endl;
        }

        // move tmp file
        int status = sal_filesystem.rename(main_config_file_temp_path.c_str(), main_config_file_path);
        if(status != 0)
        {
            throw std::runtime_error("Failed to write setting: unable to overwrite config file: " + errno_to_string(errno));
        }

        // update cache on success
        std::lock_guard<std::mutex> lock_guard(global_settings_mutex);
        global_settings_cache[key] = value;
    }
    catch(std::exception const &e)
    {
        sal_filesystem.unlink(main_config_file_temp_path.c_str());
        throw;
    }
}

bool settings_store::client_exists(const std::string &client_id) const
{
    std::lock_guard<std::mutex> lock_guard(load_config_mutex);
    for (const oauth_client &client: client_list)
    {
        if (client.id == client_id)
        {
            return true;
        }
    }
    return false;
}

settings_store_i::oauth_client const& settings_store::get_client(const std::string &client_id) const
{
    std::lock_guard<std::mutex> lock_guard(load_config_mutex);
    for (const oauth_client &client: client_list)
    {
        if (client.id == client_id)
        {
            return client;
        }
    }
    throw std::out_of_range("Client with id \"" + client_id + "\" isn't registered");
}

std::vector<std::string> const& settings_store::get_all_scopes() const
{
    return scopes_list;
}

template<class T>
void settings_store::parse_all_files_to_array(std::vector<T>    &array,
                                              std::string const &directory_path,
                                              T (settings_store::*converter)(std::string const&),
                                              bool strict_errors)
{
    try
    {
        auto &sal_filesystem = sal::filesystem::get_instance();
        for (std::string const &dir_entry : sal_filesystem.list_directory(directory_path))
        {
            if (sal_filesystem.is_regular_file(directory_path + "/" + dir_entry))
            {
                if (std::regex_match(dir_entry, configfile_regex))
                {
                    try
                    {
                        array.push_back((this->*converter)(directory_path + "/" + dir_entry));
                    }
                    catch (std::exception &err) // file not accessible
                    {
                        wc_log(log_level_t::warning, "Error reading file \"" + directory_path + "/" + dir_entry + "\": " + err.what());
                    }
                }
                // Ignore usual editor temp and backup files, but warn about other invalid config file names
                else if (!std::regex_match(dir_entry, configfile_editor_regex))
                {
                    wc_log(log_level_t::warning, "Found file \"" + dir_entry + "\" which isn't a valid config file name in \"" + directory_path + "\"");
                }
            }
        }
    }
    catch (std::runtime_error &err)
    {
        std::string const message = "Couldn't open directory: " + directory_path + ": " + err.what();
        if (strict_errors) throw std::runtime_error(message);
        wc_log(log_level_t::error, message);
    }
}

settings_store_i::oauth_client settings_store::create_client(std::string const &config_file_path)
{
    std::string id(std::regex_replace(config_file_path, configfile_path_extract_id_regex, ""));
    // defaults
    std::string label(id), redirect_uri;
    std::vector<std::string> grant_types;

    auto file_stream = sal::filesystem::get_instance().open_stream(config_file_path, std::ios_base::in);
    for(std::string line; get_setting_line(*file_stream, line); )
    {
        std::string key, value;
        split_key_value(line, key, value);
        if(key == client_label_name)
        {
            label = value;
        }
        else if (key == client_redirect_uri_name)
        {
            redirect_uri = value;
        }
        else if (key == client_grant_types_name)
        {
            grant_types = split_string(value, ';');
        }
        else
        {
            std::string const message = "Unknown setting \"" + key + "\" in config file (" + config_file_path + ")";
            wc_log(log_level_t::warning, message);
        }
    }

    if (std::find(grant_types.begin(), grant_types.end(), grant_type_code) != grant_types.end() && redirect_uri == "") {
        throw std::runtime_error("Invalid client config \"" + config_file_path + "\": Parameter \"redirect_uri\" must be provided for grant type \"" + grant_type_code + "\".");
    }

    return settings_store_i::oauth_client{.id=id, .label=label, .redirect_uri=redirect_uri, .grant_types=grant_types};
}

settings_store_i::oauth_resource_server settings_store::create_resource_server(std::string const &config_file_path)
{
    std::string id(std::regex_replace(config_file_path, configfile_path_extract_id_regex, ""));
    // defaults
    std::string label;
    std::vector<std::string> scopes;

    auto file_stream = sal::filesystem::get_instance().open_stream(config_file_path, std::ios_base::in);
    for(std::string line; get_setting_line(*file_stream, line); )
    {
        std::string key, value;
        split_key_value(line, key, value);
        if(key == resource_server_label_name)
        {
            label = value;
        }
        else if (key == resource_server_scopes_name)
        {
            scopes = split_string(value, ';');
        }
        else
        {
            std::string const message = "Unknown setting \"" + key + "\" in config file (" + config_file_path + ")";
            wc_log(log_level_t::warning, message);
        }
    }

    return settings_store_i::oauth_resource_server{.id=id, .label=label, .scopes=scopes};
}

bool settings_store::get_setting_line(std::iostream &stream,
                                      std::string   &line)
{
    char line_buf[1024*4]; //Corresponds to the limit of request header size in lighttpd
    while(stream.getline(line_buf, sizeof line_buf))
    {
        // ignore comments
        line = std::regex_replace(line_buf, comments_regex, "");

        // remove whitespace at beginning or end
        line = std::regex_replace(line, whitespace_regex, "");

        if(!line.empty())
        {
            return true;
        }
    }

    return false;
}

void settings_store::split_key_value(std::string const &config_line,
                                     std::string       &key,
                                     std::string       &value)
{
    auto key_value_separator_position = config_line.find('=');

    key   = config_line.substr(0, key_value_separator_position);
    value = config_line.substr(key_value_separator_position + 1);

    // remove whitespace at beginning or end
    key   = std::regex_replace(key,   whitespace_regex, "");
    value = std::regex_replace(value, whitespace_regex, "");
}

namespace {

inline bool string_to_bool(const std::string& str)
{
    bool b;
    std::istringstream is(str);
    is >> std::boolalpha >> b;
    return b;
}

bool validate_global_config(settings_store_i::global_config_map &config_settings_in_out,
                            settings_store_i::configuration_error_map &error_messages_out,
                            bool correct_wrong_values)
{
    // Validate some values
    uint64_t auth_code_lifetime_s;
    uint64_t access_token_lifetime_s;
    uint64_t refresh_token_lifetime_s;
    bool     silent_mode_enabled;

    try
    {
        std::size_t pos = 0;
        auto auth_code_lifetime_value = config_settings_in_out.at(settings_store::auth_code_lifetime);
        auth_code_lifetime_s = std::stoull(auth_code_lifetime_value, &pos);
        if(pos != auth_code_lifetime_value.length())
        {
            throw std::runtime_error("Invalid value for setting 'auth_code_lifetime'");
        }
        else if(auth_code_lifetime_s > std::numeric_limits<uint32_t>::max())
        {
            throw std::runtime_error("Exceeds maximum value of " + std::to_string(std::numeric_limits<uint32_t>::max()));
        }
    }
    catch(std::exception const &e)
    {
        error_messages_out.emplace(settings_store::auth_code_lifetime, std::string(e.what()));
        if (correct_wrong_values) { auth_code_lifetime_s = settings_store_i::auth_code_lifetime_default; }
    }
    try
    {
        std::size_t pos = 0;
        auto access_token_lifetime_value = config_settings_in_out.at(settings_store::access_token_lifetime);
        access_token_lifetime_s = std::stoull(access_token_lifetime_value, &pos);
        if(pos != access_token_lifetime_value.length())
        {
            throw std::runtime_error("Invalid value for setting 'access_token_lifetime'");
        }
        else if(access_token_lifetime_s > std::numeric_limits<uint32_t>::max())
        {
            throw std::runtime_error("Exceeds maximum value of " + std::to_string(std::numeric_limits<uint32_t>::max()));
        }
    }
    catch(std::exception const &e)
    {
        error_messages_out.emplace(settings_store::access_token_lifetime, std::string(e.what()));
        if (correct_wrong_values) { access_token_lifetime_s = settings_store_i::access_token_lifetime_default; }
    }
    try
    {
        std::size_t pos = 0;
        auto refresh_token_lifetime_value = config_settings_in_out.at(settings_store::refresh_token_lifetime);
        refresh_token_lifetime_s = std::stoull(refresh_token_lifetime_value, &pos);
        if(pos != refresh_token_lifetime_value.length())
        {
            throw std::runtime_error("Invalid value for setting 'refresh_token_lifetime'");
        }
        else if(refresh_token_lifetime_s > std::numeric_limits<uint32_t>::max())
        {
            throw std::runtime_error("Exceeds maximum value of " + std::to_string(std::numeric_limits<uint32_t>::max()));
        }
    }
    catch(std::exception const &e)
    {
        error_messages_out.emplace(settings_store::refresh_token_lifetime, std::string(e.what()));
        if (correct_wrong_values) { refresh_token_lifetime_s = settings_store_i::refresh_token_lifetime_default; }
    }
    try
    {
        auto silent_mode_enabled_value = config_settings_in_out.at(settings_store::silent_mode_enabled);
        if(!(silent_mode_enabled_value == "true" || silent_mode_enabled_value == "false"))
        {
            throw std::runtime_error("Invalid value for setting 'silent_mode.enabled'");
        }
        silent_mode_enabled = string_to_bool(silent_mode_enabled_value);
    }
    catch(std::exception const &e)
    {
        error_messages_out.emplace(settings_store::silent_mode_enabled, std::string(e.what()));
        if (correct_wrong_values) { silent_mode_enabled = settings_store_i::silent_mode_enabled_default; }
    }

    if (!correct_wrong_values && !error_messages_out.empty()) { return false; }

    if(auth_code_lifetime_s < 1) {
        error_messages_out.emplace(settings_store::auth_code_lifetime,
                                   std::string("Has to be at least 1"));
        if (correct_wrong_values) {
            auth_code_lifetime_s = settings_store_i::auth_code_lifetime_default;
        }
    }
    if(access_token_lifetime_s < (2 * auth_code_lifetime_s))
    {
        error_messages_out.emplace(settings_store::auth_code_lifetime,
                                   std::string("Has to be 0.5x '") + get_global_name(settings_store::access_token_lifetime) + "' at most");
        error_messages_out.emplace(settings_store::access_token_lifetime,
                                   std::string("Has to be 2x '") + get_global_name(settings_store::auth_code_lifetime) + "' at least");
        if (correct_wrong_values) {
            auth_code_lifetime_s = settings_store_i::auth_code_lifetime_default;
            access_token_lifetime_s = settings_store_i::access_token_lifetime_default;
            refresh_token_lifetime_s = settings_store_i::refresh_token_lifetime_default;
        }
    }
    if(refresh_token_lifetime_s < (2 * access_token_lifetime_s))
    {
        error_messages_out.emplace(settings_store::access_token_lifetime,
                                   std::string("Has to be 0.5x '") + get_global_name(settings_store::refresh_token_lifetime) + "' at most");
        error_messages_out.emplace(settings_store::refresh_token_lifetime,
                                   std::string("Has to be 2x '") + get_global_name(settings_store::access_token_lifetime) + "' at least");
        if (correct_wrong_values) {
            auth_code_lifetime_s = settings_store_i::auth_code_lifetime_default;
            access_token_lifetime_s = settings_store_i::access_token_lifetime_default;
            refresh_token_lifetime_s = settings_store_i::refresh_token_lifetime_default;
        }
    }

    if (!correct_wrong_values && !error_messages_out.empty()) { return false; }
    config_settings_in_out[settings_store::auth_code_lifetime] = std::to_string(auth_code_lifetime_s);
    config_settings_in_out[settings_store::access_token_lifetime] = std::to_string(access_token_lifetime_s);
    config_settings_in_out[settings_store::refresh_token_lifetime] = std::to_string(refresh_token_lifetime_s);
    config_settings_in_out[settings_store::silent_mode_enabled] = silent_mode_enabled ? "true" : "false";
    return true;
}

settings_store_i::global_setting find_global_setting(std::string const &key_name)
{
    int index = 0;
    for(const char* key_candidate: settings_store_i::global_setting_names)
    {
        if (std::string(key_candidate) == key_name)
        {
            // should be in range as the array and the enum have the same amount of entries
            return static_cast<settings_store_i::global_setting>(index); //parasoft-suppress CERT_CPP-INT50-a-3 "Enum values of status codes are explicitly designed to be searched. Values are defined without gap and with an explicit end marker."
        }
        index++;
    }
    throw std::invalid_argument(key_name + " is an invalid key");
}

char const *get_global_name(settings_store_i::global_setting key)
{
    bool check_args = key >= 0 && key < WC_ARRAY_LENGTH(settings_store_i::global_setting_names);
    WC_ASSERT(check_args);
    return check_args ? settings_store_i::global_setting_names[key] : throw std::runtime_error("Out-of-bounds access in get_global_name().");
}
}

}  // Namespace authserv
}  // Namespace wago

//---- End of source file ------------------------------------------------------
