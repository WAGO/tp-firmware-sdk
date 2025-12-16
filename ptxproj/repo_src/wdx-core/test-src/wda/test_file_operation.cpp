//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test file operations.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "files/file_operation.hpp"

#include "mocks/mock_file_api.hpp"
#include "mocks/mock_request.hpp"

#include <wc/structuring.h>
#include <gtest/gtest.h>

#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::files;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static void compare_memory(uint8_t const *data_begin,
                           uint64_t       offset,
                           uint8_t const *actual,
                           size_t         length);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char http_endl[]              = "\r\n";
static constexpr char binary_content_type[]    = "application/octet-stream";
static constexpr char multipart_content_type[] = "multipart/byteranges";
constexpr char const * invalid_ranges[] = {
    "bytes=a-z",
    "bytes= 1-2",
    "bytes =1-2",
    "bytes = 1-2",
    " bytes=1-2",
    "1-2",
    "bytes=1 - 2",
    "byte=1-2",
    "range=1-2",
    "none",
    "bytes 1-2",
    "bytes:1-2",
    "bytes: 1-2",
    "bytes: 18446744073709551615-18446744073709551615", // uint64max = 18446744073709551615
    "bytes: 1-18446744073709551616"
};

constexpr char const * invalid_content_ranges[] = {
    "bytes a-z",
    "bytes 1-2", // total length missing
    "bytes  1-2/3", // double space
    "bytes = 1-2/3", // = separator
    "bytes : 1-2/3", // : separator
    " bytes 1-2/3" // leading space
    "1-2/3", // no unit
    "bytes 1 - 2", // spaces between range limits
    "none", // unsupported unit "none"
    "bytes 18446744073709551615-18446744073709551615/18446744073709551616"
};

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class fake_streambuf : public std::streambuf
{
private:
    uint8_t  const *content;
    uint64_t        content_length;
    uint64_t        total_length;
    uint64_t        current_offset = 0;
public:
    fake_streambuf(uint8_t const *fake_content, uint64_t fake_content_length, uint64_t fake_total_length)
    : content(fake_content)
    , content_length(std::min(fake_content_length, fake_total_length))
    , total_length(fake_total_length)
    { }

    std::streambuf::int_type underflow() override
    {
        // faked total length exceeded
        if(current_offset >= total_length)
        {
            return std::streambuf::traits_type::eof();
        }
        // will always provide the same bytes
        setg(reinterpret_cast<char *>(const_cast<uint8_t *>(content)),
             reinterpret_cast<char *>(const_cast<uint8_t *>(content)),
             reinterpret_cast<char *>(const_cast<uint8_t *>(content + content_length)));
        current_offset += content_length;
        return std::streambuf::traits_type::to_int_type(*gptr());
    }
};

class fake_stream : public std::istream
{
private:
    fake_streambuf buff;
public:
    fake_stream(uint8_t const *fake_content, uint64_t fake_content_length, uint64_t fake_total_length)
    : std::istream(&buff)
    , buff(fake_content, fake_content_length, fake_total_length)
    { }
};

class file_operation_fixture : public ::testing::Test
{
public:
    std::unique_ptr<mock_request>         request_mock;
    std::shared_ptr<mock_file_api>        file_api_mock;
    size_t                                trash_size;
    uint8_t                        const *trash;

protected:
    std::mutex              thread_container_mutex;
    std::queue<std::thread> active_threads;

    file_operation_fixture()
    : trash_size(10 * 1024 * 1024), trash(nullptr)
    {
        uint8_t * new_trash = new uint8_t[trash_size];
        for(size_t i = 0; i < trash_size; ++i)
        {
            new_trash[i] = static_cast<uint8_t>(i + 1 % 201);
        }
        trash = new_trash;
    }

    ~file_operation_fixture()
    {
        delete[] trash;
    }

    void SetUp() override
    {
        request_mock = std::make_unique<mock_request>();
        request_mock->set_default_expectations();

        file_api_mock = std::make_shared<mock_file_api>();
        file_api_mock->set_default_expectations();
    }

    void TearDown() override
    {
        request_mock  = nullptr;
        file_api_mock = nullptr;
    }

    void wait_for_threads_to_finish()
    {
        bool queue_emtpy;
        do
        {
            std::thread front_thread;
            {
                std::lock_guard<std::mutex> thread_container_lock(thread_container_mutex);
                queue_emtpy = active_threads.empty();
                if(!queue_emtpy)
                {
                    front_thread = std::move(active_threads.front());
                    active_threads.pop();
                }
            }
            if(front_thread.joinable())
            {
                front_thread.join();
            }

            std::lock_guard<std::mutex> thread_container_lock(thread_container_mutex);
            queue_emtpy = active_threads.empty();
        }
        while(!queue_emtpy);
    }

    bool request_is_responded;
    void add_response_expectations(http_status_code expected_status_code,
                                   std::string      expected_content_length = "",
                                   std::string      expected_content_type   = "",
                                   std::string      expected_content_range  = "")
    {
        request_is_responded = false;
        EXPECT_CALL(*request_mock, is_responded())
            .Times(AnyNumber())
            .WillRepeatedly(Invoke([this]() { return request_is_responded; }));
        EXPECT_CALL(*request_mock, respond_mock(::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(WithArgs<0>(Invoke([
                    expected_status_code, 
                    expected_content_length, 
                    expected_content_type, 
                    expected_content_range,
                    this
                ] (response_i const &response) {
                EXPECT_EQ(expected_status_code, response.get_status_code());
                EXPECT_EQ(expected_content_type, response.get_content_type());
                EXPECT_EQ(expected_content_length, response.get_content_length());
                auto const &headers = response.get_response_header();
                EXPECT_EQ(!expected_content_range.empty(), headers.count("Content-Range") > 0);
                if(headers.count("Content-Range"))
                {
                    EXPECT_EQ(headers.at("Content-Range"), expected_content_range);
                }
                request_is_responded = true;
            })));
    }

    void test_get_file_info(uint64_t test_file_size)
    {
        std::string        test_file_id   = "test_file_id";

        http_status_code expected_status_code    = http_status_code::ok;
        std::string      expected_content_length = std::to_string(test_file_size);
        std::string      expected_content_type   = std::string(binary_content_type);

        // expect core file api calls
        EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
            .Times(Exactly(1))
            .WillRepeatedly(Invoke([test_file_size](file_id) { return wago::resolved_future(file_info_response(test_file_size)); }));
            // Return macro does not work for move-only types such as the future:
            //.WillRepeatedly(Return(wago::resolved_future(file_info_response(test_file_size)))); 

        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(false));
        add_response_expectations(expected_status_code, 
                                  expected_content_length, 
                                  expected_content_type);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_info(test_file_id, std::move(request_mock));
        }
    }

