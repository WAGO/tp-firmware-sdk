//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Mock File API.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_MOCK_FILE_API_HPP_
#define INC_WAGO_WDX_TEST_MOCK_FILE_API_HPP_

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
namespace wago {
namespace wdx {
namespace test {

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
/// This is a mock implementation for the parameter service frontend.
/// \copydoc wago::wdx::parameter_service_file_api_i
class mock_file_api : public wdx::parameter_service_file_api_i
{
public:
    ~mock_file_api() noexcept override = default;

    /// Mock implementation of wago::wdx::parameter_service_file_api_i::file_read
    /// \copydoc wago::wdx::parameter_service_file_api_i::file_read
    MOCK_METHOD3(file_read, wago::future<wdx::file_read_response>(wdx::file_id id, uint64_t offset, size_t length));

    /// Mock implementation of wago::wdx::parameter_service_file_api_i::file_write
    /// \copydoc wago::wdx::parameter_service_file_api_i::file_write
    MOCK_METHOD3(file_write, wago::future<wdx::response>(wdx::file_id id, uint64_t offset, wdx::bytes_t data));

    /// Mock implementation of wago::wdx::parameter_service_file_api_i::file_get_info
    /// \copydoc wago::wdx::parameter_service_file_api_i::file_get_info
    MOCK_METHOD1(file_get_info, wago::future<wdx::file_info_response>(wdx::file_id id));

    /// Mock implementation of wago::wdx::parameter_service_file_api_i::file_create
    /// \copydoc wago::wdx::parameter_service_file_api_i::file_create
    MOCK_METHOD2(file_create, wago::future<wdx::response>(wdx::file_id id, uint64_t capacity));

    /// Mock implementation of wago::wdx::parameter_service_file_api_i::create_parameter_upload_id
    /// \copydoc wago::wdx::parameter_service_file_api_i::create_parameter_upload_id
    MOCK_METHOD2(create_parameter_upload_id, wago::future<wdx::register_file_provider_response>(wdx::parameter_instance_path context, uint16_t timeout_seconds));

    /// Expect mocked methods not to be called.
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

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_MOCK_FILE_API_HPP_
//---- End of source file ------------------------------------------------------
