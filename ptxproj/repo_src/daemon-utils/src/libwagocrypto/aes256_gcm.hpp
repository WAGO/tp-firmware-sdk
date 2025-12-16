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
///  \brief    Cryptography utilities with AES-256-GCM encryption.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWAGOCRYPTO_AES256_GCM_HPP_
#define SRC_LIBWAGOCRYPTO_AES256_GCM_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>
#include <cstdint>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace crypto {
namespace aes256_gcm {

/// \brief Key size for AES-256-GCM
/// \note 32 bytes = 256 bits
constexpr size_t const key_size  = 32u;

/// \brief AES-256-GCM encryption function
/// \param plaintext The data to encrypt
/// \param key 32-byte AES-256 key
/// \return Encrypted data (IV + ciphertext + auth_tag)
/// \throws std::invalid_argument on invalid key size
/// \throws std::runtime_error on encryption failure
std::vector<uint8_t> encrypt(std::vector<uint8_t> const& plaintext,
                             std::vector<uint8_t> const& key);

/// \brief AES-256-GCM decryption function
/// \param ciphertext Encrypted data (IV + ciphertext + auth_tag)
/// \param key 32-byte AES-256 key
/// \return Decrypted plaintext
/// \throws std::invalid_argument on invalid parameters
/// \throws std::runtime_error on decryption failure
std::vector<uint8_t> decrypt(std::vector<uint8_t> const& ciphertext,
                             std::vector<uint8_t> const& key);

} // namespace aes256_gcm
} // namespace crypto
} // namespace wago

#endif // SRC_LIBWAGOCRYPTO_AES256_GCM_HPP_
