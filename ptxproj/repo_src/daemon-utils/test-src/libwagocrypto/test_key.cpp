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
///  \brief    Test WAGO crypto key class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/crypto/key.hpp"

#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <set>
#include <algorithm>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

// Friend function implementation for testing - must be in wago::crypto namespace
namespace wago {
namespace crypto {
std::vector<uint8_t> get_key_data_for_testing(key const& test_key)
{
    return test_key.key_data_m;
}
} // namespace crypto
} // namespace wago

namespace {

//------------------------------------------------------------------------------
// Test fixture
//------------------------------------------------------------------------------
class key_test : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        test_key_size_ = 32; // AES-256 key size
    }

    size_t test_key_size_;
};

//------------------------------------------------------------------------------
// Constructor tests
//------------------------------------------------------------------------------

TEST_F(key_test, constructor_creates_valid_key)
{
    // Creating a key should not throw
    EXPECT_NO_THROW({
        wago::crypto::key test_key(test_key_size_);
    });
}

TEST_F(key_test, constructor_generates_random_data)
{
    wago::crypto::key test_key(test_key_size_);
    auto key_data = wago::crypto::get_key_data_for_testing(test_key);
    
    // Key should have correct size
    EXPECT_EQ(key_data.size(), test_key_size_);
    
    // Key should not be all zeros (extremely unlikely with random generation)
    bool all_zeros = std::all_of(key_data.begin(), key_data.end(), [](uint8_t b) { return b == 0; });
    EXPECT_FALSE(all_zeros);
    
    // Key should not be all 0xFF (extremely unlikely with random generation)
    bool all_max = std::all_of(key_data.begin(), key_data.end(), [](uint8_t b) { return b == 0xFF; });
    EXPECT_FALSE(all_max);
}

TEST_F(key_test, multiple_keys_have_different_data)
{
    wago::crypto::key key1(test_key_size_);
    wago::crypto::key key2(test_key_size_);
    
    auto key1_data = wago::crypto::get_key_data_for_testing(key1);
    auto key2_data = wago::crypto::get_key_data_for_testing(key2);
    
    // Keys should have different data (extremely unlikely to be the same with random generation)
    EXPECT_NE(key1_data, key2_data);
    
    // Both should have correct size
    EXPECT_EQ(key1_data.size(), test_key_size_);
    EXPECT_EQ(key2_data.size(), test_key_size_);
}

TEST_F(key_test, constructor_with_different_sizes)
{
    // Test various key sizes
    std::vector<size_t> sizes = {16, 24, 32, 64, 128, 256};
    
    for (size_t size : sizes) {
        EXPECT_NO_THROW({
            wago::crypto::key test_key(size);
        }) << "Failed for key size: " << size;
    }
}

TEST_F(key_test, constructor_with_zero_size)
{
    // Zero size should work (empty key)
    EXPECT_NO_THROW({
        wago::crypto::key test_key(0);
        auto key_data = wago::crypto::get_key_data_for_testing(test_key);
        EXPECT_EQ(key_data.size(), 0);
        EXPECT_TRUE(key_data.empty());
    });
}

TEST_F(key_test, multiple_keys_are_independent)
{
    wago::crypto::key key1(test_key_size_);
    wago::crypto::key key2(test_key_size_);
    
    auto key1_data = wago::crypto::get_key_data_for_testing(key1);
    auto key2_data = wago::crypto::get_key_data_for_testing(key2);
    
    // Keys should be independently created with different data
    EXPECT_NE(key1_data, key2_data);
    
    // Both should have correct size
    EXPECT_EQ(key1_data.size(), test_key_size_);
    EXPECT_EQ(key2_data.size(), test_key_size_);
}

//------------------------------------------------------------------------------
// Destructor tests
//------------------------------------------------------------------------------

TEST_F(key_test, destructor_clears_key)
{
    std::vector<uint8_t> captured_key_data;
    
    // Create key in a scope and capture its data
    {
        wago::crypto::key test_key(test_key_size_);
        captured_key_data = wago::crypto::get_key_data_for_testing(test_key);
        
        // Verify key has valid data
        EXPECT_EQ(captured_key_data.size(), test_key_size_);
        bool all_zeros = std::all_of(captured_key_data.begin(), captured_key_data.end(), [](uint8_t b) { return b == 0; });
        EXPECT_FALSE(all_zeros);
        
        // Key goes out of scope here - destructor should clear the data
    }
    
    // We can't access the destroyed key's memory, but we verified the destructor doesn't crash
    SUCCEED();
}

