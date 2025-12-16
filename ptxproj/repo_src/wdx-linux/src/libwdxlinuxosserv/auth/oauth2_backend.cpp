//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    OAuth2 authentication backend for JSON Web Token.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "oauth2_backend.hpp"
#include "oauth2_client.hpp"
#include "oauth2_resource_provider.hpp"
#include "token_handler.hpp"
#include "system_abstraction_serv.hpp"
#include "errno_utils.hpp"

#include <wago/wdx/wda/exception.hpp>
#include <wago/wdx/wda/settings_store_i.hpp>
#include <wc/preprocessing.h>
#include <wc/std_includes.h>
#include <wc/assertion.h>
#include <wc/log.h>

#include <thread>
#include <chrono>
#include <functional>
#include <map>
#include <cstring>
#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace auth {

using backend_impl = oauth2_backend::backend_impl;
using token_result = oauth2_client::token_result;

constexpr char     const oauth_provider_scope[] = "wda";
constexpr char     const wdx_token_prefix[]     = "wdx$";
// '$' is not allowed in OAuth 2 token (https://www.rfc-editor.org/rfc/rfc6750#section-2.1)
// but allowed in HTTP Authorization header in general (https://www.rfc-editor.org/rfc/rfc2616#page-17)
// in this way it is used as unambiguous marker for wdx tokens.

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {
void log_and_throw_errno_error(char const * const error_pre_text);
std::string build_token_data(uint32_t    const  lifetime,
                             std::string const &access_token,
                             std::string const &refresh_token,
                             std::string const &user_name);
uint32_t get_min_lifetime(uint32_t const access_token_expires_in);
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
class oauth2_backend::backend_impl
{
public:
    struct cache_data
    {
        time_t      entry_expiration;
        time_t      token_expiration;
        uint32_t    token_expires_in;
        std::string user_name;
    };
    using token_cache_map = std::map<std::string, cache_data>;

public:
    oauth2_client                  client_m;
    oauth2_resource_provider       provider_m;
    token_handler                  token_handler_m;
    size_t                   const broken_token_slowdown_m;
    token_cache_map                token_cache_m;

    explicit
    backend_impl(std::shared_ptr<wdx::wda::settings_store_i> settings_store, size_t broken_token_slowdown)
    : client_m(settings_store->get_setting(wdx::wda::settings_store_i::oauth2_client_id),
               settings_store->get_setting(wdx::wda::settings_store_i::oauth2_origin),
               settings_store->get_setting(wdx::wda::settings_store_i::oauth2_token_path),
               settings_store->get_setting(wdx::wda::settings_store_i::oauth2_verify_access_path),
               settings_store->get_setting(wdx::wda::settings_store_i::oauth2_client_secret))
    , provider_m(settings_store->get_setting(wdx::wda::settings_store_i::oauth2_client_id),
                 oauth_provider_scope,
                 settings_store->get_setting(wdx::wda::settings_store_i::oauth2_origin),
                 settings_store->get_setting(wdx::wda::settings_store_i::oauth2_verify_access_path),
                 settings_store->get_setting(wdx::wda::settings_store_i::oauth2_client_secret))
    , token_handler_m(token_default_lifetime)
    , broken_token_slowdown_m(broken_token_slowdown)
    { }

    void update_token_cache();
    void add_wdx_token(auth_result       &result,
                       uint32_t    const  expires_in,
                       std::string const &access_token,
                       std::string const &refresh_token,
                       std::string const &user_name);
    auth_result verify_wdx_token(std::string    const &token,
                                 uint32_t     * const  remaining_time);
};

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
oauth2_backend::oauth2_backend(std::shared_ptr<wdx::wda::settings_store_i> settings_store, size_t const broken_token_slowdown)
: impl(std::make_unique<backend_impl>(settings_store, broken_token_slowdown))
{ }

oauth2_backend::~oauth2_backend() noexcept = default;

auth_result oauth2_backend::authenticate(std::string const &user,
                                         std::string const &password)
{
    impl->update_token_cache();
    auth_result result;
    result.user_name = user;

    // Authenticate & authorize user in role of a client to access providers own resources
    WC_DEBUG_LOG("Try to authenticate via username & password credentials");
    auto grant_result = impl->client_m.resource_owner_password_credentials_grant(oauth_provider_scope, user, password);
    result.success = grant_result.success;
    result.expired = grant_result.expired;

    if(result.success)
    {
        // Add WDx token for future access
        impl->add_wdx_token(result, grant_result.expires_in, grant_result.access_token, grant_result.refresh_token, user);
    }

    return result;
}

auth_result oauth2_backend::authenticate(std::string   const &token,
                                         uint32_t    * const  remaining_time)
{
    impl->update_token_cache();
    auth_result result;

    if(    (token.length() >= (WC_ARRAY_LENGTH(wdx_token_prefix) - 1))
        && (token.substr(0, WC_ARRAY_LENGTH(wdx_token_prefix) - 1) == wdx_token_prefix))
    {
        // Check token as WDx token
        WC_DEBUG_LOG("Try to authenticate via WDx token");
        result = impl->verify_wdx_token(token, remaining_time);
    }
    else
    {
        // Check token as OAuth 2 access token
        WC_DEBUG_LOG("Try to authenticate via OAuth 2 access token");
        uint32_t expires_in = 0;

        // Try cache first to avoid frequent HTTP communication
        auto cached_entry = impl->token_cache_m.find(token);
        if(cached_entry != impl->token_cache_m.end())
        {
            wc_log(log_level_t::info, "Found valid token in cache: Grant access");
            result.success   = true;
            expires_in       = cached_entry->second.token_expires_in;
            result.user_name = cached_entry->second.user_name;
        }
        else
        {
            WC_DEBUG_LOG("Verify token on OAuth service");
            auto oauth_result = impl->provider_m.access_token_verification(token);
            result.success = oauth_result.active;
            if(result.success)
            {
                expires_in = oauth_result.expires_in;
                result.user_name = oauth_result.username;
                timespec mono_time;
                sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);
                auto const cache_time = (oauth_result.expires_in < oauth_token_cache_time) ?
                                            oauth_result.expires_in : oauth_token_cache_time;
                time_t const cache_expiration = mono_time.tv_sec + static_cast<time_t>(cache_time);
                time_t const token_expiration = mono_time.tv_sec + static_cast<time_t>(oauth_result.expires_in);
                if((cache_expiration > mono_time.tv_sec) && (token_expiration > mono_time.tv_sec))
                {
                    WC_DEBUG_LOG(("Add successfully verified token to cache for " + std::to_string(cache_time) + " s").c_str());
                    impl->token_cache_m.emplace(token, backend_impl::cache_data({ cache_expiration, token_expiration, oauth_result.expires_in, oauth_result.username }));
                }
            }
        }
        if(remaining_time != nullptr)
        {
            *remaining_time = expires_in;
        }
    }

    // Slow down unauthenticated requests with wrong/broken tokens for security reasons if requested
    if(!result.success && (impl->broken_token_slowdown_m > 0))
    {
        std::this_thread::sleep_for(std::chrono::seconds(impl->broken_token_slowdown_m));
    }

    return result;
}