    void test_get_file_info(uint64_t test_file_size, uint64_t test_range_from, uint64_t test_range_to)
    {
        std::string        test_file_id      = "test_file_id";
        uint64_t           test_range_length = 1 + test_range_to - test_range_from;
        std::string        test_range_header = "bytes=" + std::to_string(test_range_from) + "-" + std::to_string(test_range_to);

        // range valid?
        bool range_is_bad   = test_range_to >= UINT64_MAX || test_range_to < test_range_from;
        bool range_is_valid = test_range_from < test_file_size;

        http_status_code expected_status_code    = range_is_bad || !range_is_valid
                                                        ? http_status_code::range_not_satisfiable
                                                        : http_status_code::partial_content;
        std::string      expected_content_length = (!range_is_bad && range_is_valid) ? std::to_string(test_range_length) : "";
        std::string      expected_content_type   = (!range_is_bad && range_is_valid) ? std::string(binary_content_type)  : "";
        std::string      expected_content_range  = (!range_is_bad && range_is_valid) ? "bytes " + std::to_string(test_range_from) + "-" + std::to_string(test_range_to) + "/" + std::to_string(test_file_size) : "";

        // expect core file api calls
        if(!range_is_bad)
        {
            EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
                .Times(Exactly(1))
                .WillRepeatedly(Invoke([test_file_size](file_id) { return wago::resolved_future(file_info_response(test_file_size)); }));
        }
        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_range_header));
        add_response_expectations(expected_status_code, 
                                  expected_content_length, 
                                  expected_content_type,
                                  expected_content_range);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_info(test_file_id, std::move(request_mock));
        }
    }

    void test_get_file_info(wago::wdx::status_codes test_core_status_code, 
                            http_status_code        expected_status_code)
    {
        std::string test_file_id = "test_file_id";

        // expect core file api calls
        EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
            .Times(Exactly(1))
            .WillRepeatedly(Invoke([test_core_status_code](file_id) { return wago::resolved_future(file_info_response(test_core_status_code)); }));

        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(false));
        add_response_expectations(expected_status_code);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_info(test_file_id, std::move(request_mock));
        }
    }

    void test_get_file_info(std::string test_invalid_range)
    {
        std::string test_file_id = "file123";
        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_invalid_range.c_str()));
        add_response_expectations(http_status_code::range_not_satisfiable);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_info(test_file_id, std::move(request_mock));
        }
    }

    void test_get_file_content(uint64_t test_file_size)
    {
        std::string      test_file_id                = "test_file_id";
        uint64_t         test_file_size_with_content = std::min(static_cast<uint64_t>(trash_size), test_file_size);
        bool             max_exceeded                = test_file_size > max_download_data_length;
        http_status_code max_error                   = http_status_code::range_not_satisfiable;
        http_status_code expected_status_code        = max_exceeded ? max_error : http_status_code::ok;
        std::string      expected_content_length     = max_exceeded ? ""        : std::to_string(test_file_size);
        std::string      expected_content_type       = max_exceeded ? ""        : binary_content_type;
        uint64_t         calculated_number_of_chunks = ((test_file_size + download_chunk_size - 1) / download_chunk_size);
        assert(calculated_number_of_chunks <= INT_MAX);
        int              expected_number_of_chunks   = (int)calculated_number_of_chunks;

        // expect core file api calls
        EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
            .Times(Exactly(1))
            .WillRepeatedly(Invoke([test_file_size](file_id) { return wago::resolved_future(file_info_response(test_file_size)); }));

        // Expect chunkwise calls
        if(test_file_size <= max_download_data_length)
        {
            EXPECT_CALL(*file_api_mock, file_read(test_file_id, ::testing::_, ::testing::Le(download_chunk_size)))
                .Times(Exactly(expected_number_of_chunks))
                .WillRepeatedly(Invoke([test_class=this, test_file_size_with_content](file_id, auto offset, auto length) {
                    auto data_promise = std::make_shared<wago::promise<file_read_response>>();
                    auto data_future = data_promise->get_future();
                    std::lock_guard<std::mutex> thread_container_lock(test_class->thread_container_mutex);
                    test_class->active_threads.emplace([test_class, offset, length, test_file_size_with_content, data_promise]() {
                        if((offset + length) <= test_file_size_with_content)
                        {
                            std::vector<uint8_t> data(test_class->trash + offset, test_class->trash + offset + length);
                            data_promise->set_value(file_read_response(data));
                        }
                        else
                        {
                            std::vector<uint8_t> data(static_cast<size_t>(length), 0); // no specific content
                            data_promise->set_value(file_read_response(data));
                        }
                        return;
                    });
                    return data_future;
                }));
        }

        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(false));
        add_response_expectations(expected_status_code,
                                  expected_content_length,
                                  expected_content_type);

        if(test_file_size <= max_download_data_length)
        {
            size_t offset = 0;
            EXPECT_CALL(*request_mock, send_data(::testing::_, ::testing::Le(download_chunk_size)))
                .Times(Exactly(expected_number_of_chunks))
                .WillRepeatedly(Invoke([file=trash, &offset, test_file_size_with_content] (char const *data, size_t length) {
                    if((offset + length) <= test_file_size_with_content)
                    {
                        EXPECT_TRUE(0 == ::memcmp(file + offset, data, length));
                        offset += length;
                    }

                }));
        }

        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_content(test_file_id, std::move(request_mock));
        }

        wait_for_threads_to_finish();
    }

    void test_get_file_content(uint64_t test_file_size, uint64_t test_range_from, uint64_t test_range_to)
    {
        std::string test_file_id   = "test_file_id";
        std::string test_range_header = "bytes=" + std::to_string(test_range_from) + "-" + std::to_string(test_range_to);
        uint64_t    test_file_size_with_content = std::min(static_cast<uint64_t>(trash_size), test_file_size);
        // range valid?
        bool range_is_bad   = test_range_to >= UINT64_MAX || test_range_to < test_range_from;
        bool range_is_valid = test_range_from < test_file_size;

        uint64_t         expected_range_to       = std::min(test_range_to, test_file_size - 1);
        uint64_t         download_size           = 1 + expected_range_to - test_range_from;
        bool             max_exceeded            = download_size > max_download_data_length;
        http_status_code expected_status_code    = range_is_bad || !range_is_valid || max_exceeded
                                                       ? http_status_code::range_not_satisfiable
                                                       : http_status_code::partial_content;
        std::string      expected_content_type   = (!range_is_bad && range_is_valid && !max_exceeded)
                                                       ? std::string(binary_content_type)  : "";
        std::string      expected_content_length = (!range_is_bad && range_is_valid && !max_exceeded)
                                                       ? std::to_string(download_size) : "";
        std::string      expected_content_range  = (!range_is_bad && range_is_valid && !max_exceeded)
                                                       ? "bytes " + std::to_string(test_range_from) + "-" + std::to_string(expected_range_to) + "/" + std::to_string(test_file_size) : "";

        uint64_t calculated_number_of_chunks = ((download_size + download_chunk_size - 1) / download_chunk_size);
        assert(calculated_number_of_chunks <= INT_MAX);
        int      expected_number_of_chunks   = (!range_is_bad && range_is_valid && !max_exceeded) ? (int)calculated_number_of_chunks : 0;

        // expect core file api calls
        if(!range_is_bad)
        {
            EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
                .Times(Exactly(1))
                .WillRepeatedly(Invoke([test_file_size](file_id) { return wago::resolved_future(file_info_response(test_file_size)); }));
        }

        // Expect chunkwise calls
        if(expected_number_of_chunks > 0)
        {
            EXPECT_CALL(*file_api_mock, file_read(test_file_id, ::testing::_, ::testing::Le(download_chunk_size)))
                .Times(Exactly(expected_number_of_chunks))
                .WillRepeatedly(Invoke([test_class=this, test_range_from, test_file_size_with_content](file_id, auto offset, auto length) {
                    auto data_promise = std::make_shared<wago::promise<file_read_response>>();
                    auto data_future = data_promise->get_future();
                    std::lock_guard<std::mutex> thread_container_lock(test_class->thread_container_mutex);
                    test_class->active_threads.emplace([test_class, offset=(offset-test_range_from), length, test_file_size_with_content, data_promise]() {
                        if((offset + length) <= test_file_size_with_content)
                        {
                            std::vector<uint8_t> data(test_class->trash + offset, test_class->trash + offset + length);
                            data_promise->set_value(file_read_response(data));
                        }
                        else
                        {
                            std::vector<uint8_t> data(static_cast<size_t>(length), 0); // no specific content
                            data_promise->set_value(file_read_response(data));
                        }
                        return;
                    });
                    return data_future;
                }));
        }

        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_range_header));
        add_response_expectations(expected_status_code,
                                  expected_content_length,
                                  expected_content_type,
                                  expected_content_range);
        size_t offset = 0;
        if(expected_number_of_chunks > 0)
        {
            EXPECT_CALL(*request_mock, send_data(::testing::_, ::testing::Le(download_chunk_size)))
                .Times(Exactly(expected_number_of_chunks))
                .WillRepeatedly(Invoke([&offset, file=trash, test_file_size_with_content] (char const *data, size_t length) {
                    if((offset + length) <= test_file_size_with_content)
                    {
                        EXPECT_TRUE(0 == ::memcmp(file + offset, data, length));
                        offset += length;
                    }
                    
                }));
        }
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_content(test_file_id, std::move(request_mock));
        }

        wait_for_threads_to_finish();
    }

    void test_get_file_content(wago::wdx::status_codes test_core_status_code_for_file_info, 
                               http_status_code        expected_status_code)
    {
        std::string test_file_id = "test_file_id";

        // expect core file api calls
        EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
            .Times(Exactly(1))
            .WillRepeatedly(Invoke([test_core_status_code_for_file_info](file_id) { return wago::resolved_future(file_info_response(test_core_status_code_for_file_info)); }));

        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(false));
        add_response_expectations(expected_status_code);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_content(test_file_id, std::move(request_mock));
        }
    }

    void test_get_file_content(std::string test_invalid_range)
    {
        std::string test_file_id = "file123";
        // expect http request calls
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(true));
        EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_invalid_range.c_str()));
        add_response_expectations(http_status_code::range_not_satisfiable);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.get_file_content(test_file_id, std::move(request_mock));
        }
    }

    void test_set_file_content(uint64_t test_file_size, std::string test_content_type = binary_content_type)
    {
        test_set_file_content(test_file_size, test_file_size, 0, test_content_type);
    }

    void test_set_file_content(uint64_t test_file_size, uint64_t test_content_size, uint64_t test_existing_file_size = 0, std::string test_content_type = binary_content_type)
    {
        std::string       test_file_id                = "test_file_id";
        uint64_t          test_file_size_with_content = std::min(static_cast<uint64_t>(trash_size), test_file_size);
        uint64_t          test_content_size_to_check  = std::min(test_file_size_with_content, test_content_size);
        std::string       test_content_length         = std::to_string(test_file_size);
        fake_stream       test_content_stream(trash, test_file_size_with_content, test_content_size);
        bool max_exceeded                             = test_content_size > max_upload_request_length;
        bool too_few_content                          = test_content_size < test_file_size;
        bool incorrect_content_type                   = test_content_type != binary_content_type;
        http_status_code  expected_status_code        = incorrect_content_type 
                                                        ? http_status_code::unsupported_media_type
                                                        : too_few_content 
                                                            ? http_status_code::bad_request
                                                            : max_exceeded
                                                                ? http_status_code::payload_too_large
                                                                : http_status_code::no_content;
        uint64_t          calculated_number_of_chunks = ((test_content_size + upload_chunk_size - 1) / upload_chunk_size);
        assert(calculated_number_of_chunks <= INT_MAX);
        int               expected_number_of_chunks   = (int)calculated_number_of_chunks;

        if(!max_exceeded && !incorrect_content_type)
        {
            // expect core file api calls
            EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
                .Times(AnyNumber()) // FIXME: actually, this is expected to be called once.
                .WillRepeatedly(Invoke([test_existing_file_size](file_id) { 
                    return wago::resolved_future(file_info_response(test_existing_file_size)); 
                }));
            EXPECT_CALL(*file_api_mock, file_create(test_file_id, test_file_size))
                .Times(Exactly(1))
                .WillRepeatedly(Invoke([](file_id, uint64_t) { return wago::resolved_future(wago::wdx::response(wago::wdx::status_codes::success)); }));

            // Expect chunkwise calls
            if(expected_number_of_chunks > 0)
            {
                EXPECT_CALL(*file_api_mock, file_write(test_file_id, ::testing::_, ::testing::_))
                    .Times(Exactly(expected_number_of_chunks))
                    .WillRepeatedly(Invoke([test_class=this, test_content_size_to_check](file_id, auto offset, auto data) {
                        size_t length = data.size();
                        EXPECT_GE(upload_chunk_size, length);
                        EXPECT_TRUE(length <= SIZE_MAX);
                        auto data_promise = std::make_shared<wago::promise<wago::wdx::response>>();
                        auto data_future = data_promise->get_future();
                        std::lock_guard<std::mutex> thread_container_lock(test_class->thread_container_mutex);
                        test_class->active_threads.emplace([test_class, offset, length, data, test_content_size_to_check, data_promise]() {
                            if((offset + length) <= test_content_size_to_check)
                            {
                                compare_memory(test_class->trash, offset, data.data(), length);
                            }
                            wago::wdx::response res;
                            res.status = wago::wdx::status_codes::success;
                            data_promise->set_value(std::move(res));
                        });
                        return data_future;
                    }));
            };
        }
        add_response_expectations(expected_status_code);

        // expect http request calls
        EXPECT_CALL(*request_mock, get_content_length())
            .Times(AtLeast(1))
            .WillRepeatedly(Return(test_content_length.c_str()));
        if(!max_exceeded)
        {
            EXPECT_CALL(*request_mock, get_content_type())
                .Times(AtLeast(1))
                .WillRepeatedly(Return(test_content_type.c_str()));
        }
        if(!max_exceeded && !incorrect_content_type)
        {
            EXPECT_CALL(*request_mock, get_method())
                .Times(AtLeast(1))
                .WillRepeatedly(Return(http_method::put));
            EXPECT_CALL(*request_mock, has_http_header(testing::StrEq("Content-Range")))
                .Times(AtLeast(1))
                .WillRepeatedly(Return(false));
            EXPECT_CALL(*request_mock, get_content_stream())
                .Times(Exactly(1))
                .WillRepeatedly(Invoke([&test_content_stream]() {
                    return std::ref(test_content_stream);
                }));
            EXPECT_CALL(*request_mock, destructor())
                .Times(Exactly(1));
        }
        else
        {
            EXPECT_CALL(*request_mock, get_method())
                .Times(AnyNumber())
                .WillRepeatedly(Return(http_method::put));
        }

        {
            file_operation operation(file_api_mock);
            operation.set_file_content(test_file_id, std::move(request_mock));
        }
        wait_for_threads_to_finish();
    }

    void test_set_file_content(wago::wdx::status_codes test_core_status_code_for_file_create, 
                               http_status_code        expected_status_code)
    {
        std::string test_file_id = "test_file_id";
        std::string test_content_type = binary_content_type;
        std::string test_content_length = "123"; // something > 0

        // expect core file api calls
        EXPECT_CALL(*file_api_mock, file_create(test_file_id, ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(Invoke([test_core_status_code_for_file_create](file_id, auto) { 
                    wago::wdx::response res;
                    res.status = test_core_status_code_for_file_create;
                    return wago::resolved_future(std::move(res)); 
                }));

        // expect http request calls
        EXPECT_CALL(*request_mock, get_method())
            .Times(Exactly(1))
            .WillRepeatedly(Return(http_method::put));
        EXPECT_CALL(*request_mock, get_content_type())
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_content_type.c_str()));
        EXPECT_CALL(*request_mock, get_content_length())
            .Times(Exactly(1))
            .WillRepeatedly(Return(test_content_length.c_str()));
        EXPECT_CALL(*request_mock, has_http_header(::testing::StrEq("Content-Range")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(false));
        add_response_expectations(expected_status_code);
        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.set_file_content(test_file_id, std::move(request_mock));
        }
    }

    struct test_part
    {
        uint64_t range_from;
        uint64_t range_to;
        uint64_t content_size;
        std::string content_type;
        std::string content_range;
        test_part(uint64_t range_from_,
                  uint64_t range_to_,
                  uint64_t content_size_,
                  std::string content_type_,
                  std::string content_range_)
        : range_from(range_from_)
        , range_to(range_to_)
        , content_size(content_size_)
        , content_type(content_type_)
        , content_range(content_range_)
        { }
        test_part(uint64_t range_from_,
                  uint64_t range_to_,
                  std::string content_type_  = "",
                  std::string content_range_ = "")
        : test_part(range_from_, range_to_, 1 + range_to_ - range_from_, content_type_, content_range_)
        { }

        static std::stringstream create_stream(uint8_t                const *trash,
                                               uint64_t                      trash_size,
                                               std::vector<test_part>        parts,
                                               std::string                   boundary,
                                               uint64_t                      total_file_size,
                                               std::string                   line_end)
        {
            std::stringstream stream;
            stream << "--" << boundary << line_end;
            for (size_t i = 0; i < parts.size(); ++i)
            {
                auto const& part = parts.at(i);
                if(part.content_range.empty())
                {
                    stream << "Content-Range: bytes " << part.range_from << "-" << part.range_to <<  "/" << total_file_size << line_end;
                }
                else
                {
                    stream << "Content-Range: " << part.content_range << line_end;
                }
                if(part.content_type.empty())
                {
                    stream << "Content-Type: " << binary_content_type << line_end;
                }
                else
                {
                    stream << "Content-Type: " << part.content_type << line_end;
                }

                stream << line_end;

                uint8_t const *data = trash + part.range_from;
                EXPECT_GE(INT_MAX, trash_size);
                uint64_t chunk_to_write = std::min(part.content_size, trash_size - part.range_from);
                if((part.range_from < UINT32_MAX) && (data < (trash + trash_size)))
                {
                    EXPECT_GE(INT_MAX, chunk_to_write);
                    stream.write(reinterpret_cast<char const *>(data), static_cast<int>(chunk_to_write));
                    data += chunk_to_write;
                }
                while(data <= trash + part.range_to)
                {
                    std::streamsize length_to_write = std::min(static_cast<std::streamsize>(trash + part.range_to - data + 1),
                                                               static_cast<std::streamsize>(trash_size));
                    stream.write(reinterpret_cast<char const *>(trash), length_to_write);
                    data += length_to_write;
                }

                bool is_last = (i == parts.size() - 1);
                stream << line_end << "--" << boundary << (is_last ? "--" : "") << line_end;
            }
            return stream;
        }
    };

    std::string get_stringstream_length(std::stringstream &stream)
    {
        stream.seekp(0, std::ios::end);
        auto stream_length = stream.tellp();

        return std::to_string(stream_length);
    }

    void test_set_file_content(uint64_t test_file_size, std::vector<test_part> test_parts, std::string test_boundary,
                               std::string line_end = http_endl, uint64_t test_existing_file_size = 0, std::string test_content_type = "",
                               wago::wdx::status_codes test_core_status_code_for_write = wago::wdx::status_codes::success,
                               http_status_code expected_status_code = http_status_code::no_content)
    {
        for(auto const &part : test_parts)
        {
            ASSERT_TRUE(test_file_size >= part.content_size);
        }
        std::string       test_file_id                = "test_file_id";
        uint64_t          test_file_size_with_content = std::min(static_cast<uint64_t>(trash_size), test_file_size);
        std::string       correct_content_type        = std::string(multipart_content_type) + "; boundary=" + test_boundary;
        if(test_content_type.empty())
        {
            test_content_type = correct_content_type;
        }
        std::stringstream test_content_stream         = test_part::create_stream(trash, trash_size, test_parts,
                                                                                 test_boundary, test_file_size, line_end);
        std::string       test_content_stream_length  = get_stringstream_length(test_content_stream);


        uint64_t          calculated_number_of_chunks = 0;
        bool     const    max_exceeded                = (std::stoull(test_content_stream_length) > max_upload_request_length);

        if(!max_exceeded)
        {

            if(test_core_status_code_for_write != wago::wdx::status_codes::success)
            {
                calculated_number_of_chunks = 1;
            }
            else
            {
                for(auto const &part : test_parts)
                {
                    calculated_number_of_chunks += ((part.content_size + upload_chunk_size - 1) / upload_chunk_size);
                }
            }
            ASSERT_TRUE(calculated_number_of_chunks <= INT_MAX);
        }

        bool              errorneous_content_type_in_part_one  = !test_parts.at(0).content_type.empty();
        bool              errorneous_content_range_in_part_one = !test_parts.at(0).content_range.empty();
        bool              errorneous_part_one                  = errorneous_content_type_in_part_one || errorneous_content_range_in_part_one;

        bool              incorrect_content_type      = test_content_type != correct_content_type;
                          expected_status_code        = incorrect_content_type
                                                        ? http_status_code::unsupported_media_type
                                                        : max_exceeded 
                                                            ? http_status_code::payload_too_large 
                                                            : errorneous_content_type_in_part_one 
                                                                ? http_status_code::unsupported_media_type
                                                                : errorneous_content_range_in_part_one 
                                                                    ? http_status_code::bad_request
                                                                    : expected_status_code;
        int               expected_number_of_chunks   = (int)calculated_number_of_chunks;
        bool              expect_create_file_call     = test_existing_file_size == 0;

        if(!errorneous_part_one && !incorrect_content_type && !max_exceeded)
        {
            // expect core file api calls
            EXPECT_CALL(*file_api_mock, file_get_info(test_file_id))
                .Times(Exactly(1)) 
                .WillRepeatedly(Invoke([&test_existing_file_size](file_id) { return wago::resolved_future(file_info_response(test_existing_file_size)); }));
            
            if(expect_create_file_call)
            {
                EXPECT_CALL(*file_api_mock, file_create(test_file_id, test_file_size))
                    .Times(Exactly(1))
                    .WillRepeatedly(Invoke([](file_id, uint64_t) { return wago::resolved_future(wago::wdx::response(wago::wdx::status_codes::success)); }));
            }
            // Expect chunkwise calls
            if(expected_number_of_chunks > 0)
            {
                EXPECT_CALL(*file_api_mock, file_write(test_file_id, ::testing::_, ::testing::_))
                    .Times(Exactly(expected_number_of_chunks))
                    .WillRepeatedly(Invoke([test_class=this, test_core_status_code_for_write, test_file_size_with_content](file_id, auto offset, auto data) {
                        size_t length = data.size();
                        EXPECT_GE(upload_chunk_size, length);
                        EXPECT_TRUE(length <= SIZE_MAX);
                        auto data_promise = std::make_shared<wago::promise<wago::wdx::response>>();
                        auto data_future = data_promise->get_future();
                        std::lock_guard<std::mutex> thread_container_lock(test_class->thread_container_mutex);
                        test_class->active_threads.emplace([test_class, test_core_status_code_for_write, offset, length, data, test_file_size_with_content, data_promise]() {
                            if((offset + length) <= test_file_size_with_content)
                            {
                                EXPECT_TRUE(0 == ::memcmp(test_class->trash + offset, data.data(), length));
                            }
                            wago::wdx::response res;
                            res.status = test_core_status_code_for_write;
                            data_promise->set_value(std::move(res));
                        });
                        return data_future;
                    }));
            };
        }

        // expect http request calls
        add_response_expectations(expected_status_code);
        EXPECT_CALL(*request_mock, get_content_length())
                .Times(AnyNumber())
                .WillRepeatedly(Return(test_content_stream_length.c_str()));
        if(!max_exceeded)
        {
            EXPECT_CALL(*request_mock, get_content_type())
                .Times(AtLeast(1))
                .WillRepeatedly(Return(test_content_type.c_str()));
        }
        if(!incorrect_content_type && !max_exceeded)
        {
            EXPECT_CALL(*request_mock, get_method())
                .Times(AtLeast(1))
                .WillRepeatedly(Return(http_method::patch));
            EXPECT_CALL(*request_mock, get_content_stream())
                .Times(Exactly(1))
                .WillRepeatedly(Invoke([&test_content_stream]() { 
                    return std::ref(test_content_stream); 
                }));
        }
        else
        {
            EXPECT_CALL(*request_mock, get_method())
                .Times(AnyNumber())
                .WillRepeatedly(Return(http_method::patch));
        }

        EXPECT_CALL(*request_mock, destructor())
            .Times(Exactly(1));
        {
            file_operation operation(file_api_mock);
            operation.set_file_content(test_file_id, std::move(request_mock));
        }

        wait_for_threads_to_finish();
    }
};

