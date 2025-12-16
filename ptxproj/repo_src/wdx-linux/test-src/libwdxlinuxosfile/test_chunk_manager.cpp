//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    Test chunk manager.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   Röh:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include "chunk_manager.hpp"
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::file::chunk_manager;

TEST(chunkmanager, construct_file_size)
{
    chunk_manager my_manager(123u);
}

TEST(chunkmanager, initially_incomplete)
{
    chunk_manager my_manager(123u);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, complete_with_one_chunk)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk(0, 123);
    EXPECT_TRUE(my_manager.file_completed());
}

TEST(chunkmanager, complete_with_two_chunks)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk( 0, 60);
    my_manager.add_chunk(60, 63);
    EXPECT_TRUE(my_manager.file_completed());
}

TEST(chunkmanager, complete_with_two_overlapping_chunks)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk( 0, 100);
    my_manager.add_chunk(60, 63);
    EXPECT_TRUE(my_manager.file_completed());
}

TEST(chunkmanager, incomplete_with_one_chunk_at_beginning)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk(0, 60);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, incomplete_with_one_chunk_at_end)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk(60, 63);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, incomplete_with_one_chunk_in_the_middle)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk(60, 100);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, incomplete_with_two_chunks_in_the_beginning_and_end)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk( 0, 60);
    my_manager.add_chunk(61, 62);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, incomplete_with_three_chunks_in_the_beginning_and_end_and_middle)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk( 0, 30);
    my_manager.add_chunk(32, 34);
    my_manager.add_chunk(61, 62);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, incomplete_with_overlap_but_correct_total_size)
{
    chunk_manager my_manager(123u);
    my_manager.add_chunk( 0, 60);
    my_manager.add_chunk(58, 63);
    EXPECT_FALSE(my_manager.file_completed());
}

TEST(chunkmanager, test_fail_to_optimze_if_no_chunk_is_deleted_errorneously)
{
    chunk_manager my_manager(123u);
    ASSERT_ANY_THROW({
        for(size_t i = 0; i < 123; i +=2)
        {
            my_manager.add_chunk(i, 1);
        }
    });
}

/// @brief Test to add multiple chunks even for small data.
///
/// It should be possible to upload a file with multiple
/// chunks, even if the file is not very large. The amount
/// chunks is limited by sqrt(data_size) to prevent exhaustive
/// memory consumption.
TEST(chunkmanager, test_optimize_with_multiple_chunks)
{
    constexpr size_t const data_size = 120;
    constexpr size_t const chunks = 8;
    constexpr size_t const chunk_size = data_size / chunks;

    chunk_manager my_manager(data_size);
    for(size_t i = 0; i < chunk_size; i++)
    {
        for(size_t j = 0; j < chunks; j++)
        {
            size_t const chunk_start = j * chunk_size;
            size_t const idx = chunk_start + i;
            my_manager.add_chunk(idx, 1);
        }
    }
}

TEST(chunkmanager, test_optimize_if_nr_of_chunks_is_reduced)
{
    chunk_manager my_manager(123u);
    for(size_t i = 0; i < 40; i++)
    {
        my_manager.add_chunk(i, 1);
    }
    for(size_t i = 50; i < 123; i++)
    {
        my_manager.add_chunk(i, 1);
    }
    // Should be less or sqrt(123) ~= 11
    EXPECT_EQ(my_manager.get_size() <= 11, true);
}

//---- End of source file ------------------------------------------------------