void oauth2_backend::backend_impl::update_token_cache()
{
    if(token_cache_m.size() > 0)
    {
        timespec mono_time;
        sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);

        auto iter = token_cache_m.begin();
        while(iter != token_cache_m.end())
        {
            if(iter->second.entry_expiration < mono_time.tv_sec)
            {
                WC_DEBUG_LOG(("Remove token from cache: " + iter->first).c_str());
                iter = token_cache_m.erase(iter);
            }
            else
            {
                WC_DEBUG_LOG(("Update expiration time for token in cache: " + iter->first).c_str());
                auto const expiration = iter->second.token_expiration;

                // Duration expires_in was a uint32_t value when putting expiration value into cache
                // Duration could only be greater than uint32_t max value if time is running backwards
                WC_ASSERT(std::numeric_limits<uint32_t>::max() >= static_cast<uint64_t>(expiration - mono_time.tv_sec));
                iter->second.token_expires_in = static_cast<uint32_t>(expiration - mono_time.tv_sec);
                ++iter;
            }
        }
    }
}

void oauth2_backend::backend_impl::add_wdx_token(auth_result       &result,
                                                 uint32_t    const  expires_in,
                                                 std::string const &access_token,
                                                 std::string const &refresh_token,
                                                 std::string const &user_name)
{
    try
    {
        WC_ASSERT(!access_token.empty());

        uint32_t    const lifetime     = get_min_lifetime(expires_in);
        std::string const refresh_info = refresh_token.empty() ? "" : ", containing refresh information";
        wc_log(log_level_t::info, ("Build WDX token, expires in " + std::to_string(lifetime) + " s" + refresh_info).c_str());

        std::string const token = token_handler_m.build_token(build_token_data(lifetime, access_token, refresh_token, user_name));
        result.token            = std::string(wdx_token_prefix) + token;
        result.token_expires_in = lifetime;
        result.user_name = user_name;
    }
    catch(std::exception const &e)
    {
        // We should be able to build a WDx token, but if not authentication/authorization may still be successful
        wc_log(log_level_t::error, (std::string("Failed to build token: ") + e.what()).c_str());
    }
}

