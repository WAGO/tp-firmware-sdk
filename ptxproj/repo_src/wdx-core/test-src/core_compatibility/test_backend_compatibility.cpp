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
///  \brief    Test WAGO Parameter Service Core backend compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/test/mock_backend.hpp>
#include <wago/wdx/test/wda_check.hpp>
#include "wago/wdx/file_transfer/base_file_provider.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;
using testing::Exactly;
using testing::Return;
using testing::ByMove;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class backend_fixture : public ::testing::Test
{
protected:
    // static mocks
    wago::wdx::test::mock_backend  backend_mock;

    // test specifica
    parameter_service_backend_i   *test_backend = &backend_mock;

protected:
    backend_fixture() = default;
    ~backend_fixture() override = default;

    void SetUp() override
    {
        // Set default call expectations
        backend_mock.set_default_expectations();
    }
};

TEST_F(backend_fixture, SourceCompatibilityOnConstructDelete)
{
    // Noting to do, everything is done in SetUp/TearDown
}

TEST_F(backend_fixture, SourceCompatibilityOnDeviceRegistration)
{
    // Prepare devices to register
    slot_index_t const device_0_slot = 42;
    slot_index_t const device_1_slot = 0;
    slot_index_t const device_2_slot = 1;
    device_collection_id_t const device_1_collection_id = device_collections::root;
    device_collection_id_t const device_2_collection_id = device_collections::rlb;
    char const device_0_order_number[] = "0123-4567/K010-0042/RN00-0001";
    char const device_1_order_number[] = "0768-3301";
    char const device_2_order_number[] = "0763-1108";
    char const device_0_firmware_version[] = "01.01.25";
    char const device_1_firmware_version[] = "01.00.00";
    char const device_2_firmware_version[] = "01.00.00";
    std::vector<wago::wdx::register_device_request> device_requests = { { device_0_slot,                           device_0_order_number, device_0_firmware_version },
                                                                      { { device_1_slot, device_1_collection_id }, device_1_order_number, device_1_firmware_version },
                                                                      { { device_2_slot, device_2_collection_id }, device_2_order_number, device_2_firmware_version } };
    EXPECT_EQ(3, device_requests.size());
    EXPECT_EQ(device_0_slot, device_requests.at(0).device_id.slot);
    EXPECT_EQ(device_collections::root, device_requests.at(0).device_id.device_collection_id);
    EXPECT_STREQ(device_0_order_number, device_requests.at(0).order_number.c_str());
    EXPECT_STREQ(device_0_firmware_version, device_requests.at(0).firmware_version.c_str());

    // Set mock expectations
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_devices(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->register_devices(device_requests);
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnAllDevicesUnregistration)
{
    // Set mock expectations
    wago::wdx::response test_result;
    wago::wdx::response result_dummy = test_result;
    EXPECT_CALL(backend_mock, unregister_all_devices(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->unregister_all_devices(device_collections::rlb);
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_result.status, result.status);
}

TEST_F(backend_fixture, SourceCompatibilityOnDevicesUnregistration)
{
    // Set mock expectations
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, unregister_devices(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->unregister_devices({device_id::headstation});
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    ASSERT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterParameterProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_parameter_providers(::testing::_, wago::wdx::parameter_provider_call_mode::concurrent))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->register_parameter_providers({});
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterParameterProvidersWithCallMode)
{
    // Set mock expectations
    auto call_mode = wago::wdx::parameter_provider_call_mode::serialized;
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_parameter_providers(::testing::_, call_mode))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->register_parameter_providers({}, call_mode);
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnUnregisterParameterProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> result_dummy;
    EXPECT_CALL(backend_mock, unregister_parameter_providers(::testing::_))
        .Times(Exactly(1));

    // Test interface
    test_backend->unregister_parameter_providers({});
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterSingleParameterProvider)
{
    // Prepare promise
    wago::promise<std::vector<wago::wdx::response>> response_promise;

    // Set mock expectations
    auto test_response = response(wago::wdx::status_codes::logic_error);
    auto test_result   = std::vector<wago::wdx::response>( { test_response } );
    auto result_dummy  = test_result;
    EXPECT_CALL(backend_mock, register_parameter_providers(::testing::_, wago::wdx::parameter_provider_call_mode::concurrent))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(response_promise.get_future())));

    // Test interface
    auto future_result = test_backend->register_parameter_provider(reinterpret_cast<wago::wdx::parameter_provider_i*>(1));

    // Solve promise
    response_promise.set_value(std::move(result_dummy));

    // Test future result
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_response.status, result.status);
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterModelProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_model_providers(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->register_model_providers({});
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnUnregisterModelProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> result_dummy;
    EXPECT_CALL(backend_mock, unregister_model_providers(::testing::_))
        .Times(Exactly(1));

    // Test interface
    test_backend->unregister_model_providers({});
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterSingleModelProvider)
{
    // Prepare promise
    wago::promise<std::vector<wago::wdx::response>> response_promise;

    // Set mock expectations
    auto test_response = response(wago::wdx::status_codes::logic_error);
    auto test_result   = std::vector<wago::wdx::response>( { test_response } );
    auto result_dummy  = test_result;
    EXPECT_CALL(backend_mock, register_model_providers(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(response_promise.get_future())));

    // Test interface
    auto future_result = test_backend->register_model_provider(reinterpret_cast<wago::wdx::model_provider_i*>(1));

    // Solve promise
    response_promise.set_value(std::move(result_dummy));

    // Test future result
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_response.status, result.status);
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterDeviceExtensionProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_device_extension_providers(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->register_device_extension_providers({});
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnUnregisterDeviceExtensionProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> result_dummy;
    EXPECT_CALL(backend_mock, unregister_device_extension_providers(::testing::_))
        .Times(Exactly(1));

    // Test interface
    test_backend->unregister_device_extension_providers({});
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterSingleDeviceExtensionProvider)
{
    // Prepare promise
    wago::promise<std::vector<wago::wdx::response>> response_promise;

    // Set mock expectations
    auto test_response = response(wago::wdx::status_codes::logic_error);
    auto test_result   = std::vector<wago::wdx::response>( { test_response } );
    auto result_dummy  = test_result;
    EXPECT_CALL(backend_mock, register_device_extension_providers(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(response_promise.get_future())));

    // Test interface
    auto future_result = test_backend->register_device_extension_provider(reinterpret_cast<wago::wdx::device_extension_provider_i*>(1));

    // Solve promise
    response_promise.set_value(std::move(result_dummy));

    // Test future result
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_response.status, result.status);
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterDeviceDescriptionProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> test_result;
    std::vector<wago::wdx::response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_device_description_providers(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    auto future_result = test_backend->register_device_description_providers({});
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
}

