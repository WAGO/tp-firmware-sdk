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
///  \brief    Mock file provider.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_MOCK_FILE_PROVIDER_HPP_
#define INC_WAGO_WDX_TEST_MOCK_FILE_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/file_transfer/file_provider_i.hpp>

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
/// This is a mock implementation for the file provider.
/// \copydoc wago::wdx::file_provider_i
class mock_file_provider : public wdx::file_provider_i
{
public:
    ~mock_file_provider() noexcept override = default;

    /// Mock implementation of wago::wdx::file_provider_i::read
    /// \copydoc wago::wdx::file_provider_i::read
    MOCK_METHOD2(read, future<wdx::file_read_response>(uint64_t offset, size_t length));

    /// Mock implementation of wago::wdx::file_provider_i::write
    /// \copydoc wago::wdx::file_provider_i::write
    MOCK_METHOD2(write, future<wdx::response>(uint64_t offset, std::vector<uint8_t> data));

    /// Mock implementation of wago::wdx::file_provider_i::get_file_info
    /// \copydoc wago::wdx::file_provider_i::get_file_info
    MOCK_METHOD0(get_file_info, future<wdx::file_info_response>());

    /// Mock implementation of wago::wdx::file_provider_i::create
    /// \copydoc wago::wdx::file_provider_i::create
    MOCK_METHOD1(create, future<wdx::response>(uint64_t capacity));

    /// Expect mocked methods not to be called.
    void set_default_expectations()
    {
        EXPECT_CALL(*this, read(testing::_, testing::_))
            .Times(0);
        EXPECT_CALL(*this, write(testing::_, testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_file_info())
            .Times(0);
        EXPECT_CALL(*this, create(testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_MOCK_FILE_PROVIDER_HPP_
//---- End of source file ------------------------------------------------------
