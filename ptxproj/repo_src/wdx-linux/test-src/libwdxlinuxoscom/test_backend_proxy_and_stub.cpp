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
///  \brief    Test backend proxy and stub interaction.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/backend_proxy.hpp"
#include "backend/backend_stub.hpp"
#include "backend/parameter_provider_stub.hpp"
#include "backend/file_provider_stub.hpp"

#include "mocks/mock_sender.hpp"
#include "mocks/mock_managed_object_store.hpp"
#include "mocks/mock_registered_device_store.hpp"
#include "mocks/mock_remote_backend.hpp"
#include "mocks/mock_driver.hpp"
#include <wago/wdx/test/mock_parameter_provider.hpp>
#include <wago/wdx/test/mock_file_provider.hpp>
#include <wago/wdx/test/wda_check.hpp>

#include "proxy_stub_helper.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::test;
using wago::wdx::linuxos::com::backend_proxy;
using wago::wdx::linuxos::com::backend_stub;
using wago::wdx::linuxos::com::managed_object;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::data_stream;
using wago::wdx::linuxos::com::parameter_provider_stub;
using wago::wdx::linuxos::com::file_provider_stub;

using testing::Return;
using testing::ReturnRef;
using testing::ByMove;
using testing::Invoke;
using testing::InvokeArgument;
using testing::AnyNumber;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------

class backend_proxy_and_stub_fixture : public testing::Test
{
public:
    mock_sender                  sender_mock;
    mock_driver                  driver_mock;
    mock_remote_backend          backend_mock;
    mock_managed_object_store    object_store_mock;
    mock_registered_device_store device_store_mock;

    managed_object_id    test_id;
    std::string          test_connection_name;

    backend_proxy proxy;
    backend_stub  stub;

    proxy_stub_helper helper;

    backend_proxy_and_stub_fixture()
    : test_id(42)
    , test_connection_name("Test Connection")
    , proxy(test_id, test_connection_name, sender_mock, driver_mock, object_store_mock, device_store_mock,  wc_trace_channels::all)
    , stub(test_id, test_connection_name, sender_mock, driver_mock, backend_mock, wc_trace_channels::all)
    , helper(driver_mock, sender_mock, proxy, stub)
    { }

    ~backend_proxy_and_stub_fixture() override = default;

    void SetUp() override
    {
        sender_mock.set_default_expectations();
        driver_mock.set_default_expectations();
        backend_mock.set_default_expectations();
        object_store_mock.set_default_expectations();
        device_store_mock.set_default_expectations();

        EXPECT_CALL(object_store_mock, get_connection_name())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(test_connection_name));
        EXPECT_CALL(object_store_mock, get_sender())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(sender_mock));
        EXPECT_CALL(object_store_mock, get_driver())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(driver_mock));
        EXPECT_CALL(object_store_mock, get_object_meta(testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::make_shared<mock_managed_object_store::object_meta>()));    
    }
};

TEST_F(backend_proxy_and_stub_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(backend_proxy_and_stub_fixture, register_devices)
{
    wago::wdx::register_device_request request {
        wago::wdx::device_id::headstation,
        "0123-4567",
        "01.23.45"
    };
    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must be called
    std::vector<response> core_response = { expected_response };
    EXPECT_CALL(backend_mock, remote_register_devices(::testing::_)) // FIXME: should match against request contents
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));
    EXPECT_CALL(device_store_mock, add_device(::testing::_)) // FIXME: should match against request contents
        .Times(1);

    auto roundtrip = helper.expect_roundtrip(true);

    auto future_responses = proxy.register_devices({request});
    roundtrip();

    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    EXPECT_WDA_STATUS(expected_response.status, responses.at(0).status);

}

TEST_F(backend_proxy_and_stub_fixture, unregister_devices)
{
    wago::wdx::device_id request = wago::wdx::device_id::headstation;
    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must be called
    std::vector<response> core_response = { expected_response };
    EXPECT_CALL(backend_mock, remote_unregister_devices(::testing::_)) // FIXME: should match against request contents
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));
    EXPECT_CALL(device_store_mock, remove_device(::testing::_)) // FIXME: should match against request contents
        .Times(1);

    auto roundtrip = helper.expect_roundtrip(true);

    auto future_responses = proxy.unregister_devices({request});
    roundtrip();

    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    EXPECT_WDA_STATUS(expected_response.status, responses.at(0).status);
}

