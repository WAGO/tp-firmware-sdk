//------------------------------------------------------------------------------
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
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
///  \brief    Test WAGO Parameter Service Core response compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/responses.hpp"
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;
using namespace wago::wdx::file_transfer;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

// Template should be supported by every response
template <class Response>
void source_compatibility_and_default_construction()
{
    Response test_response;
    EXPECT_FALSE(test_response.is_determined());
    EXPECT_FALSE(test_response.has_error());
    EXPECT_EQ(status_codes::no_error_yet, test_response.status);
}

// Template should be supported by every response
template <class Response>
void source_compatibility_and_error_construction1()
{
    status_codes const test_status_code = status_codes::internal_error;

    Response test_response(test_status_code);
    EXPECT_TRUE(test_response.is_determined());
    EXPECT_TRUE(test_response.has_error());
    EXPECT_EQ(test_status_code, test_response.status);
}

// Template should be supported by every response
template <class Response>
void source_compatibility_and_error_construction2()
{
    status_codes const test_status_code = status_codes::internal_error;

    Response test_response;
    test_response.status = test_status_code;
    EXPECT_TRUE(test_response.is_determined());
    EXPECT_TRUE(test_response.has_error());
    EXPECT_EQ(test_status_code, test_response.status);
}

template <class Response>
void source_compatibility_and_error_construction3()
{
    status_codes const test_status_code = status_codes::internal_error;
    std::string  const test_message     = "Test message";

    Response test_response(test_status_code, test_message);
    EXPECT_TRUE(test_response.is_determined());
    EXPECT_TRUE(test_response.has_error());
    EXPECT_EQ(test_status_code, test_response.status);
    EXPECT_STREQ(test_message.c_str(), test_response.get_message().c_str());
}

// TODO: Should every response support error_construction4?
template <class Response>
void source_compatibility_and_error_construction4()
{
    status_codes const test_status_code = status_codes::internal_error;
    std::string  const test_message     = "Test message";

    Response test_response;
    test_response.set_error(test_status_code, test_message);
    EXPECT_TRUE(test_response.is_determined());
    EXPECT_TRUE(test_response.has_error());
    EXPECT_EQ(test_status_code, test_response.status);
    EXPECT_STREQ(test_message.c_str(), test_response.get_message().c_str());
}

// Template should be supported by every response given from parameter_provider_i
template <class Response>
void source_compatibility_and_domain_specific_code_construction1()
{
    uint16_t     const test_domain_code = UINT16_MAX;
    std::string  const test_message     = "Test domain message";

    Response test_response(test_domain_code, test_message);
    EXPECT_TRUE(test_response.is_determined());
    EXPECT_TRUE(test_response.has_error());
    EXPECT_TRUE(    (status_codes::parameter_value_unavailable == test_response.status)
                 || (status_codes::could_not_set_parameter     == test_response.status)
                 || (status_codes::could_not_invoke_method     == test_response.status));
    EXPECT_STREQ(test_message.c_str(), test_response.get_message().c_str());
}

// Template should be supported by every response given from parameter_provider_i
template <class Response>
void source_compatibility_and_domain_specific_code_construction2()
{
    uint16_t     const test_domain_code = UINT16_MAX;
    std::string  const test_message     = "Test domain message";

    Response test_response;
    test_response.set_domain_specific_error(test_domain_code, test_message);
    EXPECT_TRUE(test_response.is_determined());
    EXPECT_TRUE(test_response.has_error());
    EXPECT_TRUE(    (status_codes::parameter_value_unavailable == test_response.status)
                 || (status_codes::could_not_set_parameter     == test_response.status)
                 || (status_codes::could_not_invoke_method     == test_response.status));
    EXPECT_STREQ(test_message.c_str(), test_response.get_message().c_str());
}

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<response>();
}

TEST(response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<response>();
}

TEST(response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<response>();
}

TEST(response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<response>();
}

TEST(device_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<device_response>();
}

TEST(device_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<device_response>();
}

