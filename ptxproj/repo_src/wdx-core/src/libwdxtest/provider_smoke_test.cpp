//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO Parameter Service provider smoke test implementation
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/test/provider_smoke_test.hpp"
#include "wago/wdx/test/wda_check.hpp"

#include <gtest/gtest.h>
#include <wc/structuring.h>
#include <wc/std_includes.h>

#include <nlohmann/json.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
static std::string get_case_description(char const *caller)
{
    return std::string(" --> Case: ") + caller;
}

template <class T>
static void check_future_for_valid_state(char            const *caller,
                                         wago::future<T> const &response_future)
{
    ASSERT_TRUE(response_future.valid()) << get_case_description(caller);
    response_future.wait();
    ASSERT_TRUE(response_future.ready()) << get_case_description(caller);
    EXPECT_FALSE(response_future.has_exception()) << get_case_description(caller);
    ASSERT_TRUE(response_future.has_value()) << get_case_description(caller);
}

static void check_response_for_success(char                const *caller,
                                       wago::wdx::response const &single_response)
{
    EXPECT_TRUE(single_response.is_determined()) << get_case_description(caller);
    EXPECT_FALSE(single_response.has_error()) << get_case_description(caller);
    EXPECT_WDA_SUCCESS(single_response) << get_case_description(caller);
}

static void check_response_for_invalid_request(char                    const *caller,
                                               wago::wdx::response     const &single_response,
                                               wago::wdx::status_codes const accepted_status_code)
{
    if(!single_response.is_determined())
    {
        EXPECT_FALSE(single_response.has_error()) << get_case_description(caller);
        EXPECT_WDA_STATUS(wago::wdx::status_codes::no_error_yet, single_response.status) << get_case_description(caller);
    }
    else
    {
        EXPECT_TRUE(single_response.has_error()) << get_case_description(caller);
        EXPECT_WDA_STATUS(accepted_status_code, single_response.status) << get_case_description(caller);
    }
}

