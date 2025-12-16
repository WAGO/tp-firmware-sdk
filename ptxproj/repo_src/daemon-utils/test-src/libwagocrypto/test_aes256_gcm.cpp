//------------------------------------------------------------------------------
// Copyright (c) 2025 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Test AES-256-GCM encryption and decryption functions.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "aes256_gcm.hpp"

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <set>
#include <chrono>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace {

// Helper function to create a valid AES-256 key
std::vector<uint8_t> create_test_key()
{
    std::vector<uint8_t> key(wago::crypto::aes256_gcm::key_size, 0x42);
    return key;
}

// Helper function to create deterministic sample data
std::vector<uint8_t> create_random_data(size_t size)
{
    std::vector<uint8_t> data(size);

    // Create predictable pattern: repeating sequence of bytes 0x00-0xFF
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<uint8_t>(i % 256);
    }
    return data;
}

} // anonymous namespace

//------------------------------------------------------------------------------
// Test fixture
//------------------------------------------------------------------------------
class aes256_gcm_test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_key_ = create_test_key();
        test_plaintext_ = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!'};
    }

    std::vector<uint8_t> test_key_;
    std::vector<uint8_t> test_plaintext_;
};

//------------------------------------------------------------------------------
// Basic functionality tests
//------------------------------------------------------------------------------

TEST_F(aes256_gcm_test, encrypt_decrypt_basic_functionality)
{
    // Encrypt the plaintext
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Verify ciphertext is not empty and different from plaintext
    EXPECT_FALSE(ciphertext.empty());
    EXPECT_NE(ciphertext, test_plaintext_);

    // Verify ciphertext has correct structure (IV + ciphertext + tag)
    // Should be at least: 12 bytes (IV) + plaintext_size + 16 bytes (tag)
    EXPECT_GE(ciphertext.size(), 12 + test_plaintext_.size() + 16);

    // Decrypt the ciphertext
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    // Verify decrypted data matches original plaintext
    EXPECT_EQ(decrypted, test_plaintext_);
}

TEST_F(aes256_gcm_test, encrypt_produces_different_output_with_same_input)
{
    // Encrypt the same plaintext twice
    auto ciphertext1 = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);
    auto ciphertext2 = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Results should be different due to random IV
    EXPECT_NE(ciphertext1, ciphertext2);

    // But both should decrypt to the same plaintext
    auto decrypted1 = wago::crypto::aes256_gcm::decrypt(ciphertext1, test_key_);
    auto decrypted2 = wago::crypto::aes256_gcm::decrypt(ciphertext2, test_key_);

    EXPECT_EQ(decrypted1, test_plaintext_);
    EXPECT_EQ(decrypted2, test_plaintext_);
    EXPECT_EQ(decrypted1, decrypted2);
}

TEST_F(aes256_gcm_test, encrypt_decrypt_empty_data)
{
    std::vector<uint8_t> empty_plaintext = {0x00}; // Single zero byte

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(empty_plaintext, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, empty_plaintext);
}

TEST_F(aes256_gcm_test, encrypt_decrypt_large_data)
{
    // Test with 1MB of random data
    auto large_plaintext = create_random_data(1024 * 1024);

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(large_plaintext, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, large_plaintext);
}

TEST_F(aes256_gcm_test, encrypt_decrypt_various_sizes)
{
    // Test various data sizes
    std::vector<size_t> sizes = {1, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 1023, 1024, 1025};

    for (size_t size : sizes) {
        auto plaintext = create_random_data(size);

        auto ciphertext = wago::crypto::aes256_gcm::encrypt(plaintext, test_key_);
        auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

        EXPECT_EQ(decrypted, plaintext) << "Failed for size: " << size;
    }
}

//------------------------------------------------------------------------------
// Error handling tests
//------------------------------------------------------------------------------

TEST_F(aes256_gcm_test, encrypt_invalid_key_size)
{
    // Test with key too small
    std::vector<uint8_t> small_key(31, 0x42);
    EXPECT_THROW(wago::crypto::aes256_gcm::encrypt(test_plaintext_, small_key), std::invalid_argument);

    // Test with key too large
    std::vector<uint8_t> large_key(33, 0x42);
    EXPECT_THROW(wago::crypto::aes256_gcm::encrypt(test_plaintext_, large_key), std::invalid_argument);

    // Test with empty key
    std::vector<uint8_t> empty_key;
    EXPECT_THROW(wago::crypto::aes256_gcm::encrypt(test_plaintext_, empty_key), std::invalid_argument);
}

TEST_F(aes256_gcm_test, decrypt_invalid_key_size)
{
    // First create valid ciphertext
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Test with key too small
    std::vector<uint8_t> small_key(31, 0x42);
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(ciphertext, small_key), std::invalid_argument);

    // Test with key too large
    std::vector<uint8_t> large_key(33, 0x42);
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(ciphertext, large_key), std::invalid_argument);

    // Test with empty key
    std::vector<uint8_t> empty_key;
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(ciphertext, empty_key), std::invalid_argument);
}

