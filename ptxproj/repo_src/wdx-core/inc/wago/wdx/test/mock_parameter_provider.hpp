//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Mock parameter provider.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_MOCK_PARAMETER_PROVIDER_HPP_
#define INC_WAGO_WDX_TEST_MOCK_PARAMETER_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_provider_i.hpp>

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
/// This is a mock implementation for the parameter provider.
/// \copydoc wago::wdx::parameter_provider_i
class mock_parameter_provider : public wdx::parameter_provider_i
{
public:
    ~mock_parameter_provider() noexcept override = default;


    /// Mock implementation of wago::wdx::parameter_provider_i::display_name
    /// \copydoc wago::wdx::parameter_provider_i::display_name
    MOCK_METHOD0(display_name, std::string());

    /// Mock implementation of wago::wdx::parameter_provider_i::get_provided_parameters
    /// \copydoc wago::wdx::parameter_provider_i::get_provided_parameters
    MOCK_METHOD0(get_provided_parameters, wdx::parameter_selector_response());

    /// Mock implementation of wago::wdx::parameter_provider_i::get_parameter_values
    /// \copydoc wago::wdx::parameter_provider_i::get_parameter_values
    MOCK_METHOD1(get_parameter_values, future<std::vector<wdx::value_response>>(std::vector<wdx::parameter_instance_id> parameter_ids));

    /// Mock implementation of wago::wdx::parameter_provider_i::set_parameter_values
    /// \copydoc wago::wdx::parameter_provider_i::set_parameter_values
    MOCK_METHOD1(set_parameter_values, future<std::vector<wdx::set_parameter_response>>(std::vector<wdx::value_request> value_requests));

    /// Mock implementation of wago::wdx::parameter_provider_i::invoke_method
    /// \copydoc wago::wdx::parameter_provider_i::invoke_method
    MOCK_METHOD2(invoke_method, future<wdx::method_invocation_response>(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args));

    /// Mock implementation of wago::wdx::parameter_provider_i::set_parameter_values_connection_aware
    /// \copydoc wago::wdx::parameter_provider_i::set_parameter_values_connection_aware
    MOCK_METHOD2(set_parameter_values_connection_aware, future<std::vector<wdx::set_parameter_response>>(std::vector<wdx::value_request> value_requests, bool defer_wda_web_connection_changes));

    /// Mock implementation of wago::wdx::parameter_provider_i::create_parameter_upload_id
    /// \copydoc wago::wdx::parameter_provider_i::create_parameter_upload_id
    MOCK_METHOD1(create_parameter_upload_id, future<wdx::register_file_provider_response>(wdx::parameter_id_t context));

    /// Mock implementation of wago::wdx::parameter_provider_i::remove_parameter_upload_id
    /// \copydoc wago::wdx::parameter_provider_i::remove_parameter_upload_id
    MOCK_METHOD2(remove_parameter_upload_id, future<wdx::response>(wdx::file_id id, wdx::parameter_id_t context));

    /// Expect mocked methods not to be called.
    void set_default_expectations()
    {
        EXPECT_CALL(*this, display_name())
            .Times(0);
        EXPECT_CALL(*this, get_provided_parameters())
            .Times(0);
        EXPECT_CALL(*this, get_parameter_values(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_parameter_values(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, invoke_method(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_parameter_values_connection_aware(::testing::_, ::testing::_))
            .Times(0);
        EXPECT_CALL(*this, create_parameter_upload_id(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, remove_parameter_upload_id(::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_MOCK_PARAMETER_PROVIDER_HPP_
//---- End of source file ------------------------------------------------------