//------------------------------------------------------------------------------
// Copy semantics tests (should be disabled)
//------------------------------------------------------------------------------

TEST_F(key_test, copy_constructor_is_deleted)
{
    // This test verifies that copy constructor is deleted
    EXPECT_TRUE(std::is_copy_constructible<wago::crypto::key>::value == false);
}

TEST_F(key_test, copy_assignment_is_deleted)
{
    // This test verifies that copy assignment is deleted
    EXPECT_TRUE(std::is_copy_assignable<wago::crypto::key>::value == false);
}

//------------------------------------------------------------------------------
// Move semantics tests
//------------------------------------------------------------------------------

TEST_F(key_test, move_constructor_works)
{
    wago::crypto::key original_key(test_key_size_);
    
    // Move construct new key
    wago::crypto::key moved_key = std::move(original_key);
    
    // Both operations should not crash
    SUCCEED();
}

TEST_F(key_test, move_assignment_works)
{
    wago::crypto::key key1(test_key_size_);
    wago::crypto::key key2(test_key_size_);
    
    // Move assign
    key1 = std::move(key2);
    
    // Should not crash
    SUCCEED();
}

TEST_F(key_test, move_constructor_transfers_ownership)
{
    auto original_key = std::make_unique<wago::crypto::key>(test_key_size_);
    auto original_data = wago::crypto::get_key_data_for_testing(*original_key);
    
    // Move the key
    wago::crypto::key moved_key = std::move(*original_key);
    auto moved_data = wago::crypto::get_key_data_for_testing(moved_key);
    
    // Moved key should have the same data as original
    EXPECT_EQ(moved_data, original_data);
    EXPECT_EQ(moved_data.size(), test_key_size_);
    
    // Original can be destroyed, moved key should still be valid
    original_key.reset();
    
    // Moved key should still be usable (test through regenerate)
    EXPECT_NO_THROW(moved_key.regenerate());
    
    // After regeneration, data should be different
    auto regenerated_data = wago::crypto::get_key_data_for_testing(moved_key);
    EXPECT_NE(regenerated_data, moved_data);
}

TEST_F(key_test, self_move_assignment)
{
    wago::crypto::key test_key(test_key_size_);
    
    // Self-move should not crash
    test_key = std::move(test_key);
    
    // Key should still be usable
    EXPECT_NO_THROW(test_key.regenerate());
}

//------------------------------------------------------------------------------
// Regenerate tests
//------------------------------------------------------------------------------

TEST_F(key_test, regenerate_works)
{
    wago::crypto::key test_key(test_key_size_);
    auto original_data = wago::crypto::get_key_data_for_testing(test_key);
    
    // Regenerate should not throw
    EXPECT_NO_THROW(test_key.regenerate());
    
    auto new_data = wago::crypto::get_key_data_for_testing(test_key);
    
    // New data should be different from original
    EXPECT_NE(new_data, original_data);
    
    // Size should remain the same
    EXPECT_EQ(new_data.size(), test_key_size_);
    
    // New data should not be all zeros
    bool all_zeros = std::all_of(new_data.begin(), new_data.end(), [](uint8_t b) { return b == 0; });
    EXPECT_FALSE(all_zeros);
}

TEST_F(key_test, regenerate_changes_data_every_time)
{
    wago::crypto::key test_key(test_key_size_);
    std::vector<std::vector<uint8_t>> key_generations;
    
    // Capture initial data
    key_generations.push_back(wago::crypto::get_key_data_for_testing(test_key));
    
    // Regenerate multiple times and capture data
    for (int i = 0; i < 5; ++i) {
        test_key.regenerate();
        key_generations.push_back(wago::crypto::get_key_data_for_testing(test_key));
    }
    
    // All generations should be different
    for (size_t i = 0; i < key_generations.size(); ++i) {
        for (size_t j = i + 1; j < key_generations.size(); ++j) {
            EXPECT_NE(key_generations[i], key_generations[j]) 
                << "Generation " << i << " and " << j << " have identical data";
        }
        
        // Each should have correct size
        EXPECT_EQ(key_generations[i].size(), test_key_size_);
    }
}

TEST_F(key_test, regenerate_after_move)
{
    wago::crypto::key original_key(test_key_size_);
    wago::crypto::key moved_key = std::move(original_key);
    
    // Should be able to regenerate moved key
    EXPECT_NO_THROW(moved_key.regenerate());
}

//------------------------------------------------------------------------------
// Rotate keys tests
//------------------------------------------------------------------------------

