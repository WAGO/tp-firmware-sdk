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
#ifndef SRC_LIBAUTHSERV_SETTINGS_STORE_HPP_
#define SRC_LIBAUTHSERV_SETTINGS_STORE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/settings_store_i.hpp"

#include <mutex>
#include <memory>
#include <regex>

#include<wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class settings_store : public authserv::settings_store_i
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(settings_store)
public:
    settings_store();
    ~settings_store() noexcept override;
    void reload_config() noexcept override;

    std::string const &get_global_setting(global_setting key) const override;
    bool client_exists(const std::string &client_id) const override;
    oauth_client const &get_client(const std::string &client_id) const override;
    std::vector<std::string> const &get_all_scopes() const override;
    bool set_global_config(global_config_map const &new_values,
                           configuration_error_map &error_message_out) override;

private:
    mutable std::mutex global_settings_mutex;
    global_config_map global_settings_cache;

    std::vector<oauth_client> client_list;
    std::vector<oauth_resource_server> resource_server_list;
    std::vector<std::string> scopes_list;
    void load_config(bool strict_errors = true);
    void load_global_config(bool strict_errors);
    void write_global_setting(global_setting        key,
                              std::string    const &value);

    template<class T>
    void parse_all_files_to_array(std::vector<T> &array,
                                  std::string const &directory_path,
                                  T (settings_store::*converter)(std::string const &file_path),
                                  bool strict_errors);

    oauth_client create_client(std::string const &config_file_path);
    oauth_resource_server create_resource_server(std::string const &config_file_path);
    bool get_setting_line(std::iostream &stream,
                          std::string   &line);
    void split_key_value(std::string const &config_line,
                         std::string       &key,
                         std::string       &value);

    std::regex comments_regex;
    std::regex whitespace_regex;
    std::regex configfile_regex;
    std::regex configfile_editor_regex;
    std::regex configfile_path_extract_id_regex;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_SETTINGS_STORE_HPP_
//---- End of source file ------------------------------------------------------