TEST_F(aes256_gcm_test, encrypt_empty_plaintext)
{
    std::vector<uint8_t> empty_plaintext;
    EXPECT_THROW(wago::crypto::aes256_gcm::encrypt(empty_plaintext, test_key_), std::invalid_argument);
}

TEST_F(aes256_gcm_test, decrypt_invalid_ciphertext_size)
{
    // Test with ciphertext too small (less than IV + tag size)
    std::vector<uint8_t> small_ciphertext(27, 0x42); // 12 (IV) + 16 (tag) - 1
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(small_ciphertext, test_key_), std::invalid_argument);

    // Test with empty ciphertext
    std::vector<uint8_t> empty_ciphertext;
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(empty_ciphertext, test_key_), std::invalid_argument);
}

TEST_F(aes256_gcm_test, decrypt_with_wrong_key)
{
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Create different key
    std::vector<uint8_t> wrong_key(wago::crypto::aes256_gcm::key_size, 0x24);

    // Decryption should fail due to authentication tag verification
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(ciphertext, wrong_key), std::runtime_error);
}

TEST_F(aes256_gcm_test, decrypt_tampered_ciphertext)
{
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Tamper with the ciphertext (modify middle byte)
    auto tampered_ciphertext = ciphertext;
    tampered_ciphertext[ciphertext.size() / 2] ^= 0xFF;

    // Decryption should fail due to authentication tag verification
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(tampered_ciphertext, test_key_), std::runtime_error);
}

TEST_F(aes256_gcm_test, decrypt_tampered_iv)
{
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Tamper with the IV (first 12 bytes)
    auto tampered_ciphertext = ciphertext;
    tampered_ciphertext[5] ^= 0xFF;

    // Decryption should fail due to authentication tag verification
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(tampered_ciphertext, test_key_), std::runtime_error);
}

TEST_F(aes256_gcm_test, decrypt_tampered_tag)
{
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // Tamper with the authentication tag (last 16 bytes)
    auto tampered_ciphertext = ciphertext;
    tampered_ciphertext[ciphertext.size() - 5] ^= 0xFF;

    // Decryption should fail due to authentication tag verification
    EXPECT_THROW(wago::crypto::aes256_gcm::decrypt(tampered_ciphertext, test_key_), std::runtime_error);
}

//------------------------------------------------------------------------------
// Security tests
//------------------------------------------------------------------------------

TEST_F(aes256_gcm_test, different_keys_produce_different_ciphertext)
{
    std::vector<uint8_t> key1(wago::crypto::aes256_gcm::key_size, 0x42);
    std::vector<uint8_t> key2(wago::crypto::aes256_gcm::key_size, 0x24);

    auto ciphertext1 = wago::crypto::aes256_gcm::encrypt(test_plaintext_, key1);
    auto ciphertext2 = wago::crypto::aes256_gcm::encrypt(test_plaintext_, key2);

    // Ciphertexts should be different
    EXPECT_NE(ciphertext1, ciphertext2);

    // Each should decrypt correctly with its respective key
    auto decrypted1 = wago::crypto::aes256_gcm::decrypt(ciphertext1, key1);
    auto decrypted2 = wago::crypto::aes256_gcm::decrypt(ciphertext2, key2);

    EXPECT_EQ(decrypted1, test_plaintext_);
    EXPECT_EQ(decrypted2, test_plaintext_);
}

TEST_F(aes256_gcm_test, iv_uniqueness)
{
    // Encrypt the same data multiple times and check IV uniqueness
    std::set<std::vector<uint8_t>> ivs;
    const int num_encryptions = 100;

    for (int i = 0; i < num_encryptions; ++i) {
        auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

        // Extract IV (first 12 bytes)
        std::vector<uint8_t> iv(ciphertext.begin(), ciphertext.begin() + 12);
        ivs.insert(iv);
    }

    // All IVs should be unique
    EXPECT_EQ(ivs.size(), num_encryptions);
}

//------------------------------------------------------------------------------
// Edge cases and boundary tests
//------------------------------------------------------------------------------

TEST_F(aes256_gcm_test, zero_key)
{
    std::vector<uint8_t> zero_key(wago::crypto::aes256_gcm::key_size, 0x00);

    // Should work with all-zero key (though not recommended in practice)
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, zero_key);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, zero_key);

    EXPECT_EQ(decrypted, test_plaintext_);
}

TEST_F(aes256_gcm_test, max_key)
{
    std::vector<uint8_t> max_key(wago::crypto::aes256_gcm::key_size, 0xFF);

    // Should work with all-FF key
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, max_key);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, max_key);

    EXPECT_EQ(decrypted, test_plaintext_);
}

