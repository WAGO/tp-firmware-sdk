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
///  \brief    Handler implementation for OAuth 2 tokens.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "oauth_token_handler.hpp"
#include "wago/authserv/exception.hpp"
#include "system_abstraction.hpp"

#include <wc/preprocessing.h>
#include <wc/std_includes.h>
#include <wc/assertion.h>
#include <wc/log.h>
#include <wc/base64.h>
#include <openssl/sha.h>

#include <algorithm>
#include <cstdint>
#include <thread>
#include <chrono>
#include <functional>
#include <cstring>
#include <limits>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {
struct token_data
{
     time_t      expiration_s;
     std::string type;
     std::string client_id;
     std::string scopes;
     std::string user_name;
     std::string additional_data;
};

std::string const build_token_data_string(token_data const &token_data);
token_data const parse_token_data_string(std::string const &token_data);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
constexpr char const auth_code_token_type[] = "act";
constexpr char const access_token_type[]    = "at";
constexpr char const refresh_token_type[]   = "rt";
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
oauth_token_handler::oauth_token_handler(settings_store_i &settings_store)
: token_handler_m(settings_store)
{ }

oauth_token_handler::~oauth_token_handler() noexcept = default;

std::string oauth_token_handler::generate_auth_code(uint32_t    const  lifetime_s,
                                                    std::string const &client_id,
                                                    std::string const &scopes,
                                                    std::string const &user_name,
                                                    std::string const &code_challenge)
{
    return generate_token(lifetime_s, auth_code_token_type, client_id, scopes, user_name, code_challenge);
}

std::string oauth_token_handler::generate_access_token(uint32_t    const  lifetime_s,
                                                       std::string const &client_id,
                                                       std::string const &scopes,
                                                       std::string const &user_name)
{
    wc_log(log_level_t::info, "Generate access token for user \"" + user_name + "\", client ID \"" + client_id + "\", scopes: [" + scopes + "]");
    return generate_token(lifetime_s, access_token_type, client_id, scopes, user_name);
}

std::string oauth_token_handler::generate_refresh_token(uint32_t    const  lifetime_s,
                                                        std::string const &client_id,
                                                        std::string const &scopes,
                                                        std::string const &user_name)
{
    wc_log(log_level_t::info, "Generate refresh token for user \"" + user_name + "\", client ID \"" + client_id + "\", scopes: [" + scopes + "]");
    return generate_token(lifetime_s, refresh_token_type, client_id, scopes, user_name);
}

oauth_token_handler::validation_result oauth_token_handler::validate_auth_code(std::string const &code,
                                                                               std::string const &code_verifier)
{
    validation_result result;

    std::vector<uint8_t> code_challenge_data(SHA256_DIGEST_LENGTH);
    SHA256((uint8_t const*)(code_verifier.data()), code_verifier.length(), code_challenge_data.data()); // parasoft-suppress CERT_C-EXP39-b-3 "The sign has no meaning in byte-oriented data processing"
    
    std::string base64_code_challenge;
    try 
    {
        base64_code_challenge = wc_base64_encode(code_challenge_data, true);
    }
    catch(std::exception const &e)
    {
        wc_log_format(log_level_t::error, "Failed to base64-encode code_challenge: %s", e.what());
        result.valid = false;
        return result;
    }

    result = validate_token(code, auth_code_token_type, base64_code_challenge);

    revoke_token(code); // all auth codes can only be used once
    return result;
}

oauth_token_handler::validation_result oauth_token_handler::validate_access_token(std::string const &token)
{
    return validate_token(token, access_token_type);
}

oauth_token_handler::validation_result oauth_token_handler::validate_refresh_token(std::string const &token)
{
    return validate_token(token, refresh_token_type);
}

void oauth_token_handler::revoke_token(std::string const &token)
{
    try {
        std::string const payload = token_handler_m.get_token_payload(token);
        token_data token_data = parse_token_data_string(payload);

        WC_ASSERT(token_data.expiration_s <= std::numeric_limits<time_t>::max());
        std::lock_guard<std::mutex> lock_guard(token_blacklist_mutex_m);
        token_blacklist_m.emplace(token, token_data.expiration_s);
    }
    catch(...)
    {
        size_t const token_print_length = 8U;
        char const * const token_end = (token.length() > token_print_length) ?
            &(token[token.length() - token_print_length]) : token.c_str();
        wc_log(log_level_t::warning, "Failed to revoke broken token \"[...]" + std::string(token_end) + "\". "
                                     "Returning true as this token can't be used.");
    }
}

void oauth_token_handler::revoke_all_tokens()
{
    token_handler_m.regenerate_key();
    std::lock_guard<std::mutex> lock_guard(token_blacklist_mutex_m);
    token_blacklist_m.clear();
}

std::string oauth_token_handler::generate_token(uint32_t    const  lifetime_s,
                                                std::string const &type,
                                                std::string const &client_id,
                                                std::string const &scopes,
                                                std::string const &user_name,
                                                std::string const &additional_data)
{
    // calculate expiration time
    timespec mono_time;
    sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);

    WC_ASSERT(lifetime_s <= static_cast<uint32_t>(std::numeric_limits<time_t>::max()));
    time_t const expiration_s = mono_time.tv_sec + static_cast<time_t>(lifetime_s);
    if(static_cast<int64_t>(expiration_s) < static_cast<int64_t>(lifetime_s))
    {
        std::string const error_message = "Unable to apply lifetime of " +
                                          std::to_string(lifetime_s) + "seconds with current timestamp " +
                                          std::to_string(mono_time.tv_sec) + "seconds (value overflow)";
        wc_log(log_level_t::error, error_message);
        throw authserv::exception(error_message);
    }

    // build token
    token_data token_data{expiration_s, type, client_id, scopes, user_name, additional_data};
    std::string const token_payload = build_token_data_string(token_data);

    return token_handler_m.build_token(token_payload);
}

