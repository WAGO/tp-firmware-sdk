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
///  \brief    Mock for registered_device_store_i interface (libwdxlinuxoscom).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_REGISTERED_DEVICE_STORE_HPP_
#define TEST_INC_MOCKS_MOCK_REGISTERED_DEVICE_STORE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/registered_device_store_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::registered_device_store_i;
using testing::Invoke;
using testing::Return;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_registered_device_store : public registered_device_store_i
{
public:
    MOCK_METHOD0(has_devices, bool());
    MOCK_METHOD1(has_device, bool(wago::wdx::device_id id));
    MOCK_METHOD1(get_device_meta, std::shared_ptr<device_meta>(wago::wdx::device_id id));
    MOCK_METHOD1(add_device, void(wago::wdx::register_device_request const &request));
    MOCK_METHOD1(remove_device, void(wago::wdx::device_id id));
    MOCK_METHOD1(remove_devices, void(std::function<bool(device_meta const &meta)> predicate));

    MOCK_CONST_METHOD0(get_connection_name, std::string const &());

    void set_default_expectations()
    {
        EXPECT_CALL(*this, has_devices())
            .Times(0);
        EXPECT_CALL(*this, has_device(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_device_meta(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, add_device(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remove_device(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remove_devices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_connection_name())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_REGISTERED_DEVICE_STORE_HPP_
//---- End of source file ------------------------------------------------------