TEST_F(key_test, rotate_keys_works)
{
    wago::crypto::key current_key(test_key_size_);
    wago::crypto::key old_key(test_key_size_);
    
    auto original_current_data = wago::crypto::get_key_data_for_testing(current_key);
    auto original_old_data = wago::crypto::get_key_data_for_testing(old_key);
    
    // Rotate keys should not throw
    EXPECT_NO_THROW(current_key.rotate(old_key));
    
    auto new_current_data = wago::crypto::get_key_data_for_testing(current_key);
    auto new_old_data = wago::crypto::get_key_data_for_testing(old_key);
    
    // Old key should now contain the original current key data
    EXPECT_EQ(new_old_data, original_current_data);
    
    // Current key should have new, different data
    EXPECT_NE(new_current_data, original_current_data);
    EXPECT_NE(new_current_data, original_old_data);
    
    // Sizes should remain correct
    EXPECT_EQ(new_current_data.size(), test_key_size_);
    EXPECT_EQ(new_old_data.size(), test_key_size_);
}

TEST_F(key_test, rotate_keys_transfers_current_to_old)
{
    wago::crypto::key current_key(test_key_size_);
    wago::crypto::key old_key(test_key_size_);
    
    // Store the original current key data
    auto original_current_data = wago::crypto::get_key_data_for_testing(current_key);
    
    // Rotate the keys
    current_key.rotate(old_key);
    
    auto new_old_data = wago::crypto::get_key_data_for_testing(old_key);
    auto new_current_data = wago::crypto::get_key_data_for_testing(current_key);
    
    // Verify that old key now has the original current key data
    EXPECT_EQ(new_old_data, original_current_data);
    
    // Current key should have been regenerated with new data
    EXPECT_NE(new_current_data, original_current_data);
    
    // Both keys should still be valid after rotation
    EXPECT_NO_THROW(current_key.regenerate());
    EXPECT_NO_THROW(old_key.regenerate());
}

TEST_F(key_test, multiple_rotate_keys_works)
{
    wago::crypto::key current_key(test_key_size_);
    wago::crypto::key old_key1(test_key_size_);
    wago::crypto::key old_key2(test_key_size_);
    
    // Multiple rotations should work
    EXPECT_NO_THROW(current_key.rotate(old_key1));
    EXPECT_NO_THROW(current_key.rotate(old_key2));
}

TEST_F(key_test, rotate_keys_self_reference)
{
    wago::crypto::key test_key(test_key_size_);
    auto test_key_data = wago::crypto::get_key_data_for_testing(test_key);

    // Self-rotation should be handled gracefully (though not recommended)
    test_key.rotate(test_key);
    auto test_key_data_after_rotation = wago::crypto::get_key_data_for_testing(test_key);

    // Data should change after self-rotation
    EXPECT_NE(test_key_data_after_rotation, test_key_data);
}

//------------------------------------------------------------------------------
// Thread safety tests (basic)
//------------------------------------------------------------------------------

