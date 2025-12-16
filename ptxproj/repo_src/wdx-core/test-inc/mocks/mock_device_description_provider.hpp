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
///  \brief    Mock device description provider.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_DEVICE_DESCRIPTION_PROVIDER_HPP_
#define TEST_INC_MOCKS_MOCK_DEVICE_DESCRIPTION_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "device_description_provider_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
/// This is a mock implementation for the device description provider.
/// \copydoc wago::wdx::device_description_provider_i
class mock_device_description_provider : public wago::wdx::device_description_provider_i
{
public:
    ~mock_device_description_provider() noexcept override = default;

    /// Mock implementation of wago::wdx::device_description_provider_i::get_provided_devices
    /// \copydoc wago::wdx::device_description_provider_i::get_provided_devices
    MOCK_METHOD0(get_provided_devices, wago::wdx::device_selector_response());

    /// Mock implementation of wago::wdx::device_description_provider_i::get_device_information
    /// \copydoc wago::wdx::device_description_provider_i::get_device_information
    MOCK_METHOD2(get_device_information, wago::future<wago::wdx::wdd_response>(std::string order_number, std::string firmware_version));

    /// Expect mocked methods not to be called.
    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_provided_devices())
            .Times(0);
        EXPECT_CALL(*this, get_device_information(testing::_, testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP

#endif // TEST_INC_MOCKS_MOCK_DEVICE_DESCRIPTION_PROVIDER_HPP_
//---- End of source file ------------------------------------------------------