static void compare_memory(uint8_t const *data_begin,
                           uint64_t       offset,
                           uint8_t const *actual,
                           size_t         length)
{
    uint8_t const * expected = data_begin + offset;
    if(0 != ::memcmp(expected, actual, length))
    {
        std::stringstream error_detail;
        error_detail << " Details: " << "offset=" << offset << " length=" << length << std::endl;
        size_t fail_index;
        for(fail_index = 0; fail_index < length; ++fail_index)
        {
            if(expected[fail_index] != actual[fail_index])
            {
                break;
            }
        }
        error_detail << "Fail-Idx: " << fail_index << std::endl;
        size_t start_index = fail_index < 8 ? 0 : fail_index;
        size_t cut_length  = start_index == 0 ? 16 : 8;
        if((length - start_index) < cut_length)
        {
            cut_length = (length - start_index);
        }
        uint8_t  const * const data_vectors[]                  = { actual, expected };
        constexpr size_t const vector_count                    = sizeof(data_vectors)/WC_SIZEOF_ELEMENT(data_vectors);
        char     const * const data_vector_names[vector_count] = { "  Actual", "Expected" };
        for(size_t vec_i = 0; vec_i < vector_count; ++vec_i)
        {
            error_detail << data_vector_names[vec_i] << ":";
            if(start_index > 0)
            {
                error_detail << " [...]";
            }
            for(size_t i = start_index; (i < (start_index + cut_length)) && (i < length); ++i)
            {
                error_detail << " 0x" << std::setfill('0') << std::setw(2) << std::right << std::hex
                             << static_cast<unsigned>(data_vectors[vec_i][i]);
            }
            if((start_index + cut_length) < length)
            {
                error_detail << " ...";
            }
            error_detail << std::endl;
        }

        FAIL() << error_detail.str();
    }
}