TEST_F(backend_fixture, SourceCompatibilityOnUnregisterDeviceDescriptionProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> result_dummy;
    EXPECT_CALL(backend_mock, unregister_device_description_providers(::testing::_))
        .Times(Exactly(1));

    // Test interface
    test_backend->unregister_device_description_providers({});
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterSingleDeviceDescriptionProvider)
{
    // Prepare promise
    wago::promise<std::vector<wago::wdx::response>> response_promise;

    // Set mock expectations
    auto test_response = response(wago::wdx::status_codes::logic_error);
    auto test_result   = std::vector<wago::wdx::response>( { test_response } );
    auto result_dummy  = test_result;
    EXPECT_CALL(backend_mock, register_device_description_providers(::testing::_))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(response_promise.get_future())));

    // Test interface
    auto future_result = test_backend->register_device_description_provider(reinterpret_cast<wago::wdx::device_description_provider_i*>(1));

    // Solve promise
    response_promise.set_value(std::move(result_dummy));

    // Test future result
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_response.status, result.status);
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterFileProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::register_file_provider_response> test_result  = { register_file_provider_response() };
    std::vector<wago::wdx::register_file_provider_response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, wago::wdx::provider_call_mode::concurrent))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    wago::wdx::file_transfer::base_file_provider test_provider;
    wago::wdx::parameter_id_t context = 0;
    auto future_result = test_backend->register_file_providers({register_file_provider_request(&test_provider, context)});
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
    EXPECT_WDA_STATUS(test_result.at(0).status, result.at(0).status);
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterFileProvidersWithCallMode)
{
    // Set mock expectations
    auto call_mode = wago::wdx::provider_call_mode::serialized;
    std::vector<wago::wdx::register_file_provider_response> test_result  = { register_file_provider_response() };
    std::vector<wago::wdx::register_file_provider_response> result_dummy = test_result;
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, call_mode))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(wago::resolved_future(std::move(result_dummy)))));

    // Test interface
    wago::wdx::file_transfer::base_file_provider test_provider;
    wago::wdx::parameter_id_t context = 0;
    auto future_result = test_backend->register_file_providers({register_file_provider_request(&test_provider, context)}, call_mode);
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_EQ(test_result.size(), result.size());
    EXPECT_WDA_STATUS(test_result.at(0).status, result.at(0).status);
}

