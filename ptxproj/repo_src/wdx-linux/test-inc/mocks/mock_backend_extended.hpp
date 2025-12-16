//------------------------------------------------------------------------------
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
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
///  \brief    Mock service core backend (extended).
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_BACKEND_EXTENDED_HPP_
#define TEST_INC_MOCKS_MOCK_BACKEND_EXTENDED_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_service_backend_extended_i.hpp>

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")

class mock_backend_extended : public wago::wdx::parameter_service_backend_extended_i
{
public:
    ~mock_backend_extended() override = default;

    MOCK_METHOD1(register_devices, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::register_device_request> requests));
    MOCK_METHOD1(unregister_all_devices, wago::future<wago::wdx::response> (wago::wdx::device_collection_id_t deviceCollection));
    MOCK_METHOD1(unregister_devices, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::device_id> device_ids));
    MOCK_METHOD1(register_parameter_providers, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::parameter_provider_i*> providers));
    MOCK_METHOD2(register_parameter_providers, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::parameter_provider_i*> providers, wago::wdx::parameter_provider_call_mode mode));
    MOCK_METHOD1(unregister_parameter_providers, void (std::vector<wago::wdx::parameter_provider_i*> providers));
    MOCK_METHOD1(register_model_providers, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::model_provider_i*> providers));
    MOCK_METHOD1(unregister_model_providers, void (std::vector<wago::wdx::model_provider_i*> providers));
    MOCK_METHOD1(register_device_description_providers, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::device_description_provider_i*> providers));
    MOCK_METHOD1(unregister_device_description_providers, void (std::vector<wago::wdx::device_description_provider_i*> providers));
    MOCK_METHOD1(register_device_extension_providers, wago::future<std::vector<wago::wdx::response>> (std::vector<wago::wdx::device_extension_provider_i*> providers));
    MOCK_METHOD1(unregister_device_extension_providers, void (std::vector<wago::wdx::device_extension_provider_i*> providers));
    MOCK_METHOD2(register_file_providers, wago::future<std::vector<wago::wdx::register_file_provider_response>> (std::vector<wago::wdx::register_file_provider_request> requests, wago::wdx::provider_call_mode mode));
    MOCK_METHOD1(unregister_file_providers, void (std::vector<wago::wdx::file_provider_i *> providers));
    MOCK_METHOD2(reregister_file_providers, wago::future<std::vector<wago::wdx::register_file_provider_response>> (std::vector<wago::wdx::reregister_file_provider_request> requests, wago::wdx::provider_call_mode mode));

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
        EXPECT_CALL(*this, reregister_file_providers(::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP

#endif // TEST_INC_MOCKS_MOCK_BACKEND_EXTENDED_HPP_
//---- End of source file ------------------------------------------------------
