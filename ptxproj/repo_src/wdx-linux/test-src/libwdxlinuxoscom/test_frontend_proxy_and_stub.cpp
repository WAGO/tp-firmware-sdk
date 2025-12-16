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
///  \brief    Test frontend proxy and stub interaction.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "frontend/frontend_proxy.hpp"
#include "frontend/frontend_stub.hpp"
#include "base_proxy_and_stub_with_ownership_fixture.hpp"
#include <wago/wdx/test/mock_frontend.hpp>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::frontend_proxy;
using wago::wdx::linuxos::com::frontend_stub;
using wago::wdx::test::mock_frontend;

using namespace wago::wdx;
using wago::resolved_future;

using testing::Return;
using testing::ByMove;

using frontend_proxy_and_stub_fixture = base_proxy_and_stub_with_ownership_fixture<mock_frontend,
                                                                                   frontend_proxy,
                                                                                   frontend_stub>;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

TEST_F(frontend_proxy_and_stub_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(frontend_proxy_and_stub_fixture, get_all_devices)
{
    device_collection_response expected_response;

    device_response device_a(device_id(0, 0), "1234-5678", "00.11.22", false);
    device_response device_b(device_id(1, 2), "9101-1121", "22.11.00", false);
    expected_response.status  = wago::wdx::status_codes::success;
    expected_response.devices = { device_a, device_b };

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, get_all_devices())
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.get_all_devices().set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.status, res.status);
        ASSERT_EQ(expected_response.devices.size(), res.devices.size());
        EXPECT_EQ(expected_response.devices.at(0).status, res.devices.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, get_all_parameters)
{
    parameter_filter const filter;
    size_t           const paging_offset = 13;
    size_t           const paging_limit  = 97;

    auto const expected_response = parameter_response_list_response();

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, get_all_parameters(::testing::_, ::testing::Eq(paging_offset), ::testing::Eq(paging_limit)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.get_all_parameters(filter, paging_offset, paging_limit).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
        EXPECT_EQ(expected_response.param_responses.size(), res.param_responses.size());
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, get_parameters)
{
    std::vector<parameter_instance_id> const ids = { parameter_instance_id(23), parameter_instance_id(75) };
    std::vector<parameter_response>    const expected_response = { parameter_response(), parameter_response() };

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, get_parameters(::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.get_parameters(ids).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.size(), res.size());
        EXPECT_EQ(expected_response.at(0).status, res.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, get_parameters_by_path)
{
    std::vector<parameter_instance_path> const path_requests = { parameter_instance_path("some/path"), parameter_instance_path("some/other/path") };
    std::vector<parameter_response>      const expected_response = { parameter_response(), parameter_response() };

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, get_parameters_by_path(::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.get_parameters_by_path(path_requests).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.size(), res.size());
        EXPECT_EQ(expected_response.at(0).status, res.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, set_parameter_values)
{
    std::vector<value_request>          const requests = { value_request(parameter_instance_id(298),  parameter_value::create_int32(123)),
                                                           value_request(parameter_instance_id(3658), parameter_value::create_string("test")) };
    std::vector<set_parameter_response> const expected_response = { set_parameter_response(), set_parameter_response() };

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, set_parameter_values(::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.set_parameter_values(requests).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.size(), res.size());
        EXPECT_EQ(expected_response.at(0).status, res.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, set_parameter_values_by_path)
{
    std::vector<value_path_request>     const requests = { value_path_request(parameter_instance_path("my/param"),  parameter_value::create_int32(33)),
                                                           value_path_request(parameter_instance_path("friend/param"), parameter_value::create_string("hello")) };
    std::vector<set_parameter_response> const expected_response = { set_parameter_response(), set_parameter_response() };

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, set_parameter_values_by_path(::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.set_parameter_values_by_path(requests).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        ASSERT_EQ(expected_response.size(), res.size());
        EXPECT_EQ(expected_response.at(0).status, res.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, invoke_method)
{
    parameter_instance_id const method_id(123);
    std::map<std::string, std::shared_ptr<parameter_value>> parameter;
    parameter["My Param 1"] = parameter_value::create_float64(35.71);
    parameter["My Param 2"] = parameter_value::create_string("data, data, data!");
    std::map<std::string, std::shared_ptr<parameter_value>> const &in_args = parameter;
    method_invocation_named_response const expected_response;

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, invoke_method(method_id, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.invoke_method(method_id, in_args).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, invoke_method_by_path)
{
    parameter_instance_path const method_path("some/method");
    std::map<std::string, std::shared_ptr<parameter_value>> parameter;
    parameter["Param-1"] = parameter_value::create_bytes({ 0x13, 0x01 });
    parameter["Param-2"] = parameter_value::create_int16(INT16_MAX);
    std::map<std::string, std::shared_ptr<parameter_value>> const &in_args = parameter;
    method_invocation_named_response const expected_response;

    // original interface must be called
    auto frontend_response = expected_response;
    EXPECT_CALL(interface_mock, invoke_method_by_path(::testing::_, ::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(frontend_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.invoke_method_by_path(method_path, in_args).set_notifier([expected_response, &value_notfier_called](auto &&res) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, res.status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(frontend_proxy_and_stub_fixture, get_parameters_and_dismiss)
{
    wago::promise<std::vector<parameter_response>> test_promise;

    // original interface must be called
    EXPECT_CALL(interface_mock, get_parameters(::testing::_))
        .Times(1)
        .WillRepeatedly(Return(ByMove(test_promise.get_future())));

    bool dismiss_notfier_called = false;
    test_promise.set_dismiss_notifier([&dismiss_notfier_called]() {
        dismiss_notfier_called = true;
    });

    auto roundtrip = helper.expect_send_and_receive(true, 1);

    auto test_future = proxy.get_parameters({});
    bool value_notfier_called = false;
    test_future.set_notifier([&value_notfier_called](auto) {
        value_notfier_called = true;
    });
    // send request message
    roundtrip();

    ASSERT_FALSE(value_notfier_called);

    auto roundtrip2 = helper.expect_send_and_receive(true, 1);
    test_future.dismiss();

    // resolve future by running the driver!
    roundtrip2();

    EXPECT_FALSE(value_notfier_called);
    EXPECT_FALSE(test_promise.execute());
    EXPECT_TRUE(dismiss_notfier_called);

    // should still be possible and not cause an error
    // thus having no effect.
    test_promise.set_value({});
}


//---- End of source file ------------------------------------------------------