TEST(device_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<device_response>();
}

TEST(device_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<device_response>();
}

TEST(device_collection_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<device_collection_response>();
}

TEST(device_collection_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<device_collection_response>();
}

TEST(device_collection_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<device_collection_response>();
}

TEST(device_collection_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<device_collection_response>();
}

TEST(value_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<value_response>();
}

TEST(value_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<value_response>();
}

TEST(value_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<value_response>();
}

TEST(value_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<value_response>();
}

TEST(value_response, source_compatibility_and_error_construction4)
{
    source_compatibility_and_error_construction4<value_response>();
}

TEST(value_response, source_compatibility_and_domain_specific_code_construction1)
{
    source_compatibility_and_domain_specific_code_construction1<value_response>();
}

TEST(value_response, source_compatibility_and_domain_specific_code_construction2)
{
    source_compatibility_and_domain_specific_code_construction2<value_response>();
}

TEST(value_response, source_compatibility_and_unavailable_status_value)
{
    std::string const test_message = "Test message";

    value_response test_response;
    EXPECT_FALSE(test_response.is_determined());

    test_response.set_status_unavailable(test_message);
    EXPECT_TRUE( test_response.is_determined());

    // FIXME: Should this case be successful?
    //EXPECT_TRUE( test_response.is_success());
    EXPECT_FALSE(test_response.is_success());
    EXPECT_FALSE(test_response.has_error());
    EXPECT_EQ(test_message, test_response.get_message());
}

TEST(parameter_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<parameter_response>();
}

TEST(parameter_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<parameter_response>();
}

TEST(parameter_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<parameter_response>();
}

TEST(parameter_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<parameter_response>();
}

TEST(parameter_response_list_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<parameter_response_list_response>();
}

TEST(parameter_response_list_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<parameter_response_list_response>();
}

TEST(parameter_response_list_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<parameter_response_list_response>();
}

TEST(parameter_response_list_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<parameter_response_list_response>();
}

TEST(set_parameter_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<set_parameter_response>();
}

TEST(set_parameter_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<set_parameter_response>();
}

TEST(set_parameter_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<set_parameter_response>();
}

TEST(set_parameter_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<set_parameter_response>();
}

TEST(set_parameter_response, source_compatibility_and_error_construction4)
{
    source_compatibility_and_error_construction4<set_parameter_response>();
}

TEST(set_parameter_response, source_compatibility_and_domain_specific_code_construction1)
{
    source_compatibility_and_domain_specific_code_construction1<set_parameter_response>();
}

TEST(set_parameter_response, source_compatibility_and_domain_specific_code_construction2)
{
    source_compatibility_and_domain_specific_code_construction2<set_parameter_response>();
}

TEST(method_invocation_named_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<method_invocation_named_response>();
}

TEST(method_invocation_named_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<method_invocation_named_response>();
}

TEST(method_invocation_named_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<method_invocation_named_response>();
}

TEST(method_invocation_named_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<method_invocation_named_response>();
}

TEST(method_invocation_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<method_invocation_response>();
}

TEST(method_invocation_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<method_invocation_response>();
}

TEST(method_invocation_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<method_invocation_response>();
}

TEST(method_invocation_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<method_invocation_response>();
}

TEST(method_invocation_response, source_compatibility_and_error_construction4)
{
    source_compatibility_and_error_construction4<method_invocation_response>();
}

TEST(method_invocation_response, source_compatibility_and_domain_specific_code_construction1)
{
    source_compatibility_and_domain_specific_code_construction1<method_invocation_response>();
}

TEST(method_invocation_response, source_compatibility_and_domain_specific_code_construction2)
{
    source_compatibility_and_domain_specific_code_construction2<method_invocation_response>();
}

TEST(feature_list_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<feature_list_response>();
}

TEST(feature_list_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<feature_list_response>();
}

TEST(feature_list_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<feature_list_response>();
}

TEST(feature_list_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<feature_list_response>();
}

