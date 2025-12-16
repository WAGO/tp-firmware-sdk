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
///  \brief    High-level integration tests for WAGO crypto library.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/crypto/crypto.hpp"

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <atomic>

//------------------------------------------------------------------------------
// Test fixture
//------------------------------------------------------------------------------
class crypto_test : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        test_plaintext_ = {'H', 'e', 'l', 'l', 'o', ' ', 'C', 'r', 'y', 'p', 't', 'o', '!'};
    }

    std::vector<uint8_t> test_plaintext_;
};

//------------------------------------------------------------------------------
// Basic integration tests
//------------------------------------------------------------------------------

TEST_F(crypto_test, create_key_and_encrypt_decrypt)
{
    // Create a key using the crypto interface
    auto key = wago::crypto::create_key();
    
    // Encrypt using the crypto interface
    auto ciphertext = wago::crypto::encrypt(test_plaintext_, key);
    
    // Verify ciphertext is not empty and different from plaintext
    EXPECT_FALSE(ciphertext.empty());
    EXPECT_NE(ciphertext, test_plaintext_);
    
    // Decrypt using the crypto interface
    auto decrypted = wago::crypto::decrypt(ciphertext, key);
    
    // Verify decrypted data matches original plaintext
    EXPECT_EQ(decrypted, test_plaintext_);
}

TEST_F(crypto_test, multiple_keys_produce_different_results)
{
    auto key1 = wago::crypto::create_key();
    auto key2 = wago::crypto::create_key();
    
    auto ciphertext1 = wago::crypto::encrypt(test_plaintext_, key1);
    auto ciphertext2 = wago::crypto::encrypt(test_plaintext_, key2);
    
    // Different keys should produce different ciphertexts
    EXPECT_NE(ciphertext1, ciphertext2);
    
    // Each should decrypt correctly with its respective key
    auto decrypted1 = wago::crypto::decrypt(ciphertext1, key1);
    auto decrypted2 = wago::crypto::decrypt(ciphertext2, key2);
    
    EXPECT_EQ(decrypted1, test_plaintext_);
    EXPECT_EQ(decrypted2, test_plaintext_);
    
    // Cross-decryption should fail
    EXPECT_THROW(wago::crypto::decrypt(ciphertext1, key2), std::runtime_error);
    EXPECT_THROW(wago::crypto::decrypt(ciphertext2, key1), std::runtime_error);
}

TEST_F(crypto_test, encryption_with_same_key_produces_different_output)
{
    auto key = wago::crypto::create_key();
    
    // Encrypt the same plaintext multiple times
    auto ciphertext1 = wago::crypto::encrypt(test_plaintext_, key);
    auto ciphertext2 = wago::crypto::encrypt(test_plaintext_, key);
    auto ciphertext3 = wago::crypto::encrypt(test_plaintext_, key);
    
    // All should be different (due to randomness in encryption)
    EXPECT_NE(ciphertext1, ciphertext2);
    EXPECT_NE(ciphertext2, ciphertext3);
    EXPECT_NE(ciphertext1, ciphertext3);
    
    // But all should decrypt to the same plaintext
    auto decrypted1 = wago::crypto::decrypt(ciphertext1, key);
    auto decrypted2 = wago::crypto::decrypt(ciphertext2, key);
    auto decrypted3 = wago::crypto::decrypt(ciphertext3, key);
    
    EXPECT_EQ(decrypted1, test_plaintext_);
    EXPECT_EQ(decrypted2, test_plaintext_);
    EXPECT_EQ(decrypted3, test_plaintext_);
}

//------------------------------------------------------------------------------
// Key management tests
//------------------------------------------------------------------------------

TEST_F(crypto_test, key_regeneration_invalidates_old_encryption)
{
    auto key = wago::crypto::create_key();
    
    // Encrypt with original key
    auto ciphertext = wago::crypto::encrypt(test_plaintext_, key);
    auto decrypted = wago::crypto::decrypt(ciphertext, key);
    EXPECT_EQ(decrypted, test_plaintext_);
    
    // Regenerate the key
    key.regenerate();
    
    // Old ciphertext should no longer decrypt with regenerated key
    EXPECT_THROW(wago::crypto::decrypt(ciphertext, key), std::runtime_error);
    
    // But new encryption/decryption should work
    auto new_ciphertext = wago::crypto::encrypt(test_plaintext_, key);
    auto new_decrypted = wago::crypto::decrypt(new_ciphertext, key);
    EXPECT_EQ(new_decrypted, test_plaintext_);
}

TEST_F(crypto_test, key_rotation_workflow)
{
    auto current_key = wago::crypto::create_key();
    auto old_key = wago::crypto::create_key();
    
    // Encrypt with current key
    auto ciphertext_current = wago::crypto::encrypt(test_plaintext_, current_key);
    
    // Rotate keys
    current_key.rotate(old_key);
    
    // Old ciphertext should still decrypt with old key
    auto decrypted_old = wago::crypto::decrypt(ciphertext_current, old_key);
    EXPECT_EQ(decrypted_old, test_plaintext_);
    
    // New encryption should work with current key
    auto ciphertext_new = wago::crypto::encrypt(test_plaintext_, current_key);
    auto decrypted_new = wago::crypto::decrypt(ciphertext_new, current_key);
    EXPECT_EQ(decrypted_new, test_plaintext_);
    
    // Cross-decryption should fail
    EXPECT_THROW(wago::crypto::decrypt(ciphertext_current, current_key), std::runtime_error);
    EXPECT_THROW(wago::crypto::decrypt(ciphertext_new, old_key), std::runtime_error);
}

