//------------------------------------------------------------------------------
// Copyright (c) 2025 WAGO GmbH & Co. KG
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
///  \brief    Simple test for permissions component.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "auth/permissions.hpp"
#include "mocks/mock_wdx_permission.hpp"

#include <stdexcept>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::serv::auth;
using testing::StrEq;
using testing::AtLeast;
using testing::Return;
using testing::Invoke;
using testing::WithArg;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class permissions_fixture : public ::testing::Test
{
protected:
    mock_wdx_permission permission_mock;
    permissions         permission_object;

protected:
    permissions_fixture() = default;
    ~permissions_fixture() override = default;
    void SetUp() override
    {
        // Set default call expectations
        permission_mock.set_default_expectations();
    }
};


TEST_F(permissions_fixture, construct_delete)
{
    // Everything is done on fixture SetUp
}

void check_set(std::set<std::string> expected_set,
               std::set<std::string> actual_set)
{
    for(auto const &exp_permission : expected_set)
    {
        EXPECT_TRUE(actual_set.count(exp_permission) > 0)
            << "Missing permission: " << exp_permission;
    }
    for(auto const &permission : actual_set)
    {
        EXPECT_TRUE(expected_set.count(permission) > 0)
            << "Unexpected permission: " << permission;
    }
}

TEST_F(permissions_fixture, get_user_permissions)
{
    char                     const user_name[] = "SomeUser";
    std::vector<std::string> const user_groups = {
                                                   "some-group",
                                                   "wdx-Read_Feature-ro",
                                                   "other_group2",
                                                   "wdx-write_feature-rw",
                                                   "wdx-no_feature-ry",
                                                   "nonwdx-read_feature-ro",
                                                   "wdx-Also-write-Feature-rw",
                                                   "wdx-also-write-Feature-rw",
                                                   "nonwdx-write_feature-rw",
                                                   "wdx-great2feature-rw",
                                                   "wdx-other",
                                                   "wdx-alsoread4feature-ro",
                                                   "wda",
                                                   "wdx",
                                                   "wdx-",
                                                   "wdx-y",
                                                   "wdx--ro",
                                                   "wdx-rw",
                                                   "-ro",
                                                   "rw",
                                                   "r",
                                                   "-r",
                                                   ""
                                                 };

    std::set<std::string> const expected_read_permissions  = {
                                                                 "read_feature",
                                                                 "alsoread4feature",
                                                                 "write_feature",
                                                                 "also-write-feature",
                                                                 "great2feature"
                                                             };

    std::set<std::string> const expected_write_permissions = {
                                                                 "write_feature",
                                                                 "also-write-feature",
                                                                 "great2feature"
                                                             };
    EXPECT_CALL(permission_mock, get_user_groups(StrEq(user_name)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(user_groups));

    auto const   user_permissions = permission_object.get_user_permissions(user_name);
    EXPECT_TRUE( user_permissions.is_determined());
    EXPECT_TRUE( user_permissions.is_success());
    EXPECT_FALSE(user_permissions.has_error());
    check_set(expected_read_permissions,  user_permissions.read_permissions);
    check_set(expected_write_permissions, user_permissions.write_permissions);
}

TEST_F(permissions_fixture, error_handling_for_get_user_permissions)
{
    char const user_name[]           = "SomeUser";
    char const test_exception_text[] = "Some test exception message2";

    EXPECT_CALL(permission_mock, get_user_groups(StrEq(user_name)))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArg<0>(Invoke([&user_name, test_exception_text](std::string const &actual_user_name){
            if(user_name == actual_user_name)
            {
                throw std::runtime_error(test_exception_text);
            }
            return std::vector<std::string>();
        })));

    auto const   user_permissions = permission_object.get_user_permissions(user_name);
    EXPECT_TRUE( user_permissions.is_determined());
    EXPECT_FALSE(user_permissions.is_success());
    EXPECT_TRUE( user_permissions.has_error());
    EXPECT_NE(std::string::npos, user_permissions.get_message().find(test_exception_text));
}


//---- End of source file ------------------------------------------------------
