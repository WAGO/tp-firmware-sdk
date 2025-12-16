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
///  \brief    Cryptography key implementation.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/crypto/key.hpp"

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

//------------------------------------------------------------------------------
// key class implementation
//------------------------------------------------------------------------------
WC_STATIC_ASSERT(std::is_copy_constructible<wago::crypto::key>::value == false);

key::key(size_t key_size)
: key_size_m(key_size)
, key_data_m(key_size)
{
    if (RAND_bytes(key_data_m.data(), static_cast<int>(key_data_m.size())) != 1)
    {
        secure_clear();
        throw std::runtime_error("Failed to generate random key");
    }
}

key::~key() noexcept
{
    secure_clear();
}

key::key(key&& other) noexcept
: key_size_m(other.key_size_m)
, key_data_m(std::move(other.key_data_m))
{
    other.key_data_m = {};
}

key& key::operator=(key&& other) noexcept
{
    if (this != &other)
    {
        secure_clear();
        key_size_m = other.key_size_m;
        key_data_m = std::move(other.key_data_m);
        other.key_data_m = {};
    }
    return *this;
}

void key::regenerate()
{
    secure_clear();
    *this = key(key_size_m); // Recreate with same size
}

void key::secure_clear() noexcept
{
    if (!key_data_m.empty())
    {
        OPENSSL_cleanse(key_data_m.data(), key_data_m.size());
        key_data_m.clear();
    }
}

void key::rotate(key& old_key)
{
    // Move current key to old key, then regenerate current
    old_key = std::move(*this);
    // Create new current key with same size
    *this = key(old_key.key_size_m);
}

} // namespace crypto
} // namespace wago
