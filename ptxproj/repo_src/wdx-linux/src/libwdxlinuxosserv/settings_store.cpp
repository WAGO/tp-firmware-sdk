//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Settings store for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "settings_store.hpp"
#include "system_abstraction_serv.hpp"
#include "errno_utils.hpp"

#include <wc/log.h>
#include <wc/assertion.h>

#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cstring>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {

// getter for the default setting
bool setting_exists(std::string const &key);
std::string get_default(std::string const &key);

#define              CONFIG_FILE_PATH                                              "/etc/paramd/paramd.conf"
constexpr char const config_file_path[]                                          = CONFIG_FILE_PATH;
constexpr char const temp_file_path[]                                            = CONFIG_FILE_PATH ".tmp";
constexpr char const allowed_unauthenticated_requests_for_scan_devices_default[] = "true";
constexpr char const file_api_upload_id_timeout_default[]                        = "3600"; // seconds -> equal to 1 hour
constexpr char const run_result_timeout_default[]                                = "900"; // seconds -> equal to 15 minutes
constexpr char const oauth2_origin_default[]                                     = "http://localhost";
constexpr char const oauth2_token_path_default[]                                 = "/auth/token";
constexpr char const oauth2_verify_access_path_default[]                         = "/auth/verify";
constexpr char const oauth2_client_id_default[]                                  = "paramd";
constexpr char const oauth2_client_secret_default[]                              = "";

std::mutex config_file_mutex;
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
std::string settings_store::get_setting(std::string const &key) const
{
    if(!setting_exists(key))
    {
        throw std::out_of_range("Unknown key " + key);
    }

    std::string value;
    {
        std::lock_guard<std::mutex> lock_guard(config_file_mutex);

        auto file_stream = linuxos::sal::filesystem::get_instance().open_stream(config_file_path, std::ios_base::in);
        if(file_stream.get() == nullptr)
        {
            throw std::runtime_error("Failed to open config file \"" + std::string(config_file_path) + "\"");
        }
        for (std::string line; std::getline(*file_stream, line); )
        {
            line.erase(std::remove_if(line.begin(), line.end(), ::isblank), line.end());
            auto key_value_separator_position = line.find('=');
            if (   (key_value_separator_position != std::string::npos)
                && (key == line.substr(0, key_value_separator_position)))
            {
                value = line.substr(key_value_separator_position + 1);
                break;
            }
        }
    }

    // apply default and log info if config value is missing
    if (value.empty())
    {
        value = get_default(key);
        std::string info_message = "Key \"" + key + "\" not specified in config file \"" + std::string(config_file_path) + "\": Default value of \"" +
                                   value + "\". will be applied.";
        wc_log(info, info_message.c_str());
    }
    // apply default and log an error if config value is invalid
    // TODO: Validator?
    else if(key == allow_unauthenticated_requests_for_scan_devices 
           && (value != "false" && value != "true"))
    {
        value = get_default(key);
        std::string error_message = "Malformed config file \"" + std::string(config_file_path) + "\": value for " + key + 
                                    " should either be \"true\" or \"false\", was \"" + value + "\". Default value of \"" +
                                    value + "\". will be applied.";
        wc_log(error, error_message.c_str());
    }
    // apply default and log an error if config value is invalid
    else if(    (key == file_api_upload_id_timeout)
             || (key == run_result_timeout))
    {
        auto numeric_value = std::stoul(value);
        if (numeric_value > UINT16_MAX)
        {
            std::string error_message = "Malformed config file \"" + std::string(config_file_path) + "\": value for " + key + 
                                        " should be a 16-bit unsigned integer, was \"" + value + "\". Default value of \"" +
                                        value + "\". will be applied.";
            value = get_default(key);
            wc_log(error, error_message.c_str());
        }
    }
    return value;
}