TEST_F(backend_proxy_and_stub_fixture, unregister_all_devices)
{
    wago::wdx::device_collection_id_t request = wago::wdx::device_collections::rlb;
    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };
    wago::wdx::response response_to_return = expected_response;

    // original backend must be called
    EXPECT_CALL(backend_mock, remote_unregister_all_devices(::testing::_)) // FIXME: should match against request contents
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(response_to_return)))));
    EXPECT_CALL(device_store_mock, remove_devices(::testing::_)) // FIXME: should match against request contents
        .Times(1);

    auto roundtrip = helper.expect_roundtrip(true);

    auto future_response = proxy.unregister_all_devices(request);
    roundtrip();

    ASSERT_FUTURE_VALUE(future_response);
    auto response = future_response.get();
    EXPECT_WDA_STATUS(expected_response.status, response.status);
}

TEST_F(backend_proxy_and_stub_fixture, reregister_devices_without_present_device)
{
    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must NOT be called
    EXPECT_CALL(device_store_mock, has_devices())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    proxy.reregister_devices();
}

TEST_F(backend_proxy_and_stub_fixture, reregister_devices)
{
    wago::wdx::register_device_request request_1 {
        wago::wdx::device_id::headstation,
        "0123-4567",
        "01.23.45"
    };
    wago::wdx::register_device_request request_2 {
        wago::wdx::device_id(2, wago::wdx::device_collections::rlb),
        "8123-4567",
        "00.00.01"
    };
    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must be called
    std::vector<response> core_response_1 = { expected_response };
    std::vector<response> core_response_2 = { expected_response };
    std::vector<response> core_response_3 = { expected_response };
    EXPECT_CALL(backend_mock, remote_register_devices(::testing::_)) // FIXME: should match against request contents
        .Times(3)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_1)))))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_2)))))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_3)))));
    EXPECT_CALL(device_store_mock, add_device(::testing::_)) // FIXME: should match against request contents
        .Times(2);
    EXPECT_CALL(device_store_mock, has_devices())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(device_store_mock, remove_devices(::testing::_)) // FIXME: should match against request contents
        .Times(1);

    auto roundtrip_1 = helper.expect_roundtrip(true);

    auto future_responses_1 = proxy.register_devices({request_1});
    roundtrip_1();

    ASSERT_FUTURE_VALUE(future_responses_1);
    auto responses_1 = future_responses_1.get();
    EXPECT_WDA_STATUS(expected_response.status, responses_1.at(0).status);

    auto roundtrip_2 = helper.expect_roundtrip(true);

    auto future_responses_2 = proxy.register_devices({request_2});
    roundtrip_2();

    ASSERT_FUTURE_VALUE(future_responses_2);
    auto responses_2 = future_responses_2.get();
    EXPECT_WDA_STATUS(expected_response.status, responses_2.at(0).status);

    auto roundtrip_3 = helper.expect_roundtrip(true);

    proxy.reregister_devices();
    roundtrip_3();
}

TEST_F(backend_proxy_and_stub_fixture, register_parameter_providers)
{
    mock_parameter_provider test_provider;
    wago::wdx::parameter_selector_response selector_response( { wago::wdx::parameter_selector::all_of_feature("SomeFeature") } );

    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // Provider invars may be read
    EXPECT_CALL(test_provider, display_name())
        .Times(AnyNumber())
        .WillRepeatedly(Return("Test provider name"));
    EXPECT_CALL(test_provider, get_provided_parameters())
        .Times(AnyNumber())
        .WillRepeatedly(Return(selector_response));

    // original backend must be called
    managed_object_id provider_object_id = 42;
    std::vector<managed_object_id> remote_create_ids      = { provider_object_id };
    std::vector<managed_object_id> remote_create_response = remote_create_ids;
    EXPECT_CALL(backend_mock, remote_create_parameter_provider_proxies(::testing::Eq(remote_create_ids.size())))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response)))));

    EXPECT_CALL(backend_mock, remote_update_parameter_provider_proxies(::testing::ContainerEq(remote_create_ids), ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(no_return())))));

    std::vector<response> core_response = { expected_response };
    auto test_call_mode = wago::wdx::parameter_provider_call_mode::concurrent;
    EXPECT_CALL(backend_mock, remote_register_parameter_providers(::testing::ContainerEq(remote_create_ids), test_call_mode))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));

    auto added_object = object_store_mock.expect_object_to_be_added<parameter_provider_stub>(provider_object_id);
    auto roundtrip = helper.expect_roundtrip(true, 3);

    auto future_responses = proxy.register_parameter_providers({&test_provider}, test_call_mode);
    roundtrip();

    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    EXPECT_WDA_STATUS(expected_response.status, responses.at(0).status);
}

