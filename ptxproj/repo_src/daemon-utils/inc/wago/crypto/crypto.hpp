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
///  \brief    Cryptography utilities.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_CRYPTO_CRYPTO_HPP_
#define INC_WAGO_CRYPTO_CRYPTO_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"
#include "key.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace crypto {

//------------------------------------------------------------------------------
/// Create a new random key suitable for encryption/decryption
///
/// \return
///   New key object
///
/// \throw std::runtime_error
///   On key generation failure
//------------------------------------------------------------------------------
WAGO_CRYPTO_API key create_key();

//------------------------------------------------------------------------------
/// Default encryption function
///
/// \param plaintext
///   The data to encrypt
/// \param key
///   Key to be used for encryption provided by create_key()
///
/// \return
///   Encrypted data
///
/// \throw std::invalid_argument
///   On invalid key size
/// \throw std::runtime_error
///   On encryption failure
//------------------------------------------------------------------------------
WAGO_CRYPTO_API std::vector<uint8_t> encrypt(std::vector<uint8_t> const& plaintext, 
                                             key                  const& key);

//------------------------------------------------------------------------------
/// Default decryption function
///
/// \param ciphertext
///   Encrypted data
/// \param key
///   Key to be used for decryption provided by create_key()
///
/// \return
///   Decrypted plaintext
///
/// \throw std::invalid_argument
///   On invalid parameters
/// \throw std::runtime_error
///   On decryption failure
//------------------------------------------------------------------------------
WAGO_CRYPTO_API std::vector<uint8_t> decrypt(std::vector<uint8_t> const& ciphertext, 
                                             key                  const& key);

} // namespace crypto
} // namespace wago

#endif // INC_WAGO_CRYPTO_CRYPTO_HPP_
