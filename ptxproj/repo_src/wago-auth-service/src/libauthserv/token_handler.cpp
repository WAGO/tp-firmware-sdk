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
///  \brief    Token handler implementation using WAGO crypto library.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "token_handler.hpp"
#include "system_abstraction.hpp"

#include <stdexcept>
#include <mutex>
#include <limits>

#include <wc/std_includes.h>
#include <wc/log.h>
#include <wc/base64.h>
#include <wc/structuring.h>
#include <wago/crypto/crypto.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

struct token_handler::handler_impl
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(handler_impl)

public:
    settings_store_i  &settings_store_m;
    wago::crypto::key  current_key_m;
    wago::crypto::key  old_key_m;
    std::mutex         key_mutex_m;
    time_t             key_expiration_m;

    explicit handler_impl(settings_store_i &settings_store);
    ~handler_impl() noexcept = default;
    
    void rotate_keys_if_needed();
    time_t get_current_time() const;
    time_t get_next_key_expiration_time() const;
};

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
token_handler::handler_impl::handler_impl(settings_store_i &settings_store)
: settings_store_m(settings_store)
, current_key_m(wago::crypto::create_key())
, old_key_m()
{
    key_expiration_m = get_next_key_expiration_time();
}

void token_handler::handler_impl::rotate_keys_if_needed()
{
    time_t current_time = get_current_time();
    if (current_time > key_expiration_m)
    {
        std::lock_guard<std::mutex> lock(key_mutex_m);
        current_key_m.rotate(old_key_m);
        key_expiration_m = get_next_key_expiration_time();
    }
}

time_t token_handler::handler_impl::get_current_time() const
{
    timespec mono_time;
    sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &mono_time);
    return mono_time.tv_sec;
}

time_t token_handler::handler_impl::get_next_key_expiration_time() const
{
    time_t   const current_time = get_current_time();
    uint32_t const lifetime_s   = static_cast<uint32_t>(
        std::stoul(settings_store_m.get_global_setting(settings_store_i::refresh_token_lifetime))
    );

    WC_ASSERT(lifetime_s <= static_cast<uint32_t>(std::numeric_limits<time_t>::max()));
    time_t const new_timestamp = current_time + static_cast<time_t>(lifetime_s);
    if(static_cast<int64_t>(new_timestamp) < static_cast<int64_t>(lifetime_s))
    {
        std::string const error_message = "Unable to apply lifetime of " +
                                          std::to_string(lifetime_s) + "seconds with current timestamp " +
                                          std::to_string(current_time)  + "seconds (value overflow)";
        wc_log(log_level_t::error, error_message);
        throw std::runtime_error(error_message);
    }

    return new_timestamp;
}

token_handler::token_handler(settings_store_i &settings_store)
:impl(std::make_unique<handler_impl>(settings_store))
{ }

token_handler::~token_handler() noexcept = default;

std::string token_handler::build_token(std::string const &payload)
{
    // Check if keys need rotation before encrypting
    impl->rotate_keys_if_needed();
    
    // Convert string to bytes
    std::vector<uint8_t> payload_bytes(payload.begin(), payload.end());
    
    // Encrypt to bytes using current key
    std::vector<uint8_t> ciphertext_bytes;
    {
        std::lock_guard<std::mutex> lock(impl->key_mutex_m);
        ciphertext_bytes = wago::crypto::encrypt(payload_bytes, impl->current_key_m); // parasoft-suppress CERT_C-CON33-a "This is not the system function from <crypt.h>."
    }
    
    // Convert to Base64 for string representation
    return wc_base64_encode(ciphertext_bytes, true);
}

std::string token_handler::get_token_payload(std::string const &token)
{
    // Check if keys need rotation before decrypting (in case we need to use old keys)
    impl->rotate_keys_if_needed();

    // Validate token is not empty
    if(token.empty())
    {
        throw std::runtime_error("Token is empty");
    }
    
    // Convert Base64 to bytes
    std::vector<uint8_t> ciphertext_bytes;
    try
    {
        ciphertext_bytes = wc_base64_decode(token);
    }
    catch(const std::exception&)
    {
        throw std::runtime_error("Failed to decode base64 token");
    }

    // Decrypt to bytes - try current key first, then old key
    std::vector<uint8_t> payload_bytes;
    {
        std::lock_guard<std::mutex> lock(impl->key_mutex_m);
        
        // Try with current key first
        try 
        {
            payload_bytes = wago::crypto::decrypt(ciphertext_bytes, impl->current_key_m);
        }
        catch (const std::exception&) // Catch any exception from current key
        {
            // If current key fails, try with old key
            try 
            {
                payload_bytes = wago::crypto::decrypt(ciphertext_bytes, impl->old_key_m);
            }
            catch (const std::exception&) // Catch any exception from old key
            {
                // Both keys failed - throw runtime_error as expected by tests
                throw std::runtime_error("Failed to decrypt with both current and old keys");
            }
        }
    }
    
    // Convert bytes back to string
    return std::string(payload_bytes.begin(), payload_bytes.end());
}

void token_handler::regenerate_key()
{
    std::lock_guard<std::mutex> lock(impl->key_mutex_m);
    impl->current_key_m.regenerate();
    impl->old_key_m.secure_clear(); // Clear old key
    impl->key_expiration_m = impl->get_next_key_expiration_time(); // Reset expiration
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