TEST_F(backend_proxy_and_stub_fixture, register_parameter_providers_with_call_mode)
{
    mock_parameter_provider test_provider;
    wago::wdx::parameter_selector_response selector_response( { wago::wdx::parameter_selector::all_of_feature("SomeFeature") } );

    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // Provider invars may be read
    EXPECT_CALL(test_provider, display_name())
        .Times(AnyNumber())
        .WillRepeatedly(Return("Test provider name"));
    EXPECT_CALL(test_provider, get_provided_parameters())
        .Times(AnyNumber())
        .WillRepeatedly(Return(selector_response));

    // original backend must be called
    managed_object_id provider_object_id = 42;
    std::vector<managed_object_id> remote_create_ids      = { provider_object_id };
    std::vector<managed_object_id> remote_create_response = remote_create_ids;
    EXPECT_CALL(backend_mock, remote_create_parameter_provider_proxies(::testing::Eq(remote_create_ids.size())))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response)))));

    EXPECT_CALL(backend_mock, remote_update_parameter_provider_proxies(::testing::ContainerEq(remote_create_ids), ::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(no_return())))));

    std::vector<response> core_response = { expected_response };
    auto test_call_mode = wago::wdx::parameter_provider_call_mode::serialized;
    EXPECT_CALL(backend_mock, remote_register_parameter_providers(::testing::ContainerEq(remote_create_ids), test_call_mode))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));

    auto added_object = object_store_mock.expect_object_to_be_added<parameter_provider_stub>(provider_object_id);
    auto roundtrip = helper.expect_roundtrip(true, 3);

    auto future_responses = proxy.register_parameter_providers({&test_provider}, test_call_mode);
    roundtrip();

    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    EXPECT_WDA_STATUS(expected_response.status, responses.at(0).status);
}

TEST_F(backend_proxy_and_stub_fixture, unregister_parameter_providers)
{
    mock_parameter_provider test_provider;

    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must be called
    managed_object_id provider_object_id = 42;
    std::vector<managed_object_id> unregister_request = { provider_object_id };
    no_return core_response;
    EXPECT_CALL(backend_mock, remote_unregister_parameter_providers(::testing::ContainerEq(unregister_request)))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));

    parameter_provider_stub provider_stub(provider_object_id, test_connection_name, sender_mock, driver_mock, test_provider);
    object_store_mock.expect_object_to_be_found(provider_stub);
    object_store_mock.expect_object_to_be_removed(provider_object_id);

    auto roundtrip = helper.expect_roundtrip(true);

    proxy.unregister_parameter_providers({&test_provider});
    roundtrip();
}