TEST_F(backend_fixture, SourceCompatibilityOnUnregisterFileProviders)
{
    // Set mock expectations
    std::vector<wago::wdx::response> result_dummy = { wago::wdx::response() };
    EXPECT_CALL(backend_mock, unregister_file_providers(::testing::_))
        .Times(Exactly(1));

    // Test interface
    wago::wdx::file_transfer::base_file_provider test_provider;
    test_backend->unregister_file_providers({&test_provider});
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterSingleFileProvider)
{
    wago::wdx::parameter_id_t context = 42;

    // Prepare promise
    wago::promise<std::vector<wago::wdx::register_file_provider_response>> response_promise;

    // Set mock expectations
    auto call_mode     = wago::wdx::provider_call_mode::concurrent;
    auto test_response = register_file_provider_response(wago::wdx::status_codes::logic_error);
    auto test_result   = std::vector<wago::wdx::register_file_provider_response>( { test_response } );
    auto result_dummy  = test_result;
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, call_mode))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(response_promise.get_future())));

    // Test interface
    auto future_result = test_backend->register_file_provider(reinterpret_cast<wago::wdx::file_provider_i*>(1), context);

    // Solve promise
    response_promise.set_value(std::move(result_dummy));

    // Test future result
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_response.status, result.status);
}

TEST_F(backend_fixture, SourceCompatibilityOnRegisterSingleFileProviderRequest)
{
    wago::wdx::parameter_id_t context = 53;

    // Prepare promise
    wago::promise<std::vector<wago::wdx::register_file_provider_response>> response_promise;

    // Set mock expectations
    auto call_mode        = wago::wdx::provider_call_mode::concurrent;
    auto register_request = wago::wdx::register_file_provider_request(reinterpret_cast<wago::wdx::file_provider_i*>(1), context);
    auto test_response    = register_file_provider_response(wago::wdx::status_codes::logic_error);
    auto test_result      = std::vector<wago::wdx::register_file_provider_response>( { test_response } );
    auto result_dummy     = test_result;
    EXPECT_CALL(backend_mock, register_file_providers(::testing::_, call_mode))
        .Times(Exactly(1))
        .WillOnce(Return(ByMove(response_promise.get_future())));

    // Test interface
    auto future_result = test_backend->register_file_provider(register_request);

    // Solve promise
    response_promise.set_value(std::move(result_dummy));

    // Test future result
    ASSERT_FUTURE_VALUE(future_result);
    auto result = future_result.get();
    EXPECT_WDA_STATUS(test_response.status, result.status);
}


//---- End of source file ------------------------------------------------------
