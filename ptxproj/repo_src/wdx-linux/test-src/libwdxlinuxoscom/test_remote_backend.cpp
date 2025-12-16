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
///  \brief    Test the remote backend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/remote_backend.hpp"
#include "backend/parameter_provider_proxy.hpp"
#include "backend/file_provider_proxy.hpp"

#include "mocks/mock_receiver.hpp"
#include "mocks/mock_sender.hpp"
#include "mocks/mock_managed_object_store.hpp"
#include "mocks/mock_backend_extended.hpp"
#include <wago/wdx/test/mock_parameter_provider.hpp>
#include <wago/wdx/test/mock_file_provider.hpp>
#include "mocks/mock_driver.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::test;
using wago::wdx::linuxos::com::remote_backend;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::data_stream;
using wago::wdx::linuxos::com::parameter_provider_proxy;
using wago::wdx::linuxos::com::file_provider_proxy;

using wago::wdx::parameter_service_backend_extended_i;
using wago::wdx::response;
using wago::wdx::register_device_request;
using wago::wdx::device_collection_id_t;
using wago::wdx::device_id;
using wago::wdx::parameter_provider_i;
using wago::wdx::model_provider_i;
using wago::wdx::device_description_provider_i;
using wago::wdx::device_extension_provider_i;
using wago::wdx::register_file_provider_request;
using wago::wdx::reregister_file_provider_request;
using wago::wdx::register_file_provider_response;
using wago::wdx::file_provider_i;

using testing::Return;
using testing::ReturnRef;
using testing::ByMove;
using testing::Invoke;
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

class remote_backend_fixture : public testing::Test
{
private:
    std::unique_ptr<mock_receiver> temp_receiver = std::make_unique<mock_receiver>();
public:
    mock_receiver             &receiver_mock;
    mock_sender                sender_mock;
    mock_driver                driver_mock;
    mock_managed_object_store  store_mock;
    mock_backend_extended      core_backend_mock;
    
    std::string test_connection_name = "Test";

    remote_backend backend;
    
    remote_backend_fixture()
    : receiver_mock(*temp_receiver)
    , backend(core_backend_mock, std::move(temp_receiver), store_mock)
    { }

    ~remote_backend_fixture() override = default;

    void SetUp() override
    {
        receiver_mock.set_default_expectations();
        sender_mock.set_default_expectations();
        driver_mock.set_default_expectations();
        store_mock.set_default_expectations();
        core_backend_mock.set_default_expectations();

        EXPECT_CALL(store_mock, get_connection_name())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(test_connection_name));
        EXPECT_CALL(store_mock, get_sender())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(sender_mock));
        EXPECT_CALL(store_mock, get_driver())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(driver_mock));
    }
};

TEST_F(remote_backend_fixture, construct_delete)
{
    // nothing to do
}

TEST_F(remote_backend_fixture, remote_create_parameter_provider_proxies)
{
    uint32_t test_count = 42;

    store_mock.expect_object_ids_to_be_generated(0, test_count - 1);
    auto added_objects = store_mock.expect_objects_to_be_added<parameter_provider_proxy>(0, test_count - 1);

    backend.remote_create_parameter_provider_proxies(test_count);
}

TEST_F(remote_backend_fixture, remote_create_file_provider_proxies)
{
    uint32_t test_count = 42;

    store_mock.expect_object_ids_to_be_generated(0, test_count - 1);
    auto added_objects = store_mock.expect_objects_to_be_added<file_provider_proxy>(0, test_count - 1);

    backend.remote_create_file_provider_proxies(test_count);
}

