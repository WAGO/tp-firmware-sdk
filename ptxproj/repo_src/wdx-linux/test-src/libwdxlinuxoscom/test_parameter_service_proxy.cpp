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
///  \brief    Test Parameter Service proxy component.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_proxy.hpp"

#include "mocks/mock_inotify.hpp"
#include "mocks/mock_filesystem.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::com;
using wago::wdx::parameter_service_frontend_i;
using wago::wdx::parameter_service_file_api_i;
using wago::wdx::parameter_service_backend_i;
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
class parameter_service_proxy_fixture : public testing::Test
{
public:
    mock_inotify          inotify_mock;
    mock_filesystem       filesystem_mock;

    std::string     const socket_parent_dir    = "/path/to";
    std::string     const frontend_socket_path = socket_parent_dir + "/frontend_socket";
    std::string     const file_api_socket_path = socket_parent_dir + "/file_api_socket";
    std::string     const backend_socket_path  = socket_parent_dir + "/backend_socket";

    parameter_service_proxy_fixture()
    {}

    ~parameter_service_proxy_fixture() override = default;

    void SetUp() override
    {
        inotify_mock.set_default_expectations();
        filesystem_mock.set_default_expectations();
    }
};

TEST_F(parameter_service_proxy_fixture, construct_delete)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy proxy("Unit test", frontend_socket_path, backend_socket_path);
}

// TODO: remove if deprecated constructor is removed
TEST_F(parameter_service_proxy_fixture, deprecated_frontend_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy  proxy("Unit test", frontend_socket_path, backend_socket_path);
    parameter_service_frontend_i    &frontend = proxy.get_frontend();
    EXPECT_NE(nullptr, &frontend);
}

// TODO: remove if deprecated constructor is removed
TEST_F(parameter_service_proxy_fixture, deprecated_backend_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy  proxy("Unit test", frontend_socket_path, backend_socket_path);
    parameter_service_backend_i     &backend = proxy.get_backend();
    EXPECT_NE(nullptr, &backend);
}

TEST_F(parameter_service_proxy_fixture, frontend_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(file_api_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy  proxy("Unit test", frontend_socket_path, file_api_socket_path, backend_socket_path);
    parameter_service_frontend_i    &frontend = proxy.get_frontend();
    EXPECT_NE(nullptr, &frontend);
}

TEST_F(parameter_service_proxy_fixture, file_ap_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(file_api_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy       proxy("Unit test", frontend_socket_path, file_api_socket_path, backend_socket_path);
    parameter_service_file_api_i &file_api = proxy.get_file_api();
    EXPECT_NE(nullptr, &file_api);
}

TEST_F(parameter_service_proxy_fixture, backend_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(file_api_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy  proxy("Unit test", frontend_socket_path, file_api_socket_path, backend_socket_path);
    parameter_service_backend_i     &backend = proxy.get_backend();
    EXPECT_NE(nullptr, &backend);
}

TEST_F(parameter_service_proxy_fixture, notification_manager_provider)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(file_api_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    parameter_service_proxy  proxy("Unit test", frontend_socket_path, file_api_socket_path, backend_socket_path);
    backend_notification_manager_i  &manager = proxy.get_notification_manager();
    ASSERT_NE(nullptr, &manager);
}

TEST_F(parameter_service_proxy_fixture, notification_watch)
{
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(frontend_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(file_api_socket_path)))
        .Times(1)
        .WillRepeatedly(Return(true));

    EXPECT_CALL(filesystem_mock, is_socket_existing(::testing::StrEq(backend_socket_path)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));
    std::string const parent_directory = backend_socket_path.substr(0, backend_socket_path.find_last_of('/'));
    EXPECT_CALL(filesystem_mock, is_directory_existing(::testing::StrEq(parent_directory)))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(inotify_mock, add_watch(::testing::_, ::testing::StrEq(parent_directory), ::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&inotify_mock, &mock_inotify::add_watch_followers)));

    parameter_service_proxy proxy("Unit test", frontend_socket_path, file_api_socket_path, backend_socket_path);
}


//---- End of source file ------------------------------------------------------
