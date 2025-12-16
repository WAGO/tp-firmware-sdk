//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    Mock remote backend interface.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_REMOTE_BACKEND_HPP_
#define TEST_INC_MOCKS_MOCK_REMOTE_BACKEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend/remote_backend_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::future;

using wago::wdx::linuxos::com::remote_backend_i;
using wago::wdx::linuxos::com::managed_object_id;
using wago::wdx::linuxos::com::no_return;

using wago::wdx::register_device_request;
using wago::wdx::response;
using wago::wdx::device_collection_id_t;
using wago::wdx::device_id;
using wago::wdx::parameter_id_t;
using wago::wdx::register_file_provider_response;
using wago::wdx::parameter_provider_call_mode;
using wago::wdx::provider_call_mode;
using wago::wdx::parameter_selector_response;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_remote_backend : public remote_backend_i
{
public:
    MOCK_METHOD1(remote_register_devices,       future<std::vector<response>> (std::vector<register_device_request> requests));
    MOCK_METHOD1(remote_unregister_devices,     future<std::vector<response>> (std::vector<device_id>               device_ids));
    MOCK_METHOD1(remote_unregister_all_devices, future<response>              (device_collection_id_t               deviceCollection));

    MOCK_METHOD1(remote_create_parameter_provider_proxies,          future<std::vector<managed_object_id>> (uint32_t count));
    MOCK_METHOD1(remote_create_model_provider_proxies,              future<std::vector<managed_object_id>> (uint32_t count));
    MOCK_METHOD1(remote_create_device_description_provider_proxies, future<std::vector<managed_object_id>> (uint32_t count));
    MOCK_METHOD1(remote_create_device_extension_provider_proxies,   future<std::vector<managed_object_id>> (uint32_t count));
    MOCK_METHOD1(remote_create_file_provider_proxies,               future<std::vector<managed_object_id>> (uint32_t count));

    MOCK_METHOD3(remote_update_parameter_provider_proxies,          future<no_return> (std::vector<managed_object_id>           object_ids,
                                                                                       std::vector<std::string>                 names,
                                                                                       std::vector<parameter_selector_response> selected_parameters));

    MOCK_METHOD2(remote_register_parameter_providers,            future<std::vector<response>> (std::vector<managed_object_id> object_ids, parameter_provider_call_mode mode));
    MOCK_METHOD1(remote_unregister_parameter_providers,          future<no_return>             (std::vector<managed_object_id> object_ids));
    MOCK_METHOD2(remote_register_model_providers,                future<std::vector<response>> (std::vector<managed_object_id> object_ids, provider_call_mode           mode));
    MOCK_METHOD1(remote_unregister_model_providers,              future<no_return>             (std::vector<managed_object_id> object_ids));
    MOCK_METHOD2(remote_register_device_description_providers,   future<std::vector<response>> (std::vector<managed_object_id> object_ids, provider_call_mode           mode));
    MOCK_METHOD1(remote_unregister_device_description_providers, future<no_return>             (std::vector<managed_object_id> object_ids));
    MOCK_METHOD2(remote_register_device_extension_providers,     future<std::vector<response>> (std::vector<managed_object_id> object_ids, provider_call_mode           mode));
    MOCK_METHOD1(remote_unregister_device_extension_providers,   future<no_return>             (std::vector<managed_object_id> object_ids));

    MOCK_METHOD3(remote_register_file_providers,   future<std::vector<register_file_provider_response>> (std::vector<managed_object_id> object_ids, provider_call_mode mode, std::vector<parameter_id_t> contexts));
    MOCK_METHOD1(remote_unregister_file_providers, future<no_return>                                    (std::vector<managed_object_id> object_ids));
    MOCK_METHOD4(remote_reregister_file_providers, future<std::vector<register_file_provider_response>> (std::vector<managed_object_id> object_ids, provider_call_mode mode, std::vector<parameter_id_t> contexts, std::vector<wago::wdx::file_id> file_ids));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, remote_register_devices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_devices(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_all_devices(::testing::_))
            .Times(0);

        EXPECT_CALL(*this, remote_create_parameter_provider_proxies(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_create_model_provider_proxies(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_create_device_description_provider_proxies(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_create_device_extension_provider_proxies(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_create_file_provider_proxies(::testing::_))
            .Times(0);

        EXPECT_CALL(*this, remote_update_parameter_provider_proxies(::testing::_, ::testing::_, ::testing::_))
            .Times(0);

        EXPECT_CALL(*this, remote_register_parameter_providers(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_parameter_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_register_model_providers(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_model_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_register_device_description_providers(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_device_description_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_register_device_extension_providers(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_device_extension_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_register_file_providers(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_unregister_file_providers(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remote_reregister_file_providers(::testing::_, ::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_REMOTE_BACKEND_HPP_
//---- End of source file ------------------------------------------------------