TEST_F(remote_backend_fixture, remote_register_and_auto_unregister_parameter_providers)
{
    uint32_t test_id = 53;

    std::vector<wago::wdx::response> core_response = {
        wago::wdx::response(wago::wdx::status_codes::success)
    };

    parameter_provider_proxy test_provider_proxy(test_id, test_connection_name, sender_mock, driver_mock);
    store_mock.expect_object_to_be_get(test_provider_proxy);

    auto call_mode = wago::wdx::parameter_provider_call_mode::concurrent;
    EXPECT_CALL(core_backend_mock, register_parameter_providers(testing::_, call_mode))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_response)))));
    EXPECT_CALL(core_backend_mock, unregister_parameter_providers(testing::_))
        .Times(1);

    backend.remote_register_parameter_providers({test_id}, call_mode);
}

TEST_F(remote_backend_fixture, remote_unregister_parameter_providers)
{
    uint32_t test_id = 53;

    std::vector<wago::wdx::response> core_response = {
        wago::wdx::response(wago::wdx::status_codes::success)
    };

    parameter_provider_proxy test_provider_proxy(test_id, test_connection_name, sender_mock, driver_mock);
    store_mock.expect_object_to_be_get(test_provider_proxy);
    store_mock.expect_object_meta_to_be_get(test_provider_proxy.get_id());
    store_mock.expect_object_to_be_removed(test_provider_proxy.get_id());

    EXPECT_CALL(core_backend_mock, unregister_parameter_providers(testing::_))
        .Times(1);

    backend.remote_unregister_parameter_providers({test_id});
}

TEST_F(remote_backend_fixture, remote_register_and_auto_unregister_file_providers)
{
    uint32_t test_id = 53;
    std::string test_file_id = "testfile";
    wago::wdx::parameter_id_t test_context = 1337;

    std::vector<wago::wdx::register_file_provider_response> core_register_response = {
        wago::wdx::register_file_provider_response(test_file_id)
    };

    file_provider_proxy test_provider_proxy(test_id, test_connection_name, sender_mock, driver_mock);
    store_mock.expect_object_to_be_get(test_provider_proxy);

    auto call_mode = wago::wdx::provider_call_mode::concurrent;
    EXPECT_CALL(core_backend_mock, register_file_providers(testing::_, call_mode))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_register_response)))));
    EXPECT_CALL(core_backend_mock, unregister_file_providers(testing::_))
        .Times(1);

    backend.remote_register_file_providers({test_id}, call_mode, {test_context});
}

TEST_F(remote_backend_fixture, remote_reregister_and_auto_unregister_file_providers)
{
    uint32_t test_id = 53;
    std::string test_file_id = "testfile";
    wago::wdx::parameter_id_t test_context = 1337;

    std::vector<wago::wdx::register_file_provider_response> core_reregister_response = {
        wago::wdx::register_file_provider_response(test_file_id)
    };

    file_provider_proxy test_provider_proxy(test_id, test_connection_name, sender_mock, driver_mock);
    store_mock.expect_object_to_be_get(test_provider_proxy);

    auto call_mode = wago::wdx::provider_call_mode::concurrent;
    // FIXME: cannot use following expectation, as there is no operator== defined for reregister_file_provider_request
    std::vector<reregister_file_provider_request> register_requests = {
        reregister_file_provider_request(&test_provider_proxy, test_file_id, test_context)
    };
    EXPECT_CALL(core_backend_mock, reregister_file_providers(::testing::_, call_mode))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_reregister_response)))));
    
    std::vector<file_provider_i*> unregister_requests = {
        &test_provider_proxy
    };
    EXPECT_CALL(core_backend_mock, unregister_file_providers(::testing::Eq(unregister_requests)))
        .Times(1);

    backend.remote_reregister_file_providers({test_id}, call_mode, {test_context}, {test_file_id});
}