TEST_F(aes256_gcm_test, binary_data)
{
    // Test with binary data including null bytes
    std::vector<uint8_t> binary_data = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0x00, 0x80, 0x7F};

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(binary_data, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, binary_data);
}

//------------------------------------------------------------------------------
// AES-256-GCM specific tests
//------------------------------------------------------------------------------

TEST_F(aes256_gcm_test, ciphertext_format_verification)
{
    auto ciphertext = wago::crypto::aes256_gcm::encrypt(test_plaintext_, test_key_);

    // AES-256-GCM ciphertext format: IV (12 bytes) + ciphertext + tag (16 bytes)
    EXPECT_GE(ciphertext.size(), 12 + 16); // At least IV + tag
    EXPECT_EQ(ciphertext.size(), 12 + test_plaintext_.size() + 16); // Exact size for our plaintext

    // IV should be at the beginning (first 12 bytes)
    // Tag should be at the end (last 16 bytes)
    // Actual encrypted data should be in the middle
}

TEST_F(aes256_gcm_test, key_size_constants)
{
    // Verify AES-256-GCM key size is correct
    EXPECT_EQ(wago::crypto::aes256_gcm::key_size, 32); // 256 bits = 32 bytes
}

TEST_F(aes256_gcm_test, repeating_pattern_data)
{
    // Test with repeating patterns to ensure AES-256-GCM handles them properly
    std::vector<uint8_t> pattern_data(1000, 0xAA);

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(pattern_data, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, pattern_data);

    // Ciphertext should not show the pattern (basic check for proper encryption)
    EXPECT_NE(ciphertext, pattern_data);

    // Check that ciphertext doesn't contain obvious patterns
    // (This is a basic test - a full avalanche effect test would be more complex)
    size_t same_bytes = 0;
    for (size_t i = 12; i < ciphertext.size() - 16 - 1; ++i) { // Skip IV and tag
        if (ciphertext[i] == ciphertext[i + 1]) {
            same_bytes++;
        }
    }
    // Should not have too many consecutive identical bytes in ciphertext
    EXPECT_LT(same_bytes, (ciphertext.size() - 28) / 10); // Less than 10% consecutive matches
}

TEST_F(aes256_gcm_test, all_zero_plaintext)
{
    std::vector<uint8_t> zero_data(1000, 0x00);

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(zero_data, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, zero_data);
    // Ciphertext should not be all zeros (AES-256-GCM should not leak pattern)
    EXPECT_NE(ciphertext, zero_data);

    // Check that encrypted portion is not all zeros
    bool has_non_zero = false;
    for (size_t i = 12; i < ciphertext.size() - 16; ++i) { // Skip IV and tag
        if (ciphertext[i] != 0x00) {
            has_non_zero = true;
            break;
        }
    }
    EXPECT_TRUE(has_non_zero);
}

TEST_F(aes256_gcm_test, all_max_plaintext)
{
    std::vector<uint8_t> max_data(1000, 0xFF);

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(max_data, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, max_data);
    // Ciphertext should not be all 0xFF
    EXPECT_NE(ciphertext, max_data);

    // Check that encrypted portion is not all 0xFF
    bool has_non_max = false;
    for (size_t i = 12; i < ciphertext.size() - 16; ++i) { // Skip IV and tag
        if (ciphertext[i] != 0xFF) {
            has_non_max = true;
            break;
        }
    }
    EXPECT_TRUE(has_non_max);
}

TEST_F(aes256_gcm_test, minimal_plaintext_size)
{
    // Test with single byte
    std::vector<uint8_t> single_byte = {0x42};

    auto ciphertext = wago::crypto::aes256_gcm::encrypt(single_byte, test_key_);
    auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

    EXPECT_EQ(decrypted, single_byte);
    // Ciphertext should be IV (12) + 1 byte + tag (16) = 29 bytes
    EXPECT_EQ(ciphertext.size(), 29);
}

TEST_F(aes256_gcm_test, block_boundary_sizes)
{
    // AES works with 16-byte blocks, test various sizes around block boundaries
    std::vector<size_t> boundary_sizes = {15, 16, 17, 31, 32, 33, 47, 48, 49};

    for (size_t size : boundary_sizes) {
        auto plaintext = create_random_data(size);

        auto ciphertext = wago::crypto::aes256_gcm::encrypt(plaintext, test_key_);
        auto decrypted = wago::crypto::aes256_gcm::decrypt(ciphertext, test_key_);

        EXPECT_EQ(decrypted, plaintext) << "Failed for block boundary size: " << size;

        // Verify ciphertext size is correct for GCM (no padding required)
        EXPECT_EQ(ciphertext.size(), 12 + size + 16) << "Wrong ciphertext size for plaintext size: " << size;
    }
}