void settings_store::write_setting(std::string const &key,
                                   std::string const &value)
{
    if(!setting_exists(key))
    {
        throw std::out_of_range("Unknown key " + key);
    }
    if(key == allow_unauthenticated_requests_for_scan_devices 
      && (value != "false" && value != "true"))
    {
        throw std::invalid_argument("Failed to write setting: value for " + key + " should either be \"true\" or \"false\".");
    }
    if(    (key == file_api_upload_id_timeout)
        || (key == run_result_timeout))
    {
        try {
            auto numeric_value = std::stoul(value);
            if (numeric_value > UINT16_MAX)
            {
                throw std::exception();
            }
        }
        catch(...)
        {
            throw std::invalid_argument("Failed to write setting: value for " + key + " should be 16-bit unsigned integer.");
        }
    }

    auto temp_file_stream = linuxos::sal::filesystem::get_instance().open_stream(temp_file_path, std::ios_base::out);
    if(temp_file_stream.get() == nullptr)
    {
        throw std::runtime_error("Failed to open temporary file to write \"" + std::string(temp_file_path) + "\"");
    }
    bool replaced_existing_setting = false;
    try
    {
        std::lock_guard<std::mutex> lock_guard(config_file_mutex);

        {
            auto file_stream = linuxos::sal::filesystem::get_instance().open_stream(config_file_path, std::ios_base::in);
            if(file_stream.get() == nullptr)
            {
                throw std::runtime_error("Failed to open config file \"" + std::string(config_file_path) + "\"");
            }
            for (std::string line; std::getline(*file_stream, line); )
            {
                std::string original_line = line;
                line.erase(std::remove_if(line.begin(), line.end(), ::isblank), line.end());
                auto key_value_separator_position = line.find('=');
                if (   (key_value_separator_position != std::string::npos)
                    && (key == line.substr(0, key_value_separator_position)))
                {
                    replaced_existing_setting = true;
                    *temp_file_stream << key << " = " << value << std::endl;
                }
                else
                {
                    *temp_file_stream << original_line << std::endl;
                }
            }

            // append non-existing key/value pair to config
            if (!replaced_existing_setting)
            {
                *temp_file_stream << key << " = " << value << std::endl;
            }
        }

        // move tmp file
        int status = linuxos::sal::filesystem::get_instance().rename(temp_file_path, config_file_path);
        if (status != 0)
        {
            throw std::runtime_error("Failed to write setting: unable to overwrite config file: " + errno_to_string(errno));
        }
    }
    catch(std::exception const &e)
    {
        linuxos::sal::filesystem::get_instance().unlink(temp_file_path);
        throw;
    }
}

namespace {

bool setting_exists(std::string const &key)
{
    return (key == settings_store::allow_unauthenticated_requests_for_scan_devices)
        || (key == settings_store::file_api_upload_id_timeout)
        || (key == settings_store::run_result_timeout)
        || (key == settings_store::oauth2_origin)
        || (key == settings_store::oauth2_token_path)
        || (key == settings_store::oauth2_verify_access_path)
        || (key == settings_store::oauth2_client_id)
        || (key == settings_store::oauth2_client_secret);
}

std::string get_default(std::string const &key)
{
    if (key == settings_store::allow_unauthenticated_requests_for_scan_devices)
    {
        return allowed_unauthenticated_requests_for_scan_devices_default;
    }
    else if (key == settings_store::file_api_upload_id_timeout)
    {
        return file_api_upload_id_timeout_default;
    }
    else if (key == settings_store::run_result_timeout)
    {
        return run_result_timeout_default;
    }
    else if (key == settings_store::oauth2_origin)
    {
        return oauth2_origin_default;
    }
    else if (key == settings_store::oauth2_token_path)
    {
        return oauth2_token_path_default;
    }
    else if (key == settings_store::oauth2_verify_access_path)
    {
        return oauth2_verify_access_path_default;
    }
    else if (key == settings_store::oauth2_client_id) {
        return oauth2_client_id_default;
    }
    else if (key == settings_store::oauth2_client_secret)
    {
        return oauth2_client_secret_default;
    }
    else
    {
        WC_FAIL("tried to get default for unknown setting");
        return "";
    }
}

}

} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
}  // Namespace wago

//---- End of source file ------------------------------------------------------
