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
#include "aes256_gcm.hpp"

#include <stdexcept>
#include <algorithm>
#include <limits>
#include <vector>
#include <sstream>
#include <iomanip>
#include <memory>

#include <wc/std_includes.h>
#include <wc/log.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace crypto {
namespace aes256_gcm {

//------------------------------------------------------------------------------
// global variables and const expressions
//------------------------------------------------------------------------------
constexpr size_t const iv_size_bytes   = 12u;  // 96 bits for GCM mode
constexpr size_t const tag_size_bytes  = 16u;  // 128 bits for GCM tag

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------
namespace {

// Custom deleter for EVP_CIPHER_CTX
struct EVP_CIPHER_CTX_Deleter {
    void operator()(EVP_CIPHER_CTX* ctx) const {
        if (ctx) {
            EVP_CIPHER_CTX_free(ctx);
        }
    }
};

using EVP_CIPHER_CTX_ptr = std::unique_ptr<EVP_CIPHER_CTX, EVP_CIPHER_CTX_Deleter>;
}

//------------------------------------------------------------------------------
// public functions implementation
//------------------------------------------------------------------------------
std::vector<uint8_t> encrypt(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key)
{
    if (key.size() != key_size)
    {
        throw std::invalid_argument("Key must be exactly 32 bytes for AES-256");
    }

    if (plaintext.empty())
    {
        throw std::invalid_argument("Plaintext cannot be empty");
    }

    // Generate random IV
    std::vector<uint8_t> iv(iv_size_bytes);
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1)
    {
        throw std::runtime_error("Failed to generate random IV");
    }

    // Create and initialize context with RAII
    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx)
    {
        throw std::runtime_error("Failed to create cipher context");
    }

    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
    {
        throw std::runtime_error("Failed to initialize AES-256-GCM encryption");
    }

    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) != 1)
    {
        throw std::runtime_error("Failed to set IV length");
    }

    // Set key and IV
    if (EVP_EncryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1)
    {
        throw std::runtime_error("Failed to set key and IV");
    }

    // Prepare output buffer
    std::vector<uint8_t> ciphertext(plaintext.size());
    int len = 0;
    int ciphertext_len = 0;

    // Encrypt plaintext
    if (EVP_EncryptUpdate(ctx.get(), ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size())) != 1)
    {
        throw std::runtime_error("Failed to encrypt data");
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx.get(), ciphertext.data() + len, &len) != 1)
    {
        throw std::runtime_error("Failed to finalize encryption");
    }
    ciphertext_len += len;

    // Resize to actual ciphertext length (defensive programming only, because openssl documentation says the result
    // may be smaller than the input. AES-GCM should not produce smaller ciphertext).
    ciphertext.resize(ciphertext_len);

    // Get authentication tag
    std::vector<uint8_t> tag(tag_size_bytes);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) != 1)
    {
        throw std::runtime_error("Failed to get authentication tag");
    }

    // Combine IV + ciphertext + tag
    std::vector<uint8_t> result;
    result.reserve(iv.size() + ciphertext.size() + tag.size());
    result.insert(result.end(), iv.begin(), iv.end());
    result.insert(result.end(), ciphertext.begin(), ciphertext.end());
    result.insert(result.end(), tag.begin(), tag.end());

    return result;
    // ctx automatically freed by unique_ptr destructor
}

std::vector<uint8_t> decrypt(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key)
{
    if (key.size() != key_size)
    {
        throw std::invalid_argument("Key must be exactly 32 bytes for AES-256");
    }

    if (ciphertext.size() < iv_size_bytes + tag_size_bytes)
    {
        throw std::invalid_argument("Ciphertext too short - must contain at least IV and tag");
    }

    // Extract components
    // parasoft-begin-suppress CERT_CPP-CTR55-a "Iterator is not used for iteration."
    std::vector<uint8_t> iv(ciphertext.begin(), ciphertext.begin() + iv_size_bytes);
    std::vector<uint8_t> encrypted_data(ciphertext.begin() + iv_size_bytes, ciphertext.end() - tag_size_bytes);
    std::vector<uint8_t> tag(ciphertext.end() - tag_size_bytes, ciphertext.end());
    // parasoft-end-suppress CERT_CPP-CTR55-a

    // Create and initialize context with RAII
    EVP_CIPHER_CTX_ptr ctx(EVP_CIPHER_CTX_new());
    if (!ctx)
    {
        throw std::runtime_error("Failed to create cipher context");
    }

    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
    {
        throw std::runtime_error("Failed to initialize AES-256-GCM decryption");
    }

    // Set IV length
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv.size()), nullptr) != 1)
    {
        throw std::runtime_error("Failed to set IV length");
    }

    // Set key and IV
    if (EVP_DecryptInit_ex(ctx.get(), nullptr, nullptr, key.data(), iv.data()) != 1)
    {
        throw std::runtime_error("Failed to set key and IV");
    }

    // Prepare output buffer
    std::vector<uint8_t> plaintext(encrypted_data.size());
    int len = 0;
    int plaintext_len = 0;

    // Decrypt data
    if (EVP_DecryptUpdate(ctx.get(), plaintext.data(), &len, encrypted_data.data(), static_cast<int>(encrypted_data.size())) != 1)
    {
        throw std::runtime_error("Failed to decrypt data");
    }
    plaintext_len = len;

    // Set expected tag
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, static_cast<int>(tag.size()), const_cast<uint8_t*>(tag.data())) != 1)
    {
        throw std::runtime_error("Failed to set authentication tag");
    }

    // Finalize decryption and verify tag
    if (EVP_DecryptFinal_ex(ctx.get(), plaintext.data() + len, &len) != 1)
    {
        throw std::runtime_error("Failed to verify authentication tag or finalize decryption");
    }
    plaintext_len += len;

    // Resize to actual plaintext length (defensive programming only, because openssl documentation says the result
    // may be smaller than the input. AES-GCM should not produce smaller plaintext).
    plaintext.resize(plaintext_len);

    return plaintext;
    // ctx automatically freed by unique_ptr destructor
}

//------------------------------------------------------------------------------
// internal function implementations
//------------------------------------------------------------------------------
namespace {

void secure_clear(std::vector<uint8_t>& data)
{
    if (!data.empty())
    {
        // Use OpenSSL's secure clear function (fills the vector with 0's)
        OPENSSL_cleanse(data.data(), data.size());
        data.clear();
    }
}

} // anonymous namespace

} // namespace aes256_gcm
} // namespace crypto
} // namespace wago