TEST_F(file_operation_fixture, construct_delete)
{
    file_operation operation(file_api_mock);
}

TEST_F(file_operation_fixture, get_file_info)
{
    test_get_file_info(1024);
}

TEST_F(file_operation_fixture, get_file_info_large_file)
{
    test_get_file_info(UINT64_MAX);
}

TEST_F(file_operation_fixture, get_file_info_for_range)
{
    test_get_file_info(1024, 1, 10);
}

TEST_F(file_operation_fixture, get_file_info_for_range_large_file)
{
    test_get_file_info(UINT64_MAX, 1, 10);
}

TEST_F(file_operation_fixture, get_file_info_for_range_large_file_large_range)
{
    test_get_file_info(UINT64_MAX, 128, UINT32_MAX);
}

TEST_F(file_operation_fixture, get_file_info_for_range_large_file_large_range_offset)
{
    test_get_file_info(UINT64_MAX, UINT32_MAX, UINT32_MAX + 128);
}

TEST_F(file_operation_fixture, get_file_info_for_invalid_range_start_too_large_1)
{
    test_get_file_info(1000, 1001, 2000);
}

TEST_F(file_operation_fixture, get_file_info_for_invalid_range_start_too_large_2)
{
    test_get_file_info(1000, 1001, 1001);
}