TEST_F(remote_backend_fixture, remote_unregister_file_providers)
{
    uint32_t test_id = 53;

    std::vector<wago::wdx::response> core_response = {
        wago::wdx::response(wago::wdx::status_codes::success)
    };

    file_provider_proxy test_provider_proxy(test_id, test_connection_name, sender_mock, driver_mock);
    store_mock.expect_object_to_be_get(test_provider_proxy);
    store_mock.expect_object_meta_to_be_get(test_provider_proxy.get_id());
    store_mock.expect_object_to_be_removed(test_provider_proxy.get_id());

    EXPECT_CALL(core_backend_mock, unregister_file_providers(testing::_))
        .Times(1);

    backend.remote_unregister_file_providers({test_id});
}

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
MATCHER_P(RegisterDeviceRequestEq, other, "Equality matcher for vector of register_device_request") {
    bool match = arg.size() == other.size();
    size_t i = 0;
    while(match && (i < arg.size()))
    {
        match =     match
                 && (arg[i].device_id        == other[i].device_id)
                 && (arg[i].order_number     == other[i].order_number)
                 && (arg[i].firmware_version == other[i].firmware_version);
        ++i;
    }
    return match;
}
GNUC_DIAGNOSTIC_POP

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
MATCHER_P(DeviceIdEq, other, "Equality matcher for vector of device_id") {
    bool match = arg.size() == other.size();
    size_t i = 0;
    while(match && (i < arg.size()))
    {
        match = match && arg[i] == other[i];
        ++i;
    }
    return match;
}
GNUC_DIAGNOSTIC_POP

TEST_F(remote_backend_fixture, remote_register_and_unregister_devices)
{
    std::vector<wago::wdx::register_device_request> register_requests = {
        { wago::wdx::device_id(12, 32), "1234-5678",           "17.11.38" },
        { wago::wdx::device_id(11, 72), "0008-0015/3210-4739", "00.08.15" },
        { wago::wdx::device_id( 1, 18), "9876-5432",           "00.00.01" }
    };
    std::vector<wago::wdx::device_id> unregister_requests = {
        register_requests[0].device_id,
        register_requests[1].device_id,
        register_requests[2].device_id
    };

    std::vector<wago::wdx::response> core_register_response = {
        wago::wdx::response(wago::wdx::status_codes::success),
        wago::wdx::response(wago::wdx::status_codes::success),
        wago::wdx::response(wago::wdx::status_codes::success)
    };

    std::vector<wago::wdx::response> core_unregister_response = {
        wago::wdx::response(wago::wdx::status_codes::success),
        wago::wdx::response(wago::wdx::status_codes::success),
        wago::wdx::response(wago::wdx::status_codes::success)
    };

    EXPECT_CALL(core_backend_mock, register_devices(RegisterDeviceRequestEq(register_requests)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_register_response)))));
    EXPECT_CALL(core_backend_mock, unregister_devices(DeviceIdEq(unregister_requests)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_unregister_response)))));

    backend.remote_register_devices(    register_requests);
    backend.remote_unregister_devices(unregister_requests);
}

TEST_F(remote_backend_fixture, remote_unregister_all_devices)
{
    std::vector<wago::wdx::register_device_request> register_requests = {
        { wago::wdx::device_id(250, 2), "1294-5378",           "01.00.00" },
        { wago::wdx::device_id(128, 1), "0008-0015/3211-4739", "00.08.15" },
        { wago::wdx::device_id( 54, 1), "9556-5432",           "00.01.00" }
    };
    wago::wdx::device_collection_id_t device_collection_to_clear = 1;

    std::vector<wago::wdx::response> core_response = {
        wago::wdx::response(wago::wdx::status_codes::success),
        wago::wdx::response(wago::wdx::status_codes::success),
        wago::wdx::response(wago::wdx::status_codes::success)
    };
    wago::wdx::response core_unregister_response(wago::wdx::status_codes::success);

    EXPECT_CALL(core_backend_mock, register_devices(RegisterDeviceRequestEq(register_requests)))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_response)))));
    EXPECT_CALL(core_backend_mock, unregister_all_devices(device_collection_to_clear))
        .Times(1)
        .WillRepeatedly(Return(ByMove(wago::resolved_future(std::move(core_unregister_response)))));

    backend.remote_register_devices(register_requests);
    backend.remote_unregister_all_devices(device_collection_to_clear);
}


//---- End of source file ------------------------------------------------------