TEST_F(backend_proxy_and_stub_fixture, register_file_provider)
{
    mock_file_provider test_provider;

    wago::wdx::register_file_provider_response expected_response("test_file_id");

    // original backend must be called
    managed_object_id provider_object_id = 42;
    std::vector<managed_object_id> remote_create_ids      = { provider_object_id };
    std::vector<managed_object_id> remote_create_response = remote_create_ids;
    EXPECT_CALL(backend_mock, remote_create_file_provider_proxies(::testing::Eq(remote_create_ids.size())))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response)))));

    std::vector<register_file_provider_response> core_response = { expected_response };
    std::vector<parameter_id_t> contexts = { 1337 };
    auto test_call_mode = wago::wdx::provider_call_mode::serialized;
    EXPECT_CALL(backend_mock, remote_register_file_providers(::testing::ContainerEq(remote_create_ids), test_call_mode, ::testing::ContainerEq(contexts)))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));

    auto added_objects = object_store_mock.expect_object_to_be_added<file_provider_stub>(provider_object_id);
    // as file provider stubs are stored along with their file id, which is retrieved
    // after actual registration, there is a store lookup required.
    EXPECT_CALL(object_store_mock, get_object(testing::_))
        .Times(1)
        .WillRepeatedly(Invoke([added_objects](auto) {
            return std::ref(*added_objects->at(0));
        }));
    auto roundtrip = helper.expect_roundtrip(true, 2);

    auto future_responses = proxy.register_file_providers({{&test_provider, contexts.at(0)}}, test_call_mode);
    roundtrip();

    ASSERT_FUTURE_VALUE(future_responses);
    auto responses = future_responses.get();
    EXPECT_WDA_STATUS(expected_response.status, responses.at(0).status);
    EXPECT_EQ(expected_response.registered_file_id, responses.at(0).registered_file_id);
}

TEST_F(backend_proxy_and_stub_fixture, unregister_file_providers)
{
    mock_file_provider test_provider;

    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must be called
    managed_object_id provider_object_id = 42;
    std::vector<managed_object_id> unregister_request = { provider_object_id };
    no_return core_response;
    EXPECT_CALL(backend_mock, remote_unregister_file_providers(::testing::ContainerEq(unregister_request)))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));

    file_provider_stub provider_stub(provider_object_id, test_connection_name, sender_mock, driver_mock, test_provider);
    object_store_mock.expect_object_to_be_found(provider_stub);
    object_store_mock.expect_object_to_be_removed(provider_object_id);

    auto roundtrip = helper.expect_roundtrip(true);

    proxy.unregister_file_providers({&test_provider});
    roundtrip();
}

TEST_F(backend_proxy_and_stub_fixture, reregister_file_provider)
{
    mock_file_provider test_provider;
    wago::wdx::file_id test_file_id = "test_file_id";
    wago::wdx::register_file_provider_response expected_response(test_file_id);

    // original backend must be called
    managed_object_id provider_object_id = 42;
    std::vector<managed_object_id> remote_create_ids      = { provider_object_id };
    std::vector<managed_object_id> remote_create_response = remote_create_ids;
    EXPECT_CALL(backend_mock, remote_create_file_provider_proxies(::testing::Eq(remote_create_ids.size())))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response)))));

    std::vector<register_file_provider_response> core_response = { expected_response };
    std::vector<parameter_id_t> contexts = { 1337 };
    std::vector<wago::wdx::file_id> file_ids = { test_file_id };
    auto test_call_mode = wago::wdx::provider_call_mode::serialized;
    EXPECT_CALL(backend_mock, remote_reregister_file_providers(::testing::ContainerEq(remote_create_ids), test_call_mode, ::testing::ContainerEq(contexts), ::testing::ContainerEq(file_ids)))
        .Times(1)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response)))));

    auto added_objects = object_store_mock.expect_object_to_be_added<file_provider_stub>(provider_object_id);
    // as file provider stubs are stored along with their file id, which is retrieved
    // after actual registration, there is a store lookup required.
    EXPECT_CALL(object_store_mock, get_object(testing::_))
        .Times(1)
        .WillRepeatedly(Invoke([added_objects](auto) {
            return std::ref(*added_objects->at(0));
        }));
    auto roundtrip = helper.expect_roundtrip(true, 2);

    auto future_response = proxy.reregister_file_providers({ wago::wdx::reregister_file_provider_request(&test_provider, file_ids.at(0), contexts.at(0)) }, test_call_mode);
    roundtrip();

    ASSERT_FUTURE_VALUE(future_response);
    auto response = future_response.get();
    EXPECT_WDA_STATUS(expected_response.status, response.at(0).status);
    EXPECT_EQ(expected_response.registered_file_id, response.at(0).registered_file_id);
}

TEST_F(backend_proxy_and_stub_fixture, reregister_providers_without_present_providers)
{
    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must NOT be called
    EXPECT_CALL(object_store_mock, has_generated_objects())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    proxy.reregister_providers();
}

