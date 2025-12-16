//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO auth service settings store interface.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_SETTINGS_STORE_I_HPP_
#define INC_WAGO_AUTHSERV_SETTINGS_STORE_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"

#include <wc/std_includes.h>
#include <wc/structuring.h>
#include <wc/assertion.h>

#include <string>
#include <vector>
#include <map>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

#define AUTH_CODE_LIFETIME_DEFAULT                                                  60 // 60 seconds
#define ACCESS_TOKEN_LIFETIME_DEFAULT                                            28800 //  8 hours
#define REFRESH_TOKEN_LIFETIME_DEFAULT                                         2592000 // 30 days
#define SILENT_MODE_ENABLED_DEFAULT                                              false
WC_STATIC_ASSERT(ACCESS_TOKEN_LIFETIME_DEFAULT  >= (2 * AUTH_CODE_LIFETIME_DEFAULT));
WC_STATIC_ASSERT(REFRESH_TOKEN_LIFETIME_DEFAULT >= (2 * ACCESS_TOKEN_LIFETIME_DEFAULT));

class settings_store_i
{
    WC_INTERFACE_CLASS(settings_store_i)

public:
    enum global_setting{
        auth_code_lifetime,
        access_token_lifetime,
        refresh_token_lifetime,
        silent_mode_enabled,
        system_use_notification,
        AFTER_LAST
    };
    static constexpr char const* global_setting_names[] {
        "auth_code.lifetime",
        "access_token.lifetime",
        "refresh_token.lifetime",
        "silent_mode.enabled",
        "system_use_notification"
    };
    static constexpr char const* global_setting_defaults[] {
        WC_SUBST_STR(AUTH_CODE_LIFETIME_DEFAULT),
        WC_SUBST_STR(ACCESS_TOKEN_LIFETIME_DEFAULT),
        WC_SUBST_STR(REFRESH_TOKEN_LIFETIME_DEFAULT),
        WC_SUBST_STR(SILENT_MODE_ENABLED_DEFAULT),
        ""
    };
    WC_STATIC_ASSERT(WC_ARRAY_LENGTH(global_setting_names) == WC_ARRAY_LENGTH(global_setting_defaults));
    WC_STATIC_ASSERT(WC_ARRAY_LENGTH(global_setting_names) == global_setting::AFTER_LAST);

    static constexpr uint64_t auth_code_lifetime_default = AUTH_CODE_LIFETIME_DEFAULT;
    static constexpr uint64_t access_token_lifetime_default = ACCESS_TOKEN_LIFETIME_DEFAULT;
    static constexpr uint64_t refresh_token_lifetime_default = REFRESH_TOKEN_LIFETIME_DEFAULT;
    static constexpr bool     silent_mode_enabled_default    = SILENT_MODE_ENABLED_DEFAULT;

    struct oauth_client
    {
        const std::string id;
        const std::string label;
        const std::string redirect_uri;
        const std::vector<std::string> grant_types;
        bool supports_grant_type(std::string const&) const;
    };
    static constexpr char const client_label_name[]           = "label";
    static constexpr char const client_redirect_uri_name[]    = "redirect_uri";
    static constexpr char const client_grant_types_name[]     = "grant_types";

    struct oauth_resource_server
    {
        const std::string id;
        const std::string label;
        const std::vector<std::string> scopes;
    };
    static constexpr char const resource_server_label_name[]  = "label";
    static constexpr char const resource_server_scopes_name[] = "scopes";

    using global_config_map = std::map<settings_store_i::global_setting, std::string>;
    using configuration_error_map = std::map<settings_store_i::global_setting, std::string>;

public:
    /// Load the configuration from file system.
    virtual void reload_config() noexcept = 0;

    /// Get a global settings value by key.
    ///
    /// \throw
    ///   std::runtime_error if value can not be discovered (e.g. config not available).
    ///
    /// \param key
    ///   Key to read value from.
    ///
    /// \return
    ///   Current settings value for given key.
    virtual std::string const &get_global_setting(global_setting key) const = 0;

    /// Validates config and sets after successful validation
    ///
    /// \param new_config
    ///     new configuration values
    /// \param error_message_out
    ///     short message describing each error
    /// \return
    ///     whether the config was set successful (true) or rejected completely (false)
    virtual bool set_global_config(global_config_map const &new_values,
                                   configuration_error_map &error_message_out) = 0;

    /// Check if a client id exists
    ///
    /// \param client_id
    ///   id of client to check for existence
    ///
    /// \return
    ///   true, if the client exists
    virtual bool client_exists(const std::string &client_id) const = 0;

    /// Get details about a client
    ///
    /// \throw
    ///   std::runtime_error if id isn't a configured client.
    ///
    /// \param client_id
    ///   id of client to return.
    ///
    /// \return
    ///   client config object
    virtual oauth_client const &get_client(const std::string &client_id) const = 0;

    /// Get a list of all scopes configured for the resource servers.
    ///
    /// \return
    ///   vector of all scopes
    virtual std::vector<std::string> const &get_all_scopes() const = 0;
};

#undef AUTH_CODE_LIFETIME_DEFAULT
#undef ACCESS_TOKEN_LIFETIME_DEFAULT
#undef REFRESH_TOKEN_LIFETIME_DEFAULT
#undef SILENT_MODE_ENABLED_DEFAULT

} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_SETTINGS_STORE_I_HPP_
//---- End of source file ------------------------------------------------------