auth_result oauth2_backend::backend_impl::verify_wdx_token(std::string   const &token,
                                                           uint32_t    * const  remaining_time)
{
    auth_result result;

    size_t const token_print_length = 8U;
    char const * const token_end = (token.length() > token_print_length) ?
                                   &(token.c_str()[token.length() - token_print_length]) : token.c_str();
    WC_DEBUG_LOG((std::string("Try to authenticate via WDx token [...]") + token_end).c_str());

    try
    {
        auto        const token_without_wdx_prefix = &(token.c_str()[sizeof(wdx_token_prefix) - 1]);
        std::string const payload                  = token_handler_m.get_token_payload(token_without_wdx_prefix);

        // Extract token payload parts
        WC_STATIC_ASSERT(std::string::npos == -1);
        size_t      const raw_expiration_start = 0;
        size_t      const raw_expiration_end   = payload.find('?', raw_expiration_start);
        size_t      const access_token_start   = raw_expiration_end + 1;
        size_t      const access_token_end     = payload.find('?', access_token_start);
        size_t      const refresh_token_start  = access_token_end + 1;
        size_t      const refresh_token_end    = payload.find('?', refresh_token_start);
        size_t      const user_name_start      = refresh_token_end + 1;
        size_t      const user_name_end        = payload.find('?', user_name_start);
        if(    (raw_expiration_end == std::string::npos)
            || (access_token_end   == std::string::npos)
            || (refresh_token_end  == std::string::npos))
        {
            throw std::runtime_error("Invalid WDx token format");
        }
        std::string const raw_expiration       = payload.substr(raw_expiration_start, raw_expiration_end - raw_expiration_start);
        std::string const access_token         = payload.substr(access_token_start,   access_token_end   - access_token_start);
        std::string const refresh_token        = payload.substr(refresh_token_start,  refresh_token_end  - refresh_token_start);
        std::string const user_name            = payload.substr(user_name_start,  user_name_end  - user_name_start);

        // Check if token is valid
        timespec mono_time;
        sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);

        WC_STATIC_ASSERT(sizeof(time_t) >= sizeof(long));
        time_t const expiration = (sizeof(time_t) == sizeof(long)) ? 
                                  std::stol(raw_expiration) :
                                  std::stoll(raw_expiration);
        if(mono_time.tv_sec < expiration)
        {
            WC_ASSERT(std::numeric_limits<uint32_t>::max() >= static_cast<uint64_t>(expiration - mono_time.tv_sec));
            auto const expires_in = static_cast<uint32_t>(expiration - mono_time.tv_sec);
            result.success            = true;
            result.token              = token;
            result.token_expires_in   = expires_in;
            result.user_name          = user_name;
            if(remaining_time != nullptr)
            {
                *remaining_time = expires_in;
            }
            wc_log(log_level_t::info, (std::string("Successfully authenticated via WDx token [...]") + token_end).c_str());
        }
        else
        {
            result.success = false;
            result.expired = true;
            if(remaining_time != nullptr)
            {
                *remaining_time = 0;
            }

            // Try access token verification
            auto const verify_result = provider_m.access_token_verification(access_token);
            if(verify_result.active)
            {
                result.success = true;
                result.expired = false;
                if(remaining_time != nullptr)
                {
                    *remaining_time = verify_result.expires_in;
                }
                add_wdx_token(result, verify_result.expires_in, access_token, refresh_token, verify_result.username);
            }

            // Try to refresh if a refresh token is given
            else if(!refresh_token.empty())
            {
                wc_log(log_level_t::info, (std::string("Try to refresh expired WDx token [...]") + token_end).c_str());
                auto const grant_result = client_m.refresh_token_grant(refresh_token);
                if(grant_result.success)
                {
                    WC_ASSERT(grant_result.expired == false);

                    auto const second_verify_result = provider_m.access_token_verification(grant_result.access_token);
                    result.success = second_verify_result.active;
                    result.expired = false;

                    add_wdx_token(result, second_verify_result.expires_in, grant_result.access_token , grant_result.refresh_token, second_verify_result.username);
                    if(remaining_time != nullptr)
                    {
                        *remaining_time = get_min_lifetime(second_verify_result.expires_in);
                    }
                }
            }
            if(!result.success)
            {
                WC_ASSERT(result.expired);
                wc_log(log_level_t::info, (std::string("Failed to authenticate via expired WDx token [...]") + token_end).c_str());
            }
        }
    }