TEST_F(key_test, multiple_keys_in_threads)
{
    const int num_threads = 4;
    const int num_keys_per_thread = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            try {
                for (int k = 0; k < num_keys_per_thread; ++k) {
                    wago::crypto::key test_key(test_key_size_);
                    test_key.regenerate();
                }
                success_count++;
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

TEST_F(key_test, regenerate_in_threads)
{
    wago::crypto::key test_key(test_key_size_);
    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::mutex key_mutex; // Protect key operations since they might not be thread-safe
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            try {
                for (int i = 0; i < 5; ++i) {
                    std::lock_guard<std::mutex> lock(key_mutex);
                    test_key.regenerate();
                }
                success_count++;
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

//------------------------------------------------------------------------------
// Edge cases and boundary tests
//------------------------------------------------------------------------------

TEST_F(key_test, very_large_key_size)
{
    const size_t large_size = 1024 * 1024; // 1MB key
    
    // Should be able to create very large keys (though not practical)
    EXPECT_NO_THROW({
        wago::crypto::key large_key(large_size);
        large_key.regenerate();
    });
}

//------------------------------------------------------------------------------
// Key data verification tests
//------------------------------------------------------------------------------

TEST_F(key_test, move_assignment_preserves_data)
{
    wago::crypto::key key1(test_key_size_);
    wago::crypto::key key2(test_key_size_);
    
    auto key1_original_data = wago::crypto::get_key_data_for_testing(key1);
    auto key2_original_data = wago::crypto::get_key_data_for_testing(key2);
    
    // Verify they start with different data
    EXPECT_NE(key1_original_data, key2_original_data);
    
    // Move assign
    key1 = std::move(key2);
    
    auto key1_new_data = wago::crypto::get_key_data_for_testing(key1);
    
    // key1 should now have key2's original data
    EXPECT_EQ(key1_new_data, key2_original_data);
    EXPECT_NE(key1_new_data, key1_original_data);
}

TEST_F(key_test, rotate_keys_with_different_sizes)
{
    // This tests edge case where keys might have different sizes
    // (though in practice they should be the same size for AES)
    wago::crypto::key current_key(32);  // AES-256
    wago::crypto::key old_key(16);      // AES-128 size
    
    auto original_current_data = wago::crypto::get_key_data_for_testing(current_key);
    
    // Rotation should work even with different sizes
    EXPECT_NO_THROW(current_key.rotate(old_key));
    
    auto new_old_data = wago::crypto::get_key_data_for_testing(old_key);
    auto new_current_data = wago::crypto::get_key_data_for_testing(current_key);
    
    // Old key should now have the original current key data (32 bytes)
    EXPECT_EQ(new_old_data, original_current_data);
    EXPECT_EQ(new_old_data.size(), 32);
    
    // Current key should have new data of its original size
    EXPECT_NE(new_current_data, original_current_data);
    EXPECT_EQ(new_current_data.size(), 32);
}

TEST_F(key_test, zero_size_key_behavior)
{
    wago::crypto::key zero_key(0);
    auto key_data = wago::crypto::get_key_data_for_testing(zero_key);
    
    EXPECT_EQ(key_data.size(), 0);
    EXPECT_TRUE(key_data.empty());
    
    // Regenerate should still work
    EXPECT_NO_THROW(zero_key.regenerate());
    
    auto regenerated_data = wago::crypto::get_key_data_for_testing(zero_key);
    EXPECT_EQ(regenerated_data.size(), 0);
    EXPECT_TRUE(regenerated_data.empty());
}

TEST_F(key_test, empty_key_rotation)
{
    // Test key rotation with empty keys (size 0)
    // Although not useful in practice, these operations should not crash
    wago::crypto::key empty_current_key(0);
    wago::crypto::key empty_old_key(0);
    
    // Rotation should work without throwing, even with empty keys
    EXPECT_NO_THROW(empty_current_key.rotate(empty_old_key));
    
    // Both keys should still be empty after rotation
    auto current_data = wago::crypto::get_key_data_for_testing(empty_current_key);
    auto old_data = wago::crypto::get_key_data_for_testing(empty_old_key);
    
    EXPECT_TRUE(current_data.empty());
    EXPECT_TRUE(old_data.empty());
    EXPECT_EQ(current_data.size(), 0);
    EXPECT_EQ(old_data.size(), 0);
    
    // Regeneration should still work after rotation
    EXPECT_NO_THROW(empty_current_key.regenerate());
    EXPECT_NO_THROW(empty_old_key.regenerate());
}

TEST_F(key_test, empty_key_regeneration)
{
    // Test key regeneration with empty key (size 0)
    // Although not useful in practice, this operation should not crash
    wago::crypto::key empty_key(0);
    
    // Verify initial state
    auto initial_data = wago::crypto::get_key_data_for_testing(empty_key);
    EXPECT_TRUE(initial_data.empty());
    EXPECT_EQ(initial_data.size(), 0);
    
    // Multiple regenerations should work without throwing
    EXPECT_NO_THROW(empty_key.regenerate());
    EXPECT_NO_THROW(empty_key.regenerate());
    EXPECT_NO_THROW(empty_key.regenerate());
    
    // Key should still be empty after regeneration
    auto regenerated_data = wago::crypto::get_key_data_for_testing(empty_key);
    EXPECT_TRUE(regenerated_data.empty());
    EXPECT_EQ(regenerated_data.size(), 0);
    
    // Test regeneration after move operations
    wago::crypto::key moved_empty_key = std::move(empty_key);
    EXPECT_NO_THROW(moved_empty_key.regenerate());
    
    auto moved_data = wago::crypto::get_key_data_for_testing(moved_empty_key);
    EXPECT_TRUE(moved_data.empty());
    EXPECT_EQ(moved_data.size(), 0);
}

TEST_F(key_test, empty_key_mixed_rotation)
{
    // Test rotation between empty and normal sized keys
    wago::crypto::key empty_key(0);
    wago::crypto::key normal_key(test_key_size_);
    
    // Get initial data
    auto empty_initial = wago::crypto::get_key_data_for_testing(empty_key);
    auto normal_initial = wago::crypto::get_key_data_for_testing(normal_key);
    
    EXPECT_TRUE(empty_initial.empty());
    EXPECT_FALSE(normal_initial.empty());
    EXPECT_EQ(normal_initial.size(), test_key_size_);
    
    // Rotating empty key with normal key should work
    EXPECT_NO_THROW(empty_key.rotate(normal_key));
    
    // After rotation, empty key should have new empty data
    auto empty_after = wago::crypto::get_key_data_for_testing(empty_key);
    auto normal_after = wago::crypto::get_key_data_for_testing(normal_key);
    
    EXPECT_TRUE(empty_after.empty());
    EXPECT_EQ(empty_after.size(), 0);
    EXPECT_TRUE(normal_after.empty());
    EXPECT_EQ(normal_after.size(), 0);
    
    // After rotation, both keys should be functional for their respective operations
    EXPECT_NO_THROW(empty_key.regenerate());
    EXPECT_NO_THROW(normal_key.regenerate());
    
    // Test the reverse: normal key with empty key
    wago::crypto::key normal_key2(test_key_size_);
    wago::crypto::key empty_key2(0);
    
    auto normal2_initial = wago::crypto::get_key_data_for_testing(normal_key2);
    auto empty2_initial = wago::crypto::get_key_data_for_testing(empty_key2);
    
    EXPECT_FALSE(normal2_initial.empty());
    EXPECT_TRUE(empty2_initial.empty());
    
    EXPECT_NO_THROW(normal_key2.rotate(empty_key2));
    
    // After rotation
    auto normal2_after = wago::crypto::get_key_data_for_testing(normal_key2);
    auto empty2_after = wago::crypto::get_key_data_for_testing(empty_key2);
    
    EXPECT_NE(normal2_after, normal2_initial);  // Should have new data
    EXPECT_EQ(normal2_after.size(), test_key_size_);
    EXPECT_EQ(empty2_after, normal2_initial);   // Should have original normal key data
    EXPECT_EQ(empty2_after.size(), test_key_size_);
    
    EXPECT_NO_THROW(normal_key2.regenerate());
    EXPECT_NO_THROW(empty_key2.regenerate());
}

TEST_F(key_test, empty_key_secure_clear)
{
    // Test that secure_clear works with empty keys
    wago::crypto::key empty_key(0);
    
    // Verify initial empty state
    auto initial_data = wago::crypto::get_key_data_for_testing(empty_key);
    EXPECT_TRUE(initial_data.empty());
    EXPECT_EQ(initial_data.size(), 0);
    
    // secure_clear should work without issues
    EXPECT_NO_THROW(empty_key.secure_clear());
    
    // Should still be empty after secure_clear
    auto cleared_data = wago::crypto::get_key_data_for_testing(empty_key);
    EXPECT_TRUE(cleared_data.empty());
    EXPECT_EQ(cleared_data.size(), 0);
    
    // After secure_clear, regeneration should still work
    EXPECT_NO_THROW(empty_key.regenerate());
    
    auto regenerated_data = wago::crypto::get_key_data_for_testing(empty_key);
    EXPECT_TRUE(regenerated_data.empty());
    EXPECT_EQ(regenerated_data.size(), 0);
    
    // Multiple secure_clear calls should be safe
    EXPECT_NO_THROW(empty_key.secure_clear());
    EXPECT_NO_THROW(empty_key.secure_clear());
    
    auto final_data = wago::crypto::get_key_data_for_testing(empty_key);
    EXPECT_TRUE(final_data.empty());
    EXPECT_EQ(final_data.size(), 0);
}

TEST_F(key_test, secure_clear)
{
    wago::crypto::key test_key(test_key_size_);
    auto original_data = wago::crypto::get_key_data_for_testing(test_key);
    
    // Verify key has data initially
    EXPECT_EQ(original_data.size(), test_key_size_);
    bool all_zeros = std::all_of(original_data.begin(), original_data.end(), [](uint8_t b) { return b == 0; });
    EXPECT_FALSE(all_zeros);
    
    // Call secure_clear - should be publicly accessible now
    EXPECT_NO_THROW(test_key.secure_clear());
    
    // After secure_clear, key data should be empty
    auto cleared_data = wago::crypto::get_key_data_for_testing(test_key);
    EXPECT_TRUE(cleared_data.empty());
    EXPECT_EQ(cleared_data.size(), 0);
}

} // anonymous namespace
