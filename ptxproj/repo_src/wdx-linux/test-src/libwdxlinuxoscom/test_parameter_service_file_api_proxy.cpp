//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Test Parameter Service file api proxy component.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_file_api_proxy.hpp"

#include "mocks/mock_inotify.hpp"
#include "mocks/mock_filesystem.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::com;
using wago::wdx::parameter_service_file_api_i;
using testing::Return;
using testing::AnyNumber;
using testing::WithArgs;
using testing::Invoke;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class parameter_service_file_api_proxy_fixture : public testing::Test
{
public:
    mock_inotify          inotify_mock;
    mock_filesystem       filesystem_mock;

    std::string     const socket_parent_dir = "/path/to";
    std::string     const socket_path       = socket_parent_dir + "/socket";

    parameter_service_file_api_proxy_fixture()
    {}

    ~parameter_service_file_api_proxy_fixture() override = default;

    void SetUp() override
    {
        inotify_mock.set_default_expectations();
        filesystem_mock.set_default_expectations();
    }
};

TEST_F(parameter_service_file_api_proxy_fixture, construct_delete)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_file_api_proxy proxy("Unit test", socket_path);
}

TEST_F(parameter_service_file_api_proxy_fixture, file_api_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_file_api_proxy  proxy("Unit test", socket_path);
    parameter_service_file_api_i     &frontend = proxy.get_file_api();
    EXPECT_NE(nullptr, &frontend);
}

//---- End of source file ------------------------------------------------------
