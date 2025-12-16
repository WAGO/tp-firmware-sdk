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
///  \brief    Test file api proxy and stub interaction.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_api/file_api_proxy.hpp"
#include "file_api/file_api_stub.hpp"
#include "base_proxy_and_stub_with_ownership_fixture.hpp"
#include <wago/wdx/test/mock_file_api.hpp>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::file_api_proxy;
using wago::wdx::linuxos::com::file_api_stub;
using wago::wdx::test::mock_file_api;

using namespace wago::wdx;
using wago::resolved_future;

using testing::Return;
using testing::ByMove;

using file_api_proxy_and_stub_fixture = base_proxy_and_stub_with_ownership_fixture<mock_file_api,
                                                                                   file_api_proxy,
                                                                                   file_api_stub>;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

TEST_F(file_api_proxy_and_stub_fixture, construct_delete)
{
    // nothing to do
}


TEST_F(file_api_proxy_and_stub_fixture, file_read)
{
    file_id  test_file_id = "abc";
    uint64_t test_offset  = 1;
    size_t   test_length  = 2;
    bytes_t  test_data    = { 4, 2 };

    file_read_response expected_response;
    expected_response.status  = wago::wdx::status_codes::success;
    expected_response.data    = test_data;

    // original interface must be called
    auto file_api_response = expected_response;
    EXPECT_CALL(interface_mock, file_read(test_file_id, test_offset, test_length))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(file_api_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.file_read(test_file_id, test_offset, test_length).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.status, res.status);
        ASSERT_EQ(expected_response.data, res.data);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_api_proxy_and_stub_fixture, file_write)
{
    file_id  test_file_id = "abc";
    uint64_t test_offset  = 1;
    bytes_t  test_data    = { 4, 2 };

    response expected_response;
    expected_response.status  = wago::wdx::status_codes::success;

    // original interface must be called
    auto file_api_response = expected_response;
    EXPECT_CALL(interface_mock, file_write(test_file_id, test_offset, test_data))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(file_api_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.file_write(test_file_id, test_offset, test_data).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_api_proxy_and_stub_fixture, file_get_info)
{
    file_id  test_file_id = "abc";
    uint64_t test_size    = 1337;

    file_info_response expected_response;
    expected_response.status    = wago::wdx::status_codes::success;
    expected_response.file_size = test_size;

    // original interface must be called
    auto file_api_response = expected_response;
    EXPECT_CALL(interface_mock, file_get_info(test_file_id))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(file_api_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.file_get_info(test_file_id).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.status, res.status);
        ASSERT_EQ(expected_response.file_size, res.file_size);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_api_proxy_and_stub_fixture, file_create)
{
    file_id  test_file_id  = "abc";
    uint64_t test_capacity = 1337;

    response expected_response;
    expected_response.status = wago::wdx::status_codes::success;

    // original interface must be called
    auto file_api_response = expected_response;
    EXPECT_CALL(interface_mock, file_create(test_file_id, test_capacity))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(file_api_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.file_create(test_file_id, test_capacity).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_api_proxy_and_stub_fixture, create_parameter_upload_id)
{
    file_id                 test_file_id = "abc";
    parameter_instance_path test_file_context("a/parameter/path");
    uint16_t                test_timeout = 42;

    wago::wdx::file_id_response expected_response(test_file_id);

    // original interface must be called
    auto file_api_response = expected_response;
    EXPECT_CALL(interface_mock, create_parameter_upload_id(test_file_context, test_timeout))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(file_api_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.create_parameter_upload_id(test_file_context, test_timeout).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

//---- End of source file ------------------------------------------------------
