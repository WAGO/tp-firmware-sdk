//------------------------------------------------------------------------------
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock service identity.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_SERVICE_IDENTITY_HPP_
#define TEST_INC_MOCKS_MOCK_SERVICE_IDENTITY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/service_identity_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::service_identity_i;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_service_identity : public service_identity_i
{
public:
    ~mock_service_identity() override = default;

    MOCK_CONST_METHOD0(get_id, char const *());
    MOCK_CONST_METHOD0(get_name, char const *());
    MOCK_CONST_METHOD0(get_version_string, char const *());
    MOCK_CONST_METHOD0(get_major_version, uint16_t());
    MOCK_CONST_METHOD0(get_minor_version, uint16_t());
    MOCK_CONST_METHOD0(get_bugfix_version, uint16_t());
    MOCK_CONST_METHOD0(get_revision_string, char const *());

    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_id())
            .Times(0);
        EXPECT_CALL(*this, get_name())
            .Times(0);
        EXPECT_CALL(*this, get_version_string())
            .Times(0);
        EXPECT_CALL(*this, get_major_version())
            .Times(0);
        EXPECT_CALL(*this, get_minor_version())
            .Times(0);
        EXPECT_CALL(*this, get_bugfix_version())
            .Times(0);
        EXPECT_CALL(*this, get_revision_string())
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_SERVICE_IDENTITY_HPP_
//---- End of source file ------------------------------------------------------
