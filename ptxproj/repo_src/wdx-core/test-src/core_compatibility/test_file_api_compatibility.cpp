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
///  \brief    Test WAGO Parameter Service Core file api compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "mocks/mock_file_api.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;
using wago::resolved_future;
using testing::Exactly;
using testing::Return;
using testing::ByMove;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class file_api_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_file_api                  file_api_mock;

    // test specifica
    parameter_service_file_api_i  *test_file_api = &file_api_mock;

protected:
    file_api_fixture() = default;
    ~file_api_fixture() override = default;

    void SetUp() override
    {
        // Set default call expectations
        file_api_mock.set_default_expectations();
    }
};

TEST_F(file_api_fixture, source_compatibility_construct_delete)
{
    // Noting to do, everything is done in SetUp/TearDown
}

TEST_F(file_api_fixture, source_compatibility_file_read)
{
    file_id  test_id     = "testfile";
    uint64_t test_offset = 128;
    size_t   test_length = 2048;

    file_read_response expected_response = file_read_response({ 0x15, 0x42 });
    file_read_response core_response = expected_response;
    EXPECT_CALL(file_api_mock, file_read(test_id, test_offset, test_length))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    file_read_response actual_response = test_file_api->file_read(test_id, test_offset, test_length).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
    EXPECT_EQ(expected_response.data.size(), actual_response.data.size());
    for(size_t i = 0; i < actual_response.data.size(); ++i)
    {
        EXPECT_EQ(expected_response.data.at(i), actual_response.data.at(i));
    }
}

TEST_F(file_api_fixture, source_compatibility_file_write)
{
    file_id  test_id                 = "testfile";
    uint64_t             test_offset = 13;
    std::vector<uint8_t> test_data   = { 0x18, 0x11 };

    response expected_response = response(wago::wdx::status_codes::success);
    response core_response = expected_response;
    EXPECT_CALL(file_api_mock, file_write(test_id, test_offset, test_data))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    response actual_response = test_file_api->file_write(test_id, test_offset, test_data).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
}

TEST_F(file_api_fixture, source_compatibility_file_get_info)
{
    file_id  test_id = "testfileXY";

    file_info_response expected_response = file_info_response(128593);
    file_info_response core_response = expected_response;
    EXPECT_CALL(file_api_mock, file_get_info(test_id))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    file_info_response actual_response = test_file_api->file_get_info(test_id).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
    EXPECT_EQ(expected_response.file_size, actual_response.file_size);
}

TEST_F(file_api_fixture, source_compatibility_file_create)
{
    file_id  test_id       = "testfileXYZ";
    uint64_t test_capacity = 326546487674;

    response expected_response = response(wago::wdx::status_codes::success);
    response core_response = expected_response;
    EXPECT_CALL(file_api_mock, file_create(test_id, test_capacity))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    response actual_response = test_file_api->file_create(test_id, test_capacity).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
}

TEST_F(file_api_fixture, source_compatibility_create_parameter_upload_id)
{
    wago::wdx::parameter_instance_path test_context(  parameter_instance_path_t("some/parameter/path"));
    uint16_t                           test_timeout = 64;

    file_id_response expected_response = file_id_response("newfileid");
    file_id_response core_response = expected_response;
    EXPECT_CALL(file_api_mock, create_parameter_upload_id(test_context, test_timeout))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    file_id_response actual_response = test_file_api->create_parameter_upload_id(test_context, test_timeout).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
    EXPECT_EQ(expected_response.registered_file_id, actual_response.registered_file_id);
}

TEST_F(file_api_fixture, source_compatibility_create_parameter_upload_id_default_timeout)
{
    wago::wdx::parameter_instance_path test_context(     parameter_instance_path_t("some/other/parameter/path"));
    uint16_t                           default_timeout = 3600;

    file_id_response expected_response = file_id_response("newfileid2");
    file_id_response core_response = expected_response;
    EXPECT_CALL(file_api_mock, create_parameter_upload_id(test_context, default_timeout))
       .Times(Exactly(1))
       .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));

    file_id_response actual_response = test_file_api->create_parameter_upload_id(test_context).get();
    EXPECT_EQ(expected_response.status, actual_response.status);
    EXPECT_EQ(expected_response.registered_file_id, actual_response.registered_file_id);
}


//---- End of source file ------------------------------------------------------