TEST_F(file_operation_fixture, get_file_info_for_invalid_range_end_smaller_than_start)
{
    test_get_file_info(1000, 1000, 999);
}

TEST_F(file_operation_fixture, get_file_info_error_unknown_file_id)
{
    test_get_file_info(wago::wdx::status_codes::unknown_file_id, http_status_code::not_found);
}

TEST_F(file_operation_fixture, get_file_info_error_file_not_accessible)
{
    test_get_file_info(wago::wdx::status_codes::file_not_accessible, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, get_file_info_error_fallback)
{
    test_get_file_info(wago::wdx::status_codes::unknown_parameter_id, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_1)
{
    test_get_file_info(invalid_ranges[0]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_2)
{
    test_get_file_info(invalid_ranges[1]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_3)
{
    test_get_file_info(invalid_ranges[2]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_4)
{
    test_get_file_info(invalid_ranges[3]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_5)
{
    test_get_file_info(invalid_ranges[4]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_6)
{
    test_get_file_info(invalid_ranges[5]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_7)
{
    test_get_file_info(invalid_ranges[6]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_8)
{
    test_get_file_info(invalid_ranges[7]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_9)
{
    test_get_file_info(invalid_ranges[8]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_10)
{
    test_get_file_info(invalid_ranges[9]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_11)
{
    test_get_file_info(invalid_ranges[10]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_12)
{
    test_get_file_info(invalid_ranges[11]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_13)
{
    test_get_file_info(invalid_ranges[12]);
}

// UINT64_MAX: 18446744073709551615
TEST_F(file_operation_fixture, get_file_malformed_range_header_14)
{
    test_get_file_info(invalid_ranges[13]);
}

TEST_F(file_operation_fixture, get_file_malformed_range_header_15)
{
    test_get_file_info(invalid_ranges[14]);
}

TEST_F(file_operation_fixture, read_file)
{
    test_get_file_content(1024);
}

TEST_F(file_operation_fixture, read_file_chunksize)
{
    test_get_file_content(download_chunk_size);
}

TEST_F(file_operation_fixture, read_file_chunksize_plus_one)
{
    test_get_file_content(download_chunk_size + 1);
}

TEST_F(file_operation_fixture, read_file_chunksize_times_ten)
{
    test_get_file_content(download_chunk_size * 10);
}

TEST_F(file_operation_fixture, read_file_max)
{
    test_get_file_content(max_download_data_length);
}

TEST_F(file_operation_fixture, read_file_larger_than_max)
{
    test_get_file_content(max_download_data_length + 1ULL);
}

// Disabled because of long test run time
TEST_F(file_operation_fixture, read_file_large_file)
{
    test_get_file_content(UINT32_MAX/128);
}

// Disabled because of long test run time
TEST_F(file_operation_fixture, read_file_even_larger_file)
{
    test_get_file_content(UINT32_MAX + 1ULL);
}

TEST_F(file_operation_fixture, read_file_range)
{
    test_get_file_content(1024, 100, 500);
}

TEST_F(file_operation_fixture, read_file_range_oversized)
{
    test_get_file_content(1024, 100, 2048);
}

TEST_F(file_operation_fixture, read_file_range_after_one_chunksize)
{
    test_get_file_content(download_chunk_size + 100, download_chunk_size + 1, download_chunk_size + 5);
}

TEST_F(file_operation_fixture, read_file_range_more_than_one_chunksize)
{
    test_get_file_content(download_chunk_size * 3, 100, download_chunk_size * 2);
}

TEST_F(file_operation_fixture, read_file_range_quite_large_offset)
{
    test_get_file_content(UINT64_MAX, UINT32_MAX - 1024 * 1024 * 5, UINT32_MAX + 1ULL);
}

TEST_F(file_operation_fixture, read_file_range_very_large_offset)
{
    test_get_file_content(UINT64_MAX, UINT64_MAX - 1024 * 1024 * 5, UINT64_MAX - 1);
}

TEST_F(file_operation_fixture, read_file_range_max)
{
    test_get_file_content(UINT64_MAX, UINT32_MAX + 1ULL, UINT32_MAX + max_download_data_length);
}

TEST_F(file_operation_fixture, read_file_range_larger_than_max)
{
    test_get_file_content(UINT64_MAX, UINT32_MAX + 1ULL, UINT32_MAX + max_download_data_length + 1ULL);
}

TEST_F(file_operation_fixture, read_file_range_invalid_range_start_too_large_1)
{
    test_get_file_content(1000, 1001, 2000);
}

TEST_F(file_operation_fixture, read_file_range_invalid_range_start_too_large_2)
{
    test_get_file_content(1000, 1001, 1001);
}

TEST_F(file_operation_fixture, read_file_range_invalid_range_end_smaller_than_start)
{
    test_get_file_content(1000, 1000, 999);
}

TEST_F(file_operation_fixture, read_file_range_invalid_range_end_too_large)
{
    test_get_file_content(UINT64_MAX, 0, UINT64_MAX);
}

TEST_F(file_operation_fixture, read_file_get_info_error_unknown_file_id)
{
    test_get_file_content(wago::wdx::status_codes::unknown_file_id, http_status_code::not_found);
}

TEST_F(file_operation_fixture, read_file_get_file_info_error_file_not_accessible)
{
    test_get_file_content(wago::wdx::status_codes::file_not_accessible, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, read_file_get_file_info_error_fallback)
{
    test_get_file_content(wago::wdx::status_codes::unknown_parameter_id, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_1)
{
    test_get_file_content(invalid_ranges[0]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_2)
{
    test_get_file_content(invalid_ranges[1]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_3)
{
    test_get_file_content(invalid_ranges[2]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_4)
{
    test_get_file_content(invalid_ranges[3]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_5)
{
    test_get_file_content(invalid_ranges[4]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_6)
{
    test_get_file_content(invalid_ranges[5]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_7)
{
    test_get_file_content(invalid_ranges[6]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_8)
{
    test_get_file_content(invalid_ranges[7]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_9)
{
    test_get_file_content(invalid_ranges[8]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_10)
{
    test_get_file_content(invalid_ranges[9]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_11)
{
    test_get_file_content(invalid_ranges[10]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_12)
{
    test_get_file_content(invalid_ranges[11]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_13)
{
    test_get_file_content(invalid_ranges[12]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_14)
{
    test_get_file_content(invalid_ranges[13]);
}

TEST_F(file_operation_fixture, read_file_malformed_range_header_15)
{
    test_get_file_content(invalid_ranges[14]);
}

TEST_F(file_operation_fixture, write_file)
{
    test_set_file_content(1024);
}

TEST_F(file_operation_fixture, write_file_multiple_chunks)
{
    test_set_file_content(upload_chunk_size * 5);
}

TEST_F(file_operation_fixture, write_file_max)
{
    test_set_file_content(max_upload_request_length);
}

TEST_F(file_operation_fixture, write_file_larger_than_max)
{
    test_set_file_content(max_upload_request_length + 1ULL);
}

TEST_F(file_operation_fixture, write_file_quite_large)
{
    test_set_file_content(UINT32_MAX + 1ULL);
}

TEST_F(file_operation_fixture, write_file_very_large)
{
    test_set_file_content(UINT64_MAX); 
}

TEST_F(file_operation_fixture, write_file_too_small_content_in_first_chunk)
{
    test_set_file_content(1024 * 3, 1024);
}

TEST_F(file_operation_fixture, write_file_too_small_content_in_second_chunk)
{
    test_set_file_content(upload_chunk_size + 1024, upload_chunk_size + 1);
}

TEST_F(file_operation_fixture, write_file_too_large_content_in_first_chunk)
{
    // TODO: should this be an error case? currently there will be a 204
    test_set_file_content(1024, 1024 * 3);
}

TEST_F(file_operation_fixture, write_file_too_large_content_in_second_chunk)
{
    // TODO: should this be an error case? currently there will be a 204
    test_set_file_content(upload_chunk_size + 1, upload_chunk_size + 1024);
}

TEST_F(file_operation_fixture, write_file_existing_file_smaller_than_new_file)
{
    test_set_file_content(1024, 1024, 128);
}

TEST_F(file_operation_fixture, write_file_existing_file_larger_than_new_file)
{
    test_set_file_content(1024, 1024, 2048);
}

TEST_F(file_operation_fixture, write_file_wrong_content_type_header_1)
{
    test_set_file_content(1024, "wrong/content-type");
}

TEST_F(file_operation_fixture, write_file_wrong_content_type_header_2)
{
    test_set_file_content(1024, std::string(multipart_content_type) + "; boundary=abcdef");
}

TEST_F(file_operation_fixture, write_file_wrong_content_type_header_3)
{
    test_set_file_content(1024, "application/json");
}

TEST_F(file_operation_fixture, write_file_range_nl_end)
{
    test_set_file_content(1024, { {0, 511} }, "abcdef", "\n");
}

TEST_F(file_operation_fixture, write_file_range_cr_end)
{
    test_set_file_content(1024, { {0, 511} }, "abcdef", "\r");
}

TEST_F(file_operation_fixture, write_file_range_cr_nl_end)
{
    test_set_file_content(1024, { {0, 511} }, "abcdef", "\r\n");
}

TEST_F(file_operation_fixture, write_file_range_nl_cr_end)
{
    test_set_file_content(1024, { {0, 511} }, "abcdef", "\n\r");
}

TEST_F(file_operation_fixture, write_file_range)
{
    test_set_file_content(1024, { {0, 511} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_2)
{
    test_set_file_content(1024, { {100, 511} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_3)
{
    test_set_file_content(1024, { {UINT32_MAX - 1, UINT32_MAX + 1022ULL} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_4)
{
    test_set_file_content(1024, { {UINT64_MAX - 1024, UINT64_MAX - 1} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_full_range)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_existing_file_smaller_than_range)
{
    test_set_file_content(1024,  { {100, 1023} }, "abcdef", http_endl, 128);
}

TEST_F(file_operation_fixture, write_file_range_existing_file_larger_than_range)
{
    test_set_file_content(1024,  { {100, 1023} }, "abcdef", http_endl, 2048);
}

TEST_F(file_operation_fixture, write_file_range_large)
{
    test_set_file_content(max_upload_request_length + 128ULL, { {16, 16ULL + max_upload_request_length - 256} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_max)
{
    test_set_file_content(max_upload_request_length + 128ULL, { {16, 16ULL + max_upload_request_length - 1} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_larger_than_max)
{
    test_set_file_content(max_upload_request_length + 128ULL, { {16, 16ULL + max_upload_request_length} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_multiple_larger_than_max)
{
    test_set_file_content(max_upload_request_length + 128ULL, { {0, max_upload_request_length - 1}, { 0, 16 } }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_multiple_ranges)
{
    test_set_file_content(1024, { {0, 511}, {512, 1023} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_overlapping_ranges)
{
    test_set_file_content(1024, { {0, 1023}, {512, 1023} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_type_header_1)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef", http_endl, 0, "wrong/content-type");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_type_header_2)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef", http_endl, 0, binary_content_type);
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_type_header_3)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef", http_endl, 0, "application/json");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_type_header_inside_multiart_1)
{
    test_set_file_content(1024, { {0, 1023, "wrong/content-type"} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_type_header_inside_multiart_2)
{
    test_set_file_content(1024, { {0, 1023, multipart_content_type} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_type_header_inside_multiart_3)
{
    test_set_file_content(1024, { {0, 1023, "application/json"} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart)
{
    test_set_file_content(1024, { {0, 1023, "", "notarange"} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_1)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[0]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_2)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[1]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_3)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[2]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_4)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[3]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_5)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[4]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_6)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[5]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_7)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[6]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_8)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[7]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_wrong_content_range_header_inside_multiart_9)
{
    test_set_file_content(1024, { {0, 1023, "", invalid_content_ranges[8]} }, "abcdef");
}

TEST_F(file_operation_fixture, write_file_range_provider_internal_error)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef", http_endl, 0, "", wago::wdx::status_codes::internal_error, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, write_file_range_provider_file_size_exceeded)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef", http_endl, 1024, "", wago::wdx::status_codes::file_size_exceeded, http_status_code::bad_request);
}

TEST_F(file_operation_fixture, write_file_range_provider_file_not_accessible)
{
    test_set_file_content(1024, { {0, 1023} }, "abcdef", http_endl, 1024, "", wago::wdx::status_codes::file_not_accessible, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, write_file_get_info_error_unknown_file_id)
{
    test_set_file_content(wago::wdx::status_codes::unknown_file_id, http_status_code::not_found);
}

TEST_F(file_operation_fixture, write_file_get_file_info_error_file_not_accessible)
{
    test_set_file_content(wago::wdx::status_codes::file_not_accessible, http_status_code::internal_server_error);
}

TEST_F(file_operation_fixture, write_file_get_file_info_error_fallback)
{
    test_set_file_content(wago::wdx::status_codes::unknown_parameter_id, http_status_code::internal_server_error);
}
/*
- get_file_content
    -> PUT Ungültiger Accept header im request -> 406
*/


//---- End of source file ------------------------------------------------------