#ifndef NDEBUG
    catch(std::exception const &e)
    {
        WC_DEBUG_LOG((std::string("Broken WDx token: ") + e.what()).c_str());
    }
    catch(...)
    {
        WC_FAIL("Caught unexpected thing");
#else
    catch(...)
    {
#endif
        wc_log(log_level_t::error, "Failed to authenticate with broken WDx token");
    }

    return result;
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace {
void log_and_throw_errno_error(char const * const error_pre_text)
{
    std::string const error_message = std::string(error_pre_text) +
                                                  errno_to_string(errno) +
                                                  " (" + std::to_string(errno) + ")";
    wc_log(log_level_t::error, error_message.c_str());
    throw wdx::wda::exception(error_message);
}

std::string build_token_data(uint32_t    const  lifetime,
                             std::string const &access_token,
                             std::string const &refresh_token,
                             std::string const &user_name)
{
    // Get timestamp and add lifetime
    timespec mono_time;
    sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);

    WC_ASSERT(lifetime <= static_cast<size_t>(std::numeric_limits<time_t>::max()));
    time_t const expiration = mono_time.tv_sec + static_cast<time_t>(lifetime);

    // Build token data from parts
    return         std::to_string(expiration)
           + "?" + access_token
           + "?" + refresh_token
           + "?" + user_name
           + "?";
}

uint32_t get_min_lifetime(uint32_t const access_token_expires_in)
{
    uint32_t result = 0;
    if(access_token_expires_in > 0)
    {
        if(token_default_lifetime < access_token_expires_in)
        {
            result = token_default_lifetime;
        }
        else
        {
            // Sub one second from original token expiration to honor processing time
            result = access_token_expires_in - 1U;
        }
    }

    return result;
}


} // Anonymous namespace
} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
