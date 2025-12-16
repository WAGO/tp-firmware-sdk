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
///  \brief    Mock frontend.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_MOCK_FRONTEND_HPP_
#define INC_WAGO_WDX_TEST_MOCK_FRONTEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_service_frontend_i.hpp>

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
/// This is a mock implementation for the parameter service frontend.
/// \copydoc wago::wdx::parameter_service_frontend_i
class mock_frontend : public wdx::parameter_service_frontend_i
{
public:
    ~mock_frontend() noexcept override = default;

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::get_all_devices
    /// \copydoc wago::wdx::parameter_service_frontend_i::get_all_devices
    MOCK_METHOD0(get_all_devices, wago::future<wdx::device_collection_response>());

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::get_all_parameters
    /// \copydoc wago::wdx::parameter_service_frontend_i::get_all_parameters
    MOCK_METHOD3(get_all_parameters, wago::future<wdx::parameter_response_list_response>(wdx::parameter_filter filter, size_t paging_offset, size_t paging_limit));

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::get_parameters
    /// \copydoc wago::wdx::parameter_service_frontend_i::get_parameters
    MOCK_METHOD1(get_parameters, wago::future<std::vector<wdx::parameter_response>>(std::vector<wdx::parameter_instance_id> ids));

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::get_parameters_by_path
    /// \copydoc wago::wdx::parameter_service_frontend_i::get_parameters_by_path
    MOCK_METHOD1(get_parameters_by_path, wago::future<std::vector<wdx::parameter_response>>(std::vector<wdx::parameter_instance_path> paths));

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::invoke_method
    /// \copydoc wago::wdx::parameter_service_frontend_i::invoke_method
    MOCK_METHOD2(invoke_method, wago::future<wdx::method_invocation_named_response>(wdx::parameter_instance_id method_id, std::map<std::string, std::shared_ptr<wdx::parameter_value>> in_args));

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::invoke_method_by_path
    /// \copydoc wago::wdx::parameter_service_frontend_i::invoke_method_by_path
    MOCK_METHOD2(invoke_method_by_path, wago::future<wdx::method_invocation_named_response>(wdx::parameter_instance_path method_path, std::map<std::string, std::shared_ptr<wdx::parameter_value>> in_args));

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::set_parameter_values
    /// \copydoc wago::wdx::parameter_service_frontend_i::set_parameter_values
    MOCK_METHOD1(set_parameter_values, wago::future<std::vector<wdx::set_parameter_response>>(std::vector<wdx::value_request> value_requests));

    /// Mock implementation of wago::wdx::parameter_service_frontend_i::set_parameter_values_by_path
    /// \copydoc wago::wdx::parameter_service_frontend_i::set_parameter_values_by_path
    MOCK_METHOD1(set_parameter_values_by_path, wago::future<std::vector<wdx::set_parameter_response>>(std::vector<wdx::value_path_request> value_path_requests));

    /// Expect mocked methods not to be called.
    void set_default_expectations()
    {
        EXPECT_CALL(*this, get_all_devices())
            .Times(0);
        EXPECT_CALL(*this, get_all_parameters(testing::_, testing::_, testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_parameters(testing::_))
            .Times(0);
        EXPECT_CALL(*this, get_parameters_by_path(testing::_))
            .Times(0);
        EXPECT_CALL(*this, invoke_method(testing::_, testing::_))
            .Times(0);
        EXPECT_CALL(*this, invoke_method_by_path(testing::_, testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_parameter_values(testing::_))
            .Times(0);
        EXPECT_CALL(*this, set_parameter_values_by_path(testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_MOCK_FRONTEND_HPP_
//---- End of source file ------------------------------------------------------
