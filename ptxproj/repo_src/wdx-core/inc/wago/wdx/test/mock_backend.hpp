//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Mock service core backend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_MOCK_BACKEND_HPP_
#define INC_WAGO_WDX_TEST_MOCK_BACKEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_service_backend_i.hpp>

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
/// This is a mock implementation for the parameter service backend.
/// \copydoc wago::wdx::parameter_service_backend_i
class mock_backend : public wdx::parameter_service_backend_i
{
public:
    ~mock_backend() noexcept override = default;

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_devices
    /// \copydoc wago::wdx::parameter_service_backend_i::register_devices
    MOCK_METHOD1(register_devices, future<std::vector<wdx::response>> (std::vector<wdx::register_device_request> requests));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_all_devices
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_all_devices
    MOCK_METHOD1(unregister_all_devices, future<wdx::response> (wdx::device_collection_id_t deviceCollection));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_devices
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_devices
    MOCK_METHOD1(unregister_devices, future<std::vector<wdx::response>> (std::vector<wdx::device_id> device_ids));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_parameter_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::register_parameter_providers
    MOCK_METHOD1(register_parameter_providers, future<std::vector<wdx::response>> (std::vector<wdx::parameter_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_parameter_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::register_parameter_providers
    MOCK_METHOD2(register_parameter_providers, future<std::vector<wdx::response>> (std::vector<wdx::parameter_provider_i*> providers, wdx::parameter_provider_call_mode mode));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_parameter_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_parameter_providers
    MOCK_METHOD1(unregister_parameter_providers, void (std::vector<wdx::parameter_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_model_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::register_model_providers
    MOCK_METHOD1(register_model_providers, future<std::vector<wdx::response>> (std::vector<wdx::model_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_model_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_model_providers
    MOCK_METHOD1(unregister_model_providers, void (std::vector<wdx::model_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_device_description_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::register_device_description_providers
    MOCK_METHOD1(register_device_description_providers, future<std::vector<wdx::response>> (std::vector<wdx::device_description_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_device_description_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_device_description_providers
    MOCK_METHOD1(unregister_device_description_providers, void (std::vector<wdx::device_description_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_device_extension_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::register_device_extension_providers
    MOCK_METHOD1(register_device_extension_providers, future<std::vector<wdx::response>> (std::vector<wdx::device_extension_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_device_extension_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_device_extension_providers
    MOCK_METHOD1(unregister_device_extension_providers, void (std::vector<wdx::device_extension_provider_i*> providers));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::register_file_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::register_file_providers
    MOCK_METHOD2(register_file_providers, future<std::vector<wdx::register_file_provider_response>> (std::vector<wdx::register_file_provider_request> requests, wdx::provider_call_mode mode));

    /// Mock implementation of wago::wdx::parameter_service_backend_i::unregister_file_providers
    /// \copydoc wago::wdx::parameter_service_backend_i::unregister_file_providers
    MOCK_METHOD1(unregister_file_providers, void (std::vector<wdx::file_provider_i *> providers));

    /// Expect mocked methods not to be called.
    void set_default_expectations()
    {
        EXPECT_CALL(*this, register_devices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_all_devices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_devices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, register_parameter_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, register_parameter_providers(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_parameter_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, register_model_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_model_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, register_device_description_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_device_description_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, register_device_extension_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_device_extension_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, register_file_providers(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, unregister_file_providers(::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_MOCK_BACKEND_HPP_
//---- End of source file ------------------------------------------------------
