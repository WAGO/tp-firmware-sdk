//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock service core frontend.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_FILE_API_HPP_
#define TEST_INC_MOCKS_MOCK_FILE_API_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::file_transfer::parameter_service_file_api_i;
using wago::wdx::response;
using wago::wdx::file_read_response;
using wago::wdx::file_info_response;
using wago::wdx::file_id;
using wago::wdx::register_file_provider_response;
using wago::wdx::parameter_instance_path;
using wago::future;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_file_api : public parameter_service_file_api_i
{
public:
    ~mock_file_api() override = default;
    MOCK_METHOD3(file_read, future<file_read_response>(file_id id, uint64_t offset, size_t length));
    MOCK_METHOD3(file_write, future<response>(file_id id, uint64_t offset, std::vector<uint8_t> data));
    MOCK_METHOD1(file_get_info, future<file_info_response>(file_id id));
    MOCK_METHOD2(file_create, future<response>(file_id id, uint64_t capacity));
    MOCK_METHOD2(create_parameter_upload_id, future<register_file_provider_response>(parameter_instance_path context, uint16_t timeout_seconds));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, file_read(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, file_write(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, file_get_info(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, file_create(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, create_parameter_upload_id(::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_FILE_API_HPP_
//---- End of source file ------------------------------------------------------
