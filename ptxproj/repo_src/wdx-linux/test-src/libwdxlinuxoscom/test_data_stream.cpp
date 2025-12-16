//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Test data stream class based on std::vector.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/data_stream.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::data_stream;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(data_stream, construct_delete)
{
    std::vector<uint8_t> test_buffer;
    data_stream teststream(test_buffer);
}

TEST(data_stream, write_read_bytes)
{
    std::vector<uint8_t> const test_data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::vector<uint8_t>       read_buffer(test_data.size());

    std::vector<uint8_t> test_buffer;
    data_stream teststream(test_buffer);

    EXPECT_FALSE(teststream.eof());
    EXPECT_FALSE(teststream.fail());
    EXPECT_FALSE(teststream.bad());
    EXPECT_TRUE(teststream.good());
    teststream.write(test_data.data(), static_cast<ssize_t>(test_data.size()));
    EXPECT_FALSE(teststream.fail());
    EXPECT_FALSE(teststream.bad());
    EXPECT_TRUE(teststream.good());
    ASSERT_EQ(test_data.size(), test_buffer.size());
    for(size_t i = 0; i < test_data.size(); ++i)
    {
        EXPECT_EQ(test_data.at(i), test_buffer.at(i));
    }
    teststream.read(read_buffer.data(), static_cast<ssize_t>(read_buffer.size()));
    for(size_t i = 0; i < test_buffer.size(); ++i)
    {
        EXPECT_EQ(test_buffer.at(i), read_buffer.at(i));
    }
    teststream.read(read_buffer.data(), 1);
    EXPECT_TRUE(teststream.eof());
    EXPECT_TRUE(teststream.fail());
    EXPECT_FALSE(teststream.good());
}

TEST(data_stream, write_get_bytes)
{
    std::vector<uint8_t> const test_data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::vector<uint8_t>       read_buffer(test_data.size());

    std::vector<uint8_t> test_buffer;
    data_stream teststream(test_buffer);

    EXPECT_FALSE(teststream.eof());
    EXPECT_FALSE(teststream.fail());
    EXPECT_FALSE(teststream.bad());
    EXPECT_TRUE(teststream.good());
    teststream.write(test_data.data(), static_cast<ssize_t>(test_data.size()));
    EXPECT_FALSE(teststream.fail());
    EXPECT_FALSE(teststream.bad());
    EXPECT_TRUE(teststream.good());
    ASSERT_EQ(test_data.size(), test_buffer.size());
    for(size_t i = 0; i < test_data.size(); ++i)
    {
        EXPECT_EQ(test_data.at(i), test_buffer.at(i));
    }
    for(uint8_t byte : test_data)
    {
        EXPECT_EQ(byte, teststream.get());
    }
    EXPECT_EQ(std::istream::traits_type::eof(), teststream.get());
    EXPECT_TRUE(teststream.eof());
    EXPECT_TRUE(teststream.fail());
    EXPECT_FALSE(teststream.good());
}

TEST(data_stream, put_read_bytes)
{
    std::vector<uint8_t> const test_data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::vector<uint8_t>       read_buffer(test_data.size());

    std::vector<uint8_t> test_buffer;
    data_stream teststream(test_buffer);

    EXPECT_FALSE(teststream.eof());
    EXPECT_FALSE(teststream.fail());
    EXPECT_FALSE(teststream.bad());
    EXPECT_TRUE(teststream.good());
    for(uint8_t byte : test_data)
    {
        teststream.put(byte);
        ASSERT_FALSE(teststream.fail());
        ASSERT_FALSE(teststream.bad());
        ASSERT_TRUE(teststream.good());
    }
    ASSERT_EQ(test_data.size(), test_buffer.size());
    for(size_t i = 0; i < test_data.size(); ++i)
    {
        EXPECT_EQ(test_data.at(i), test_buffer.at(i));
    }
    teststream.read(read_buffer.data(), static_cast<ssize_t>(read_buffer.size()));
    for(size_t i = 0; i < test_buffer.size(); ++i)
    {
        EXPECT_EQ(test_buffer.at(i), read_buffer.at(i));
    }
    teststream.read(read_buffer.data(), 1);
    EXPECT_TRUE(teststream.eof());
    EXPECT_TRUE(teststream.fail());
    EXPECT_FALSE(teststream.good());
}

TEST(data_stream, put_get_bytes)
{
    std::vector<uint8_t> const test_data = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    std::vector<uint8_t>       read_buffer(test_data.size());

    std::vector<uint8_t> test_buffer;
    data_stream teststream(test_buffer);

    EXPECT_FALSE(teststream.eof());
    EXPECT_FALSE(teststream.fail());
    EXPECT_FALSE(teststream.bad());
    EXPECT_TRUE(teststream.good());
    for(uint8_t byte : test_data)
    {
        teststream.put(byte);
        ASSERT_FALSE(teststream.fail());
        ASSERT_FALSE(teststream.bad());
        ASSERT_TRUE(teststream.good());
    }
    ASSERT_EQ(test_data.size(), test_buffer.size());
    for(size_t i = 0; i < test_data.size(); ++i)
    {
        EXPECT_EQ(test_data.at(i), test_buffer.at(i));
    }
    for(uint8_t byte : test_data)
    {
        EXPECT_EQ(byte, teststream.get());
    }
    EXPECT_EQ(std::istream::traits_type::eof(), teststream.get());
    EXPECT_TRUE(teststream.eof());
    EXPECT_TRUE(teststream.fail());
    EXPECT_FALSE(teststream.good());
}


//---- End of source file ------------------------------------------------------
