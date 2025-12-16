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
///  \brief    Test file provider proxy and stub interaction.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/file_provider_proxy.hpp"
#include "backend/file_provider_stub.hpp"
#include "base_proxy_and_stub_fixture.hpp"
#include <wago/wdx/test/mock_file_provider.hpp>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::test;
using wago::wdx::linuxos::com::file_provider_proxy;
using wago::wdx::linuxos::com::file_provider_stub;

using wago::resolved_future;

using testing::Return;
using testing::ByMove;

using file_provider_proxy_and_stub_fixture = base_proxy_and_stub_fixture<mock_file_provider,
                                                                         file_provider_proxy,
                                                                         file_provider_stub>;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------


using file_provider_proxy_and_stub_fixture = base_proxy_and_stub_fixture<mock_file_provider,
                                                                         file_provider_proxy,
                                                                         file_provider_stub>;

TEST_F(file_provider_proxy_and_stub_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(file_provider_proxy_and_stub_fixture, read)
{
    uint64_t           test_offset = 123;
    size_t             test_length = 4;
    wago::wdx::bytes_t test_data = { 0x01, 0x02, 0x03 };

    auto expected_response = wago::wdx::file_read_response(
        test_data
    );

    // original provider must be called
    auto provider_response = expected_response;
    EXPECT_CALL(interface_mock, read(::testing::Eq(test_offset), ::testing::Eq(test_length)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.read(test_offset, test_length).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
        EXPECT_EQ(expected_response.data, res.data);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_provider_proxy_and_stub_fixture, write)
{
    uint64_t           test_offset = 123;
    wago::wdx::bytes_t test_data = { 0x01, 0x02, 0x03 };

    auto expected_response = wago::wdx::response(
        wago::wdx::status_codes::success
    );

    // original provider must be called
    auto provider_response = expected_response;
    EXPECT_CALL(interface_mock, write(::testing::Eq(test_offset), ::testing::Eq(test_data)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.write(test_offset, test_data).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_provider_proxy_and_stub_fixture, get_file_info)
{
    uint64_t test_size = 4;

    auto expected_response = wago::wdx::file_info_response(
        test_size
    );

    // original provider must be called
    auto provider_response = expected_response;
    EXPECT_CALL(interface_mock, get_file_info())
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.get_file_info().set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
        EXPECT_EQ(expected_response.file_size, res.file_size);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(file_provider_proxy_and_stub_fixture, create)
{
    uint64_t test_capacity = 4;

    auto expected_response = wago::wdx::response(
        wago::wdx::status_codes::success
    );

    // original provider must be called
    auto provider_response = expected_response;
    EXPECT_CALL(interface_mock, create(::testing::Eq(test_capacity)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.create(test_capacity).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

//---- End of source file ------------------------------------------------------
