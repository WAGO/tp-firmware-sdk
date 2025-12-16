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
///  \brief    Test the remote backend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_api/file_api_chunking_handler.hpp"

#include <wago/wdx/test/mock_file_api.hpp>
#include <wago/wdx/test/wda_check.hpp>

#include <wc/assertion.h>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::test;
using wago::wdx::linuxos::com::file_api_chunking_handler;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------


TEST(file_api_chunking_handler, construct_delete)
{
    mock_file_api file_api_mock;
    file_api_mock.set_default_expectations();

    ssize_t test_chunk_size = 42;

    file_api_chunking_handler chunking_handler(file_api_mock, test_chunk_size, test_chunk_size + 1);
}

TEST(file_api_chunking_handler, file_create)
{
    mock_file_api file_api_mock;
    file_api_mock.set_default_expectations();

    ssize_t     test_chunk_size = 42;
    std::string test_id         = "123";
    uint64_t    test_capacity   = 1337;

    EXPECT_CALL(file_api_mock, file_create(test_id, test_capacity))
        .Times(::testing::Exactly(1))
        .WillOnce(::testing::Return(::testing::ByMove(wago::resolved_future(wago::wdx::response(wago::wdx::status_codes::success)))));

    file_api_chunking_handler chunking_handler(file_api_mock, test_chunk_size, test_chunk_size - 1);
    auto response = chunking_handler.file_create(test_id, test_capacity).get();
    EXPECT_EQ(wago::wdx::status_codes::success, response.status);
}

TEST(file_api_chunking_handler, file_get_info)
{
    mock_file_api file_api_mock;
    file_api_mock.set_default_expectations();

    ssize_t     test_chunk_size = 42;
    std::string test_id         = "123";
    uint64_t    test_size       = 1337;

    EXPECT_CALL(file_api_mock, file_get_info(test_id))
        .Times(::testing::Exactly(1))
        .WillOnce(::testing::Return(::testing::ByMove(wago::resolved_future(wago::wdx::file_info_response(test_size)))));

    file_api_chunking_handler chunking_handler(file_api_mock, test_chunk_size, test_chunk_size);
    auto response = chunking_handler.file_get_info(test_id).get();
    EXPECT_EQ(wago::wdx::status_codes::success, response.status);
    EXPECT_EQ(test_size, response.file_size);
}

// chunksize, offset, length
class file_api_chunking_handler_fixture : public ::testing::TestWithParam<std::tuple<ssize_t, uint64_t, size_t>>
{
public:
    mock_file_api        file_api_mock;
    std::string          test_file_id     = "test123";
    ssize_t              test_chunk_size  = std::get<0>(GetParam());
    uint64_t             test_file_offset = std::get<1>(GetParam());
    size_t               test_file_length = std::get<2>(GetParam());
    std::vector<uint8_t> test_file_data   = std::vector<uint8_t>(test_file_length);
    ssize_t              test_chunk_count = static_cast<ssize_t>((test_file_length + static_cast<size_t>(test_chunk_size) - 1)
                                                                 / static_cast<size_t>(test_chunk_size));

public:
    file_api_chunking_handler chunking_handler = file_api_chunking_handler(file_api_mock, test_chunk_size, test_chunk_size);

    std::vector<uint8_t> get_test_data(uint64_t offset, uint64_t length)
    {
        auto const part_offset = static_cast<ssize_t>(offset - test_file_offset);
        auto const part_length = static_cast<ssize_t>(length);
        WC_ASSERT(static_cast<size_t>(part_offset + part_length) <= test_file_data.size());
        return std::vector<uint8_t>(test_file_data.begin() + part_offset, test_file_data.begin() + part_offset + part_length);
    }
    void SetUp() override
    {
        file_api_mock.set_default_expectations();
        uint8_t x = 0;
        for (auto &byte : test_file_data) { byte = x; ++x; }
    }
};

TEST_P(file_api_chunking_handler_fixture, file_read)
{
    auto test_data = test_file_data;
    EXPECT_CALL(file_api_mock, file_read(test_file_id, testing::_, testing::Le(test_chunk_size)))
        .Times(::testing::Exactly(test_chunk_count))
        .WillRepeatedly(::testing::Invoke([this](wago::wdx::file_id, uint64_t offset, uint64_t length){
            return wago::resolved_future(wago::wdx::file_read_response(this->get_test_data(offset, length)));
        }));

    auto result_future = chunking_handler.file_read(test_file_id, test_file_offset, test_file_length);
    ASSERT_FUTURE_VALUE(result_future);
    auto result = result_future.get();
    EXPECT_WDA_SUCCESS(result);
    EXPECT_EQ(test_file_length, result.data.size());
    EXPECT_EQ(0, memcmp(result.data.data(), test_file_data.data(), test_file_data.size()));
}

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
MATCHER_P(vectorSizeLe, size, "")
{
    return arg.size() <= static_cast<size_t>(size);
}
GNUC_DIAGNOSTIC_POP

TEST_P(file_api_chunking_handler_fixture, file_write)
{
    std::vector<uint8_t> written_data;
    EXPECT_CALL(file_api_mock, file_write(test_file_id, testing::_, vectorSizeLe(test_chunk_size)))
        .Times(::testing::Exactly(test_chunk_count))
        .WillRepeatedly(::testing::Invoke([&written_data](wago::wdx::file_id, uint64_t offset, std::vector<uint8_t> data){
            // optionally fill gap with zeroes
            for(size_t i = written_data.size(); i < offset; ++i)
            {
                written_data.push_back(0);
            }
            // set given data range
            auto internal_offset = static_cast<size_t>(offset);
            for(size_t i = internal_offset; i < static_cast<size_t>(offset + data.size()); ++i)
            {
                WC_ASSERT(i <= written_data.size());
                if(i == written_data.size())
                {
                    written_data.push_back(data[i - internal_offset]);
                }
                else
                {
                    written_data[i] = data[i - internal_offset];
                }
            }
            return wago::resolved_future(wago::wdx::response(wago::wdx::status_codes::success));
        }));

    auto result_future = chunking_handler.file_write(test_file_id, test_file_offset, test_file_data);
    ASSERT_FUTURE_VALUE(result_future);
    auto result = result_future.get();
    EXPECT_WDA_SUCCESS(result);
    EXPECT_EQ(test_file_offset + test_file_length, written_data.size());
    EXPECT_EQ(0, memcmp(written_data.data() + test_file_offset, test_file_data.data(), test_file_length));
}

// values: chunk size, offset, length
INSTANTIATE_TEST_CASE_P(file_api_chunking_handler, file_api_chunking_handler_fixture, ::testing::Values(
    std::make_tuple(42, 0, 42),    // 0: exactly one chunk
    std::make_tuple(42, 0, 84),    // 1: exactly two chunks
    std::make_tuple(42, 0, 85),    // 2: more than two chunks
    std::make_tuple(42, 0, 12),    // 3: start of file fitting in one chunk
    std::make_tuple(42, 12, 12),   // 4: random part of file fitting in one chunk
    std::make_tuple(42, 0, 48),    // 5: file larger than one chunk
    std::make_tuple(42, 12, 48),   // 6: file larger than one chunk with offset
    std::make_tuple(42, 12, 1234)//, // 7: file with offset and requiring many chunks
    // std::make_tuple(42, 12, 0)     // 8: empty
));


//---- End of source file ------------------------------------------------------
