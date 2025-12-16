//------------------------------------------------------------------------------
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for user management interface (system abstraction).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_USER_MANAGEMENT_HPP_
#define TEST_INC_MOCKS_MOCK_USER_MANAGEMENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

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

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_user_management : public wago::authserv::sal::user_management
{
private:
    user_management *saved_instance;

public:
    passwd user_data  = {};
    group  group_data = {};

public:
    static void set_instance(user_management *new_instance)
    {
        instance = new_instance;
    }

    mock_user_management()
    {
        saved_instance = &get_instance();
        set_instance(this);
    }
    ~mock_user_management()
    {
        set_instance(saved_instance);
    }

    MOCK_METHOD(std::string, getgroupname, (gid_t gid), (const, noexcept));
    MOCK_METHOD(std::vector<std::string>, get_groups, (std::string const &user), (const));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, getgroupname(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_groups(::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_USER_MANAGEMENT_HPP_
//---- End of source file ------------------------------------------------------