static void parameter_provider_smoke_test_name(wago::wdx::parameter_provider_i &provider)
{
    // Check provided name
    std::string const name = provider.display_name();
    EXPECT_FALSE(name.empty()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
}

static void parameter_provider_smoke_test_provided_ids(wago::wdx::parameter_provider_i &provider)
{
    // Check provided parameter IDs
    auto const selector_response = provider.get_provided_parameters();
    EXPECT_TRUE(selector_response.is_determined()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    EXPECT_FALSE(selector_response.has_error()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    EXPECT_WDA_SUCCESS(selector_response) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    EXPECT_LE(1U, selector_response.selected_parameters.size()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
}

static void parameter_provider_smoke_test_get_invalid_id(wago::wdx::parameter_provider_i &provider)
{
    // Prepare invalid ID
    wago::wdx::parameter_instance_id const invalid_id(UINT32_MAX);

    // Send invalid request
    std::vector<wago::wdx::parameter_instance_id> requests = { invalid_id };
    auto response_future = provider.get_parameter_values(requests);

    // Check response
    check_future_for_valid_state(WC_ARRAY_TO_PTR(__func__), response_future);
    auto const responses = response_future.get();
    ASSERT_EQ(requests.size(), responses.size()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    auto const response = responses.at(0);
    check_response_for_invalid_request(WC_ARRAY_TO_PTR(__func__), response, wago::wdx::status_codes::parameter_not_provided);
}

static void parameter_provider_smoke_test_set_invalid_id(wago::wdx::parameter_provider_i &provider)
{
    // Prepare invalid ID
    wago::wdx::parameter_instance_id const invalid_id(UINT32_MAX);

    // Send invalid request
    wago::wdx::value_request invalid_request(invalid_id, wago::wdx::parameter_value::create_boolean(false));
    std::vector<wago::wdx::value_request> requests = { invalid_request };
    auto response_future = provider.set_parameter_values(requests);

    // Check response
    check_future_for_valid_state(WC_ARRAY_TO_PTR(__func__), response_future);
    auto const responses = response_future.get();
    ASSERT_EQ(requests.size(), responses.size()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    auto const response = responses.at(0);
    check_response_for_invalid_request(WC_ARRAY_TO_PTR(__func__), response, wago::wdx::status_codes::parameter_not_provided);
}

static void parameter_provider_smoke_test_invoke_invalid_id(wago::wdx::parameter_provider_i &provider)
{
    // Prepare invalid ID
    wago::wdx::parameter_instance_id const invalid_id(UINT32_MAX);

    // Send invalid request
    auto invalid_arg = wago::wdx::parameter_value::create_boolean(false);
    std::vector<std::shared_ptr<wago::wdx::parameter_value>> in_args = { invalid_arg };
    auto response_future = provider.invoke_method(invalid_id, in_args);

    // Check response
    check_future_for_valid_state(WC_ARRAY_TO_PTR(__func__), response_future);
    auto const response = response_future.get();
    check_response_for_invalid_request(WC_ARRAY_TO_PTR(__func__), response, wago::wdx::status_codes::parameter_not_provided);
}

/*static void model_provider_smoke_test_model_information(wago::wdx::model_provider_i &provider)
{
    // Check provided model
    auto response_future = provider.get_model_information();

    // Check response
    check_future_for_valid_state(WC_ARRAY_TO_PTR(__func__), response_future);
    auto const response = response_future.get();
    check_response_for_success(WC_ARRAY_TO_PTR(__func__), response);
    if(!response.wdm_content.empty())
    {
        using nlohmann::json;

        // Some basic JSON tests
        json const json_object = json::parse(response.wdm_content);
        EXPECT_FALSE(json_object.is_primitive()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        EXPECT_FALSE(json_object.is_string()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        EXPECT_TRUE(json_object.is_structured()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        EXPECT_TRUE(json_object.is_object()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        EXPECT_TRUE(json_object.contains("Name")) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        EXPECT_TRUE(json_object.contains("WDMMVersion")) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        ASSERT_TRUE(json_object.contains("Features")) << get_case_description(WC_ARRAY_TO_PTR(__func__));
        EXPECT_TRUE(json_object.at("Features").is_array()) << get_case_description(WC_ARRAY_TO_PTR(__func__));

        // TODO: Add extended model check by core function
    }
}

static void device_description_provider_smoke_test_device_information(wago::wdx::device_description_provider_i &provider)
{
    // Check provided device information
    auto const response = provider.get_provided_devices();

    // Check response
    check_response_for_success(WC_ARRAY_TO_PTR(__func__), response);
    EXPECT_FALSE(response.selected_devices.empty()) << get_case_description(WC_ARRAY_TO_PTR(__func__)); // Without supported devices this provider is useless
}

static void device_description_provider_smoke_test_device_description_invalid_device(wago::wdx::device_description_provider_i &provider,
                                                                                     std::string const order_number,
                                                                                     std::string const firmware_version)
{
    // Send invalid request
    auto response_future = provider.get_device_information(order_number, firmware_version);

    // Check response
    check_future_for_valid_state(WC_ARRAY_TO_PTR(__func__), response_future);
    auto const response = response_future.get();
    check_response_for_invalid_request(WC_ARRAY_TO_PTR(__func__), response, wago::wdx::status_codes::internal_error);
}

static void device_description_provider_smoke_test_device_description_invalid_devices(wago::wdx::device_description_provider_i &provider)
{
    // Prepare invalid device requests
    std::string const order_number                 = "0750-8102";
    std::string const order_number_invalid_1       = "0000-0000";
    std::string const order_number_invalid_2       = "0000-0000/0000-0000";
    std::string const order_number_invalid_3       = "0000-0000/K000-0000";
    std::string const order_number_malformed_1     =  "750-1234";
    std::string const order_number_malformed_2     = "0750-123";
    std::string const order_number_malformed_3     = "0000-0000/0000-K000";
    std::string const order_number_malformed_4     = "K000-0000";
    std::string const order_number_malformed_5     = "00000-0000";
    std::string const order_number_malformed_6     = "0000-00000";
    std::string const order_number_malformed_7     = "abcd-efgh";
    std::string const firmware_version             = "00.00.00";
    std::string const firmware_version_malformed_1 =  "0.00.00";
    std::string const firmware_version_malformed_2 =  "0.0.0";
    std::string const firmware_version_malformed_3 =  "1a.01.01";
    std::string const firmware_version_malformed_4 =  "01.1i.01";
    std::string const firmware_version_malformed_5 =  "01.01.1x";

    // Send invalid requests
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_invalid_1, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_invalid_2, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_invalid_3, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_1, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_2, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_3, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_4, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_5, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_6, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number_malformed_7, firmware_version);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number, firmware_version_malformed_1);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number, firmware_version_malformed_2);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number, firmware_version_malformed_3);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number, firmware_version_malformed_4);
    device_description_provider_smoke_test_device_description_invalid_device(provider, order_number, firmware_version_malformed_5);
}

static void device_extension_provider_smoke_test_device_extensions(wago::wdx::device_extension_provider_i &provider)
{
    // Check provided device extensions
    auto response = provider.get_device_extensions();

    // Check response
    check_response_for_success(WC_ARRAY_TO_PTR(__func__), response);
    response.get_description();
}*/

static void file_provider_smoke_test_file_information(wago::wdx::file_provider_i &provider)
{
    // Check provided file information
    auto response_future = provider.get_file_info();

    // Check response
    check_future_for_valid_state(WC_ARRAY_TO_PTR(__func__), response_future);
    auto const response = response_future.get();
    EXPECT_TRUE(response.is_determined()) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    if(!response.has_error())
    {
        EXPECT_WDA_SUCCESS(response) << get_case_description(WC_ARRAY_TO_PTR(__func__));
    }
    else
    {
        EXPECT_TRUE(    (response.status == wago::wdx::status_codes::file_not_accessible)
                     || (response.status == wago::wdx::status_codes::internal_error))
          << get_case_description(WC_ARRAY_TO_PTR(__func__));
    }
}


//------------------------------------------------------------------------------
// public function implementation
//------------------------------------------------------------------------------
void parameter_provider_smoke_test(wago::wdx::parameter_provider_i &provider)
{
    parameter_provider_smoke_test_name(provider);
    parameter_provider_smoke_test_provided_ids(provider);
    parameter_provider_smoke_test_get_invalid_id(provider);
    parameter_provider_smoke_test_set_invalid_id(provider);
    parameter_provider_smoke_test_invoke_invalid_id(provider);
}

/*void model_provider_smoke_test(wago::wdx::model_provider_i &provider)
{
    model_provider_smoke_test_model_information(provider);
}

void device_description_provider_smoke_test(wago::wdx::device_description_provider_i &provider)
{
    device_description_provider_smoke_test_device_information(provider);
    device_description_provider_smoke_test_device_description_invalid_devices(provider);
}

void device_extension_provider_smoke_test(wago::wdx::device_extension_provider_i &provider)
{
    device_extension_provider_smoke_test_device_extensions(provider);
}*/

void file_provider_smoke_test(wago::wdx::file_provider_i &provider)
{
    file_provider_smoke_test_file_information(provider);
}


} // Namespace test
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
