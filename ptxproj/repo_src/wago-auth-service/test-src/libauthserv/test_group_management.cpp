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
///  \brief    Basic tests for group management.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "gmock/gmock-actions.h"
#include "gmock/gmock-more-actions.h"
#include "gmock/gmock-spec-builders.h"
#include "group_management.hpp"
#include "mocks/mock_user_management.hpp"
#include "fail.hpp"

#include <gtest/gtest.h>
#include <sys/types.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
using testing::Return;
using testing::WithArg;
using testing::Invoke;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(group_management, get_groups)
{
    // Test settings
    std::string test_username = "testuser";
    gid_t test_groups[] = {
        1, 2, 3, 42
    };
    int test_groups_count = 4;
    std::vector<std::string> test_group_names = { "group1", "g2", "other", "fourtytwo" };
    std::map<gid_t, group> test_group_infos = {
        { 1,  { const_cast<char*>(test_group_names[0].c_str()), const_cast<char*>(""), 1,  nullptr} },
        { 2,  { const_cast<char*>(test_group_names[1].c_str()), const_cast<char*>(""), 2,  nullptr} },
        { 3,  { const_cast<char*>(test_group_names[2].c_str()), const_cast<char*>(""), 3,  nullptr} },
        { 42, { const_cast<char*>(test_group_names[3].c_str()), const_cast<char*>(""), 42, nullptr} }
    };

    // Prepare mock
    mock_user_management user_management_mock;
    user_management_mock.set_default_expectations();

    // Set call expectations
    EXPECT_CALL(user_management_mock, get_groups(test_username.c_str()))
        .Times(1)
        .WillOnce(Return(test_group_names));

    // Test
    {
        auto groups = get_groups(test_username);
        EXPECT_EQ(test_group_names, groups);
    }
}

TEST(group_management, get_too_many_groups_results_in_empty_groups)
{
    // Test settings
    std::string test_username = "testuser";
    char const test_exception_text[] = "Some test exception message for groups";

    // Prepare mock
    mock_user_management user_management_mock;
    user_management_mock.set_default_expectations();

    EXPECT_CALL(user_management_mock, get_groups(test_username.c_str()))
        .Times(1)
        .WillRepeatedly(WithArg<0>(Invoke([&test_username, test_exception_text](std::string const &actual_user_name){
            if(test_username == actual_user_name)
            {
                throw std::runtime_error(test_exception_text);
            }
            return std::vector<std::string>();
        })));

    // Test
    {
        auto groups = get_groups(test_username);
        EXPECT_EQ(0, groups.size());
    }
}
