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
///  \brief    Cryptography implementation with AES-256-GCM encryption.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/crypto/crypto.hpp"
#include "aes256_gcm.hpp"

namespace wago {
namespace crypto {

key create_key()
{
    return key(aes256_gcm::key_size);
}

std::vector<uint8_t> encrypt(std::vector<uint8_t> const& plaintext,
                             key                  const& encryption_key)
{
    return aes256_gcm::encrypt(plaintext, encryption_key.key_data_m);
}

std::vector<uint8_t> decrypt(std::vector<uint8_t> const& ciphertext,
                             key                  const& decryption_key)
{
    return aes256_gcm::decrypt(ciphertext, decryption_key.key_data_m);
}

}
}