TEST(instance_list_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<instance_list_response>();
}

TEST(instance_list_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<instance_list_response>();
}

TEST(instance_list_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<instance_list_response>();
}

TEST(instance_list_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<instance_list_response>();
}

TEST(class_list_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<class_list_response>();
}

TEST(class_list_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<class_list_response>();
}

TEST(class_list_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<class_list_response>();
}

TEST(class_list_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<class_list_response>();
}

TEST(monitoring_list_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<monitoring_list_response>();
}

TEST(monitoring_list_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<monitoring_list_response>();
}

TEST(monitoring_list_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<monitoring_list_response>();
}

TEST(monitoring_list_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<monitoring_list_response>();
}

TEST(monitoring_lists_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<monitoring_lists_response>();
}

TEST(monitoring_lists_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<monitoring_lists_response>();
}

TEST(monitoring_lists_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<monitoring_lists_response>();
}

TEST(monitoring_lists_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<monitoring_lists_response>();
}

TEST(monitoring_list_values_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<monitoring_list_values_response>();
}

TEST(monitoring_list_values_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<monitoring_list_values_response>();
}

TEST(monitoring_list_values_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<monitoring_list_values_response>();
}

TEST(monitoring_list_values_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<monitoring_list_values_response>();
}

TEST(delete_monitoring_list_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<delete_monitoring_list_response>();
}

TEST(delete_monitoring_list_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<delete_monitoring_list_response>();
}

TEST(delete_monitoring_list_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<delete_monitoring_list_response>();
}

TEST(delete_monitoring_list_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<delete_monitoring_list_response>();
}

TEST(parameter_selector_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<parameter_selector_response>();
}

TEST(parameter_selector_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<parameter_selector_response>();
}

TEST(parameter_selector_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<parameter_selector_response>();
}

TEST(parameter_selector_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<parameter_selector_response>();
}

TEST(device_selector_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<device_selector_response>();
}

TEST(device_selector_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<device_selector_response>();
}

TEST(device_selector_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<device_selector_response>();
}

TEST(device_selector_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<device_selector_response>();
}

TEST(wdm_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<wdm_response>();
}

TEST(wdm_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<wdm_response>();
}

TEST(wdm_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<wdm_response>();
}

TEST(wdm_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<wdm_response>();
}

TEST(wdd_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<wdd_response>();
}

TEST(wdd_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<wdd_response>();
}

TEST(wdd_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<wdd_response>();
}

TEST(wdd_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<wdd_response>();
}

TEST(device_extension_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<device_extension_response>();
}

TEST(device_extension_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<device_extension_response>();
}

TEST(device_extension_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<device_extension_response>();
}

TEST(device_extension_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<device_extension_response>();
}

TEST(file_read_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<file_read_response>();
}

TEST(file_read_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<file_read_response>();
}

TEST(file_read_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<file_read_response>();
}

TEST(file_read_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<file_read_response>();
}

TEST(file_info_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<file_info_response>();
}

TEST(file_info_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<file_info_response>();
}

TEST(file_info_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<file_info_response>();
}

TEST(file_info_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<file_info_response>();
}

TEST(file_id_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<file_id_response>();
}

TEST(file_id_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<file_id_response>();
}

TEST(file_id_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<file_id_response>();
}

TEST(file_id_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<file_id_response>();
}

TEST(register_file_provider_response, source_compatibility_and_default_construction)
{
    source_compatibility_and_default_construction<register_file_provider_response>();
}

TEST(register_file_provider_response, source_compatibility_and_error_construction1)
{
    source_compatibility_and_error_construction1<register_file_provider_response>();
}

TEST(register_file_provider_response, source_compatibility_and_error_construction2)
{
    source_compatibility_and_error_construction2<register_file_provider_response>();
}

TEST(register_file_provider_response, source_compatibility_and_error_construction3)
{
    source_compatibility_and_error_construction3<register_file_provider_response>();
}


//---- End of source file ------------------------------------------------------