oauth_token_handler::validation_result oauth_token_handler::validate_token(std::string const &token,
                                                                           std::string const &expected_type,
                                                                           std::string const &expected_additional_data) noexcept
{
    validation_result result;

    timespec mono_time;
    sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);

    // Check if token is blacklisted
    {
        std::lock_guard<std::mutex> lock_guard(token_blacklist_mutex_m);
        if (token_blacklist_m.count(token) > 0)
        {
            result.valid = false;
            return result;
        }
    }

    size_t const token_print_length = 8U;
    char const * const token_end = (token.length() > token_print_length) ?
        &(token[token.length() - token_print_length]) : token.c_str();
    WC_DEBUG_LOG("Validate token [...]" + std::string(token_end));

    try
    {
        std::string const payload = token_handler_m.get_token_payload(token);
        token_data token_data = parse_token_data_string(payload);

        // Check token payload
        if((token_data.type == expected_type) && (token_data.additional_data == expected_additional_data))
        {
            result.valid = true;

            if(mono_time.tv_sec <= token_data.expiration_s)
            {
                result.expired          = false;
                WC_ASSERT(std::numeric_limits<uint32_t>::max() >= static_cast<uint64_t>(token_data.expiration_s - mono_time.tv_sec));
                result.remaining_time_s = static_cast<uint32_t>(token_data.expiration_s - mono_time.tv_sec);
            }
            else
            {
                result.expired = true;
                result.remaining_time_s = 0;
                wc_log(log_level_t::warning, "Failed to validate expired token [...]" + std::string(token_end));
            }

            result.user_name = token_data.user_name;
            result.client_id = token_data.client_id;
            result.scopes    = token_data.scopes;
            std::string const message = "Successfully validated token [...]" + std::string(token_end) + " for client \"" + token_data.client_id + "\" (scopes: [" + token_data.scopes + "])";
            wc_log(log_level_t::info, message);
        }
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, "Failed to validate broken token [...]" + std::string(token_end) + ": " + e.what());
        // TODO: Slow down unauthenticated requests with wrong/broken tokens for security reasons if requested?
//        if(broken_token_slowdown_m > 0)
//        {
//            std::this_thread::sleep_for(std::chrono::seconds(broken_token_slowdown_m));
//        }
    }

    return result;
}

void oauth_token_handler::cleanup_blacklist() noexcept {
    timespec mono_time;
    sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);

    // cleanup blacklist of tokens that are expired
    std::lock_guard<std::mutex> lock_guard(token_blacklist_mutex_m);
    for (auto current = token_blacklist_m.begin(); current != token_blacklist_m.end();)
    {
        if (current->second < mono_time.tv_sec)
        {
            current = token_blacklist_m.erase(current);
        }
        else
        {
            ++current;
        }
    }
}


//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
std::string const build_token_data_string(token_data const &token_data)
{
    std::string const payload = token_data.type
                              + "?" + token_data.user_name
                              + "?" + std::to_string(token_data.expiration_s)
                              + "?" + token_data.client_id
                              + "?" + token_data.scopes
                              + "?" + token_data.additional_data
                              + "?";
    return payload;
}

token_data const parse_token_data_string(std::string const &token_string)
{
    // local utility to check if an index is not npos
    auto find_not_npos = [](std::string const & source, char what, size_t start_index) {
        auto found = source.find(what, start_index);
        if (found == std::string::npos) {
            // De-facto this should never happen, as the string is always build
            // by the build_token_data_string function in the first place.
            // But just in case, we throw an exception, if somehow a malformed
            // string is provided due to some security issue we are currently
            // not aware of.
            throw std::runtime_error("Token string is broken");
        }
        return found;
    };

    token_data result;

    // Extract token payload string parts
    size_t const start_of_type       = 0;
    size_t const end_of_type         = find_not_npos(token_string, '?', start_of_type);
    size_t const start_of_user_name  = end_of_type + 1;
    size_t const end_of_user_name    = find_not_npos(token_string, '?', start_of_user_name);
    size_t const start_of_expiration = end_of_user_name + 1;
    size_t const end_of_expiration   = find_not_npos(token_string, '?', start_of_expiration);
    size_t const start_of_client_id  = end_of_expiration + 1;
    size_t const end_of_client_id    = find_not_npos(token_string, '?', start_of_client_id);
    size_t const start_of_scopes     = end_of_client_id + 1;
    size_t const end_of_scopes       = find_not_npos(token_string, '?', start_of_scopes);
    size_t const start_of_additional = end_of_scopes + 1;
    size_t const end_of_additional   = find_not_npos(token_string, '?', start_of_additional);

    // Parse string parts into result members
    result.type                      = token_string.substr(start_of_type,       end_of_type       - start_of_type);
    result.user_name                 = token_string.substr(start_of_user_name,  end_of_user_name  - start_of_user_name);
    std::string const expiration_raw = token_string.substr(start_of_expiration, end_of_expiration - start_of_expiration);
    WC_STATIC_ASSERT((sizeof(time_t) == sizeof(long)) || (sizeof(time_t) == sizeof(long long)));
    if (sizeof(time_t) == sizeof(long))
    {
        result.expiration_s          = std::stol(expiration_raw);
    }
    else
    {
        result.expiration_s          = std::stoll(expiration_raw);
    }
    result.client_id                 = token_string.substr(start_of_client_id,  end_of_client_id  - start_of_client_id);
    result.scopes                    = token_string.substr(start_of_scopes,     end_of_scopes     - start_of_scopes);
    result.additional_data           = token_string.substr(start_of_additional, end_of_additional - start_of_additional);

    return result;
}

} // Anonymous namespace
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
