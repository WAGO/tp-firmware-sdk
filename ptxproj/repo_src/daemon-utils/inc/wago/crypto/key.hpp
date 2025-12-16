//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Cryptography key.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_CRYPTO_KEY_HPP_
#define INC_WAGO_CRYPTO_KEY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <string>
#include <cstdint>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace crypto {

/// \brief Secure key class that prevents copying and securely clears data
class WAGO_CRYPTO_API key final
{
public:
    //------------------------------------------------------------------------------
    /// Create a new random key
    ///
    /// \param key_size
    ///   Size of the key in bytes (default is 0, which generates an empty key).
    ///
    /// \throw std::runtime_error
    ///   On key generation failure
    //------------------------------------------------------------------------------
    key(size_t key_size = 0);

    //------------------------------------------------------------------------------
    /// Destructor - securely clears key data
    //------------------------------------------------------------------------------
    ~key() noexcept;

    // Delete copy constructor and copy assignment operator, as keys should
    // not be copied to prevent accidental leaks of sensitive data.
    key(key const&) = delete;
    key& operator=(key const&) = delete;

    // Allow move constructor and move assignment operator
    key(key&&) noexcept;
    key& operator=(key&&) noexcept;

    //------------------------------------------------------------------------------
    /// Regenerate the key (invalidates existing tokens using this key)
    ///
    /// \throw std::runtime_error
    ///   On key generation failure
    //------------------------------------------------------------------------------
    void regenerate();

    //------------------------------------------------------------------------------
    /// Rotate keys: this key becomes old, generate new current
    ///
    /// \param old_key
    ///   [in/out] Will receive the current key after being cleared safely.
    ///
    /// \throw std::runtime_error
    ///   On key generation failure
    //------------------------------------------------------------------------------
    void rotate(key& old_key);

    //------------------------------------------------------------------------------
    /// Securely clear the key data
    //------------------------------------------------------------------------------
    void secure_clear() noexcept;

private:
    size_t               key_size_m;
    std::vector<uint8_t> key_data_m;

    // Friend functions need access to key data
    friend std::vector<uint8_t> encrypt(std::vector<uint8_t> const& plaintext, 
                                        key                  const& encryption_key);
    friend std::vector<uint8_t> decrypt(std::vector<uint8_t> const& ciphertext, 
                                        key                  const& decryption_key);
    
    // Friend function for testing purposes only
    friend std::vector<uint8_t> get_key_data_for_testing(key const& test_key);
};


} // namespace crypto
} // namespace wago

#endif // INC_WAGO_CRYPTO_KEY_HPP_
