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
///  \brief    Test differenct provider proxy and stub interaction.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/parameter_provider_proxy.hpp"
#include "backend/parameter_provider_stub.hpp"
#include "base_proxy_and_stub_fixture.hpp"
#include <wago/wdx/test/fail.hpp>
#include <wago/wdx/test/mock_parameter_provider.hpp>

#include <gtest/gtest.h>

#include <exception>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::test;
using wago::wdx::linuxos::com::parameter_provider_proxy;
using wago::wdx::linuxos::com::parameter_provider_stub;

using wago::resolved_future;

using testing::Return;
using testing::ByMove;

using parameter_provider_proxy_and_stub_fixture = base_proxy_and_stub_fixture<mock_parameter_provider,
                                                                              parameter_provider_proxy,
                                                                              parameter_provider_stub>;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
ACTION_P(ThrowStdException, error_message)
{
    throw std::runtime_error(error_message);
}

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

TEST_F(parameter_provider_proxy_and_stub_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(parameter_provider_proxy_and_stub_fixture, get_display_name)
{
    std::string expected_response = "My display name!";

    // original provider must NOT be called
    proxy.set_name(expected_response);

    auto response = proxy.display_name();
    EXPECT_EQ(expected_response, response);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, get_provided_parameters)
{
    auto expected_response = wago::wdx::parameter_selector_response({});

    // original provider must NOT be called
    proxy.set_provided_parameters(expected_response);

    auto response = proxy.get_provided_parameters();
    EXPECT_EQ(expected_response.status, response.status);
    EXPECT_EQ(expected_response.selected_parameters.size(), response.selected_parameters.size());
}

TEST_F(parameter_provider_proxy_and_stub_fixture, get_parameter_values)
{
    auto test_instance     = wago::wdx::parameter_instance_id();
    auto expected_response = wago::wdx::value_response(
        wago::wdx::parameter_value::create_int16(42)
    );
    auto test_requests = std::vector<wago::wdx::parameter_instance_id>({
        test_instance
    });

    // original provider must be called
    std::vector<wago::wdx::value_response> provider_response = { expected_response };
    EXPECT_CALL(interface_mock, get_parameter_values(::testing::ContainerEq(test_requests))) // TODO: verify instance id to equal test_id
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.get_parameter_values(test_requests).set_notifier([expected_response, &value_notfier_called](auto &&responses) {
        value_notfier_called = true;
        EXPECT_EQ(1, responses.size());
        EXPECT_EQ(expected_response.status, responses.at(0).status);
        EXPECT_EQ(*expected_response.value, *responses.at(0).value);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, set_parameter_values)
{
    auto test_instance = wago::wdx::parameter_instance_id();
    auto test_value    = wago::wdx::parameter_value::create_int16(42);
    auto test_requests = std::vector<wago::wdx::value_request>({
        {test_instance, test_value}
    });
    auto expected_response = wago::wdx::set_parameter_response(wago::wdx::status_codes::success);

    // original provider must be called
    std::vector<wago::wdx::set_parameter_response> provider_response = { expected_response };
    EXPECT_CALL(interface_mock, set_parameter_values(::testing::_)) // TODO: verify requests match original requests. ATM operator==() is not supported on value_request
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.set_parameter_values(test_requests).set_notifier([expected_response, &value_notfier_called](auto &&responses) {
        value_notfier_called = true;
        EXPECT_EQ(1, responses.size());
        EXPECT_EQ(expected_response.status, responses.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, set_parameter_values_connection_aware)
{
    auto test_instance = wago::wdx::parameter_instance_id();
    auto test_value    = wago::wdx::parameter_value::create_int16(42);
    auto test_requests = std::vector<wago::wdx::value_request>({
        {test_instance, test_value}
    });
    auto expected_response = wago::wdx::set_parameter_response(wago::wdx::status_codes::wda_connection_changes_deferred);

    // original provider must be called
    std::vector<wago::wdx::set_parameter_response> provider_response = { expected_response };
    EXPECT_CALL(interface_mock, set_parameter_values_connection_aware(::testing::_, true)) // TODO: verify requests match original requests. ATM operator==() is not supported on value_request
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(true);
    bool value_notfier_called = false;
    proxy.set_parameter_values_connection_aware(test_requests, true).set_notifier([expected_response, &value_notfier_called](auto &&responses) {
        value_notfier_called = true;
        EXPECT_EQ(1, responses.size());
        EXPECT_EQ(expected_response.status, responses.at(0).status);
    });
    // resolve future by running the driver!
    roundtrip();
    EXPECT_TRUE(value_notfier_called);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, invoke_method)
{
    auto test_instance = wago::wdx::parameter_instance_id();
    auto test_value    = wago::wdx::parameter_value::create_int16(42);
    auto test_args     = std::vector<std::shared_ptr<wago::wdx::parameter_value>>({
        test_value
    });
    auto expected_response = wago::wdx::method_invocation_response({
        test_value
    });

    // original provider must be called
    wago::wdx::method_invocation_response provider_response = expected_response;
    EXPECT_CALL(interface_mock, invoke_method(::testing::Eq(test_instance), ::testing::_)) // TODO: verify args to equal test_args
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(1);

    bool value_notfier_called = false;
    proxy.invoke_method(test_instance, test_args).set_notifier([expected_response, &value_notfier_called](auto &&response) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, response.status);
        EXPECT_EQ(expected_response.domain_specific_status_code, response.domain_specific_status_code);
        ASSERT_EQ(expected_response.out_args.size(), response.out_args.size());
        EXPECT_EQ(*expected_response.out_args.at(0), *response.out_args.at(0));
    });
    // resolve future by running the driver!
    roundtrip();

    EXPECT_TRUE(value_notfier_called);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, create_parameter_upload_id)
{
    auto test_context      = wago::wdx::parameter_id_t(61);
    auto test_value        = wago::wdx::parameter_value::create_file_id("somefile");
    auto expected_response = wago::wdx::file_id_response(test_value->get_file_id());

    // original provider must be called
    wago::wdx::file_id_response provider_response = expected_response;
    EXPECT_CALL(interface_mock, create_parameter_upload_id(::testing::Eq(test_context)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(1);

    bool value_notfier_called = false;
    proxy.create_parameter_upload_id(test_context).set_notifier([expected_response, &value_notfier_called](auto &&response) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, response.status);
        ASSERT_EQ(expected_response.registered_file_id, response.registered_file_id);
    });
    // resolve future by running the driver!
    roundtrip();

    EXPECT_TRUE(value_notfier_called);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, remove_parameter_upload_id)
{
    auto test_context      = wago::wdx::parameter_id_t(6587);
    auto test_value        = wago::wdx::parameter_value::create_file_id("someotherfile");
    auto expected_response = wago::wdx::response(wago::wdx::status_codes::success);

    // original provider must be called
    wago::wdx::response provider_response = expected_response;
    EXPECT_CALL(interface_mock, remove_parameter_upload_id(::testing::Eq(test_value->get_file_id()), ::testing::Eq(test_context)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(provider_response)))));

    auto roundtrip = helper.expect_roundtrip(1);

    bool value_notfier_called = false;
    proxy.remove_parameter_upload_id(test_value->get_file_id(), test_context).set_notifier([expected_response, &value_notfier_called](auto &&response) {
        value_notfier_called = true;
        EXPECT_EQ(expected_response.status, response.status);
    });
    // resolve future by running the driver!
    roundtrip();

    EXPECT_TRUE(value_notfier_called);
}

TEST_F(parameter_provider_proxy_and_stub_fixture, get_parameter_values_exception)
{
    auto test_instance               = wago::wdx::parameter_instance_id();
    char const expected_error_text[] = "Test exception message";
    auto test_requests = std::vector<wago::wdx::parameter_instance_id>({
        test_instance
    });

    // original provider must be called
    EXPECT_CALL(interface_mock, get_parameter_values(::testing::ContainerEq(test_requests))) // TODO: verify instance id to equal test_id
        .Times(1)
        .WillRepeatedly(ThrowStdException(expected_error_text));

    auto roundtrip = helper.expect_roundtrip(1);

    bool got_exception = false;
    add_expected_fail();
    proxy.get_parameter_values(test_requests).set_exception_notifier([&expected_error_text, &got_exception](auto exception_ptr) {
        ASSERT_NE(nullptr, exception_ptr);
        try
        {
            std::rethrow_exception(exception_ptr);
        }
        catch(std::exception &e)
        {
            got_exception = true;
            EXPECT_TRUE(std::string(e.what()).find(expected_error_text));
        }
    });
    // resolve future by running the driver!
    roundtrip();
    check_fail_count();

    // check for exception
    EXPECT_TRUE(got_exception);
}


//---- End of source file ------------------------------------------------------