// FIXME: Why is this test disabled?
TEST_F(backend_proxy_and_stub_fixture, DISABLED_reregister_providers_with_parameter_provider)
{
    mock_parameter_provider test_provider_1;
    mock_parameter_provider test_provider_2;

    wago::wdx::response expected_response {
        wago::wdx::status_codes::success
    };

    // original backend must be called
    managed_object_id provider_object_id_1 = 42;
    std::vector<managed_object_id> remote_create_ids_1        = { provider_object_id_1 };
    std::vector<managed_object_id> remote_create_response_1_1 = remote_create_ids_1;
    std::vector<managed_object_id> remote_create_response_1_2 = remote_create_ids_1;
    EXPECT_CALL(backend_mock, remote_create_parameter_provider_proxies(::testing::Eq(remote_create_ids_1.size())))
        .Times(2)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response_1_1)))))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response_1_2)))));

    managed_object_id provider_object_id_2 = 43;
    std::vector<managed_object_id> remote_create_ids_2        = { provider_object_id_2 };
    std::vector<managed_object_id> remote_create_response_2_1 = remote_create_ids_2;
    std::vector<managed_object_id> remote_create_response_2_2 = remote_create_ids_2;
    EXPECT_CALL(backend_mock, remote_create_parameter_provider_proxies(::testing::Eq(remote_create_ids_2.size())))
        .Times(2)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response_2_1)))))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(remote_create_response_2_2)))));

    std::vector<response> core_response_1_1 = { expected_response };
    std::vector<response> core_response_1_2 = { expected_response };
    auto test_call_mode_1 = wago::wdx::parameter_provider_call_mode::concurrent;
    EXPECT_CALL(backend_mock, remote_register_parameter_providers(::testing::ContainerEq(remote_create_ids_1), test_call_mode_1))
        .Times(2)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_1_1)))))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_1_2)))));

    std::vector<response> core_response_2_1 = { expected_response };
    std::vector<response> core_response_2_2 = { expected_response };
    auto test_call_mode_2 = wago::wdx::parameter_provider_call_mode::serialized;
    EXPECT_CALL(backend_mock, remote_register_parameter_providers(::testing::ContainerEq(remote_create_ids_2), test_call_mode_2))
        .Times(2)
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_2_1)))))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(core_response_2_2)))));

    auto added_object_1 = object_store_mock.expect_object_to_be_added<parameter_provider_stub>(provider_object_id_1);
    managed_object const &object_1 = *(added_object_1->at(0).get());

    auto added_object_2 = object_store_mock.expect_object_to_be_added<parameter_provider_stub>(provider_object_id_1);
    managed_object const &object_2 = *(added_object_2->at(0).get());

    EXPECT_CALL(object_store_mock, has_generated_objects())
        .Times(AnyNumber())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(object_store_mock, remove_objects(::testing::_))
        .Times(1)
        .WillRepeatedly(testing::IgnoreResult(InvokeArgument<0>(std::ref(object_1))));
    EXPECT_CALL(object_store_mock, remove_objects(::testing::_))
        .Times(1)
        .WillRepeatedly(testing::IgnoreResult(InvokeArgument<0>(std::ref(object_2))));

    auto roundtrip_1 = helper.expect_roundtrip(true, 2);

    auto future_response_1 = proxy.register_parameter_provider(&test_provider_1);
    roundtrip_1();

    ASSERT_FUTURE_VALUE(future_response_1);
    auto response_1 = future_response_1.get();
    EXPECT_WDA_STATUS(expected_response.status, response_1.status);

    auto roundtrip_2 = helper.expect_roundtrip(true, 2);

    auto future_response_2 = proxy.register_parameter_provider(&test_provider_2);
    roundtrip_2();

    ASSERT_FUTURE_VALUE(future_response_2);
    auto response_2 = future_response_2.get();
    EXPECT_WDA_STATUS(expected_response.status, response_2.status);

    auto roundtrip_3 = helper.expect_roundtrip(true);

    proxy.reregister_providers();
    roundtrip_3();
}


//---- End of source file ------------------------------------------------------