TEST_F(crypto_test, multiple_key_rotations)
{
    auto current_key = wago::crypto::create_key();
    std::vector<std::unique_ptr<wago::crypto::key>> old_keys;
    std::vector<std::vector<uint8_t>> ciphertexts;
    
    // Create multiple generations of keys and encrypt data with each
    for (int i = 0; i < 3; ++i) {
        // Encrypt with current key
        auto ciphertext = wago::crypto::encrypt(test_plaintext_, current_key);
        ciphertexts.push_back(ciphertext);
        
        // Rotate to next generation
        auto old_key = std::make_unique<wago::crypto::key>();
        current_key.rotate(*old_key);
        old_keys.push_back(std::move(old_key));
    }
    
    // Each old key should still decrypt its corresponding ciphertext
    for (size_t i = 0; i < old_keys.size(); ++i) {
        auto decrypted = wago::crypto::decrypt(ciphertexts[i], *old_keys[i]);
        EXPECT_EQ(decrypted, test_plaintext_) << "Failed for key generation " << i;
    }
    
    // Current key should not decrypt old ciphertexts
    for (const auto& ciphertext : ciphertexts) {
        EXPECT_THROW(wago::crypto::decrypt(ciphertext, current_key), std::runtime_error);
    }
}

//------------------------------------------------------------------------------
// Error handling integration tests
//------------------------------------------------------------------------------

TEST_F(crypto_test, tampered_data_detection)
{
    auto key = wago::crypto::create_key();
    auto ciphertext = wago::crypto::encrypt(test_plaintext_, key);
    
    // Tamper with various parts of the ciphertext
    auto tampered_ciphertext = ciphertext;
    
    // Tamper with beginning
    tampered_ciphertext[5] ^= 0xFF;
    EXPECT_THROW(wago::crypto::decrypt(tampered_ciphertext, key), std::runtime_error);
    
    // Restore and tamper with middle
    tampered_ciphertext = ciphertext;
    tampered_ciphertext[ciphertext.size() / 2] ^= 0xFF;
    EXPECT_THROW(wago::crypto::decrypt(tampered_ciphertext, key), std::runtime_error);
    
    // Restore and tamper with end
    tampered_ciphertext = ciphertext;
    tampered_ciphertext[ciphertext.size() - 5] ^= 0xFF;
    EXPECT_THROW(wago::crypto::decrypt(tampered_ciphertext, key), std::runtime_error);
}

TEST_F(crypto_test, invalid_ciphertext_formats)
{
    auto key = wago::crypto::create_key();
    
    // Too small ciphertext
    std::vector<uint8_t> too_small(10, 0x42);
    EXPECT_THROW(wago::crypto::decrypt(too_small, key), std::invalid_argument);
    
    // Empty ciphertext
    std::vector<uint8_t> empty_ciphertext;
    EXPECT_THROW(wago::crypto::decrypt(empty_ciphertext, key), std::invalid_argument);
}

//------------------------------------------------------------------------------
// Thread safety integration tests
//------------------------------------------------------------------------------

TEST_F(crypto_test, multiple_keys_in_parallel)
{
    const int num_threads = 4;
    const int operations_per_thread = 50;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            try {
                for (int op = 0; op < operations_per_thread; ++op) {
                    auto key = wago::crypto::create_key();
                    auto ciphertext = wago::crypto::encrypt(test_plaintext_, key);
                    auto decrypted = wago::crypto::decrypt(ciphertext, key);
                    if (decrypted == test_plaintext_) {
                        // Success
                    } else {
                        return; // Failure
                    }
                }
                success_count.fetch_add(1);
            } catch (...) {
                // Thread failed
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All threads should have succeeded
    EXPECT_EQ(success_count.load(), num_threads);
}

TEST_F(crypto_test, shared_key_operations)
{
    auto shared_key = wago::crypto::create_key();
    const int num_threads = 4;
    const int operations_per_thread = 25;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    // Multiple threads using the same key for encryption/decryption
    // (Note: key regeneration is not thread-safe, so we don't test that here)
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            try {
                for (int op = 0; op < operations_per_thread; ++op) {
                    auto ciphertext = wago::crypto::encrypt(test_plaintext_, shared_key);
                    auto decrypted = wago::crypto::decrypt(ciphertext, shared_key);
                    if (decrypted == test_plaintext_) {
                        // Success
                    } else {
                        return; // Failure
                    }
                }
                success_count.fetch_add(1);
            } catch (...) {
                // Thread failed
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All threads should have succeeded
    EXPECT_EQ(success_count.load(), num_threads);
}
