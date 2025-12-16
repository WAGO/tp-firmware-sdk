//------------------------------------------------------------------------------
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
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
///  \brief    Test WAGO Parameter Service Core file provider compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/file_transfer/file_provider_i.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;


namespace {

class test_file_provider : public file_provider_i
{
private:
    std::vector<uint8_t> const data_m;

public:
    test_file_provider(std::vector<uint8_t> const &data)
    :data_m(data)
    {}
    ~test_file_provider() override = default;
    wago::future<file_read_response> read(uint64_t offset, size_t length) override
    {
        auto first = data_m.begin() + static_cast<ssize_t>(offset);
        auto last  = data_m.begin() + static_cast<ssize_t>(offset + length);
        std::vector<uint8_t> sub_vector(first, last);
        file_read_response response(sub_vector);
        return wago::resolved_future(std::move(response));
    }
    wago::future<response> write(uint64_t offset, std::vector<uint8_t> data) override
    {
        (void)offset; (void)data;
        return wago::resolved_future(response(wago::wdx::status_codes::success));
    }
    wago::future<file_info_response> get_file_info() override
    {
        return wago::resolved_future(file_info_response(data_m.size()));
    }
    wago::future<response> create(uint64_t capacity) override
    {
        (void)capacity;
        return wago::resolved_future(response(wago::wdx::status_codes::success));
    }
};

}

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(file_provider_i, SourceCompatibilityAndConstructDelete)
{
    test_file_provider provider({42, 53});
}

TEST(file_provider_i, SourceCompatibilityAndGetInfo)
{
    std::vector<uint8_t> const test_data = { 173, 99, 103 };
    test_file_provider provider(test_data);
    wago::future<file_info_response> future = provider.get_file_info();
    file_info_response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
    ASSERT_EQ(test_data.size(), current_response.file_size);
}

TEST(file_provider_i, SourceCompatibilityAndRead)
{
    std::vector<uint8_t> const test_data   = { 55, 11, 22 };
    uint64_t             const test_offset = 1;
    uint64_t             const test_length = 2;
    test_file_provider provider(test_data);
    wago::future<file_read_response> future = provider.read(test_offset, test_length);
    file_read_response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
    ASSERT_EQ(test_length, current_response.data.size());
    ASSERT_EQ(test_data.at(test_offset), current_response.data.at(0));
}

TEST(file_provider_i, SourceCompatibilityAndWrite)
{
    std::vector<uint8_t> const test_data   = { 55, 11, 22 };
    uint64_t             const test_offset = 1;
    test_file_provider provider(test_data);
    wago::future<response> future = provider.write(test_offset, { 0, 0 });
    response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
}

TEST(file_provider_i, SourceCompatibilityAndCreate)
{
    std::vector<uint8_t> const test_data = {};
    test_file_provider provider(test_data);
    wago::future<response> future = provider.create(1);
    response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
}

//---- End of source file ------------------------------------------------------
