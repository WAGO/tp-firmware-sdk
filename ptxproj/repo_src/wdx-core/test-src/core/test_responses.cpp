//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
///  \brief    Test response objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "wago/wdx/responses.hpp"

using namespace wago::wdx;


class responses_test_fixture: public testing::TestWithParam<std::shared_ptr<response>>{};

TEST_P(responses_test_fixture, default_constructor)
{
    auto response_object = GetParam();

    EXPECT_EQ("",                         response_object->message);
    EXPECT_EQ(status_codes::no_error_yet, response_object->status);

    EXPECT_FALSE(response_object->is_determined());
    EXPECT_FALSE(response_object->is_success());
    EXPECT_FALSE(response_object->has_error());
}

TEST_P(responses_test_fixture, response_operations)
{
    char const error_message[] = "Some error text";

    auto response_object = GetParam();

    response_object->set_success();
    EXPECT_TRUE( response_object->is_determined());
    EXPECT_TRUE( response_object->is_success());
    EXPECT_FALSE(response_object->has_error());

    response_object->set_error(status_codes::internal_error, error_message);
    EXPECT_TRUE( response_object->is_determined());
    EXPECT_FALSE(response_object->is_success());
    EXPECT_TRUE( response_object->has_error());
    EXPECT_EQ(   error_message, response_object->get_message());
}

INSTANTIATE_TEST_CASE_P(all_responses, responses_test_fixture, testing::Values(
    std::make_shared<response>(),
    std::make_shared<device_response>(),
    std::make_shared<device_collection_response>(),
    std::make_shared<value_response>(),
    std::make_shared<parameter_response>(),
    std::make_shared<parameter_response_list_response>(),
    std::make_shared<set_parameter_response>(),
    std::make_shared<method_invocation_named_response>(),
    std::make_shared<method_invocation_response>(),
    std::make_shared<feature_list_response>(),
    std::make_shared<feature_response>(),
    std::make_shared<instance_list_response>(),
    std::make_shared<class_list_response>(),
    std::make_shared<monitoring_list_response>(),
    std::make_shared<monitoring_lists_response>(),
    std::make_shared<monitoring_list_values_response>(),
    std::make_shared<delete_monitoring_list_response>(),
    std::make_shared<parameter_selector_response>(),
    std::make_shared<device_selector_response>(),
    std::make_shared<wdm_response>(),
    std::make_shared<wdd_response>(),
    std::make_shared<device_extension_response>(),
    std::make_shared<enum_definition_response>()
));


class response_with_domain_specific_status_code_test_fixture: public testing::TestWithParam<std::shared_ptr<response_with_domain_specific_status_code>>{};
static uint16_t const initial_domain_status_code = 53;
static char     const initial_error_message[]    = "Some text";

TEST_P(response_with_domain_specific_status_code_test_fixture, domain_status_code_operations)
{
    uint16_t const domain_status_code = 42;
    char     const error_message[]    = "Answer should be 42";

    auto response_object = GetParam();

    EXPECT_EQ(initial_domain_status_code, response_object->domain_specific_status_code);
    EXPECT_EQ(initial_error_message,      response_object->get_message());

    response_object->set_domain_specific_error(domain_status_code, error_message);
    EXPECT_EQ(domain_status_code, response_object->domain_specific_status_code);
    EXPECT_EQ(error_message,      response_object->get_message());
}

INSTANTIATE_TEST_CASE_P(domain_specific_status_code_responses, response_with_domain_specific_status_code_test_fixture, testing::Values(
    //std::make_shared<response_with_domain_specific_status_code>()
    std::make_shared<method_invocation_named_response>(initial_domain_status_code, initial_error_message),
    std::make_shared<method_invocation_response>(      initial_domain_status_code, initial_error_message),
    std::make_shared<value_response>(                  initial_domain_status_code, initial_error_message),
    std::make_shared<parameter_response>(              initial_domain_status_code, initial_error_message),
    std::make_shared<set_parameter_response>(          initial_domain_status_code, initial_error_message)
));


class response_with_value_test_fixture: public testing::TestWithParam<std::shared_ptr<response_with_value>>{};

TEST_P(response_with_value_test_fixture, parameter_value_operations)
{
    auto response_object = GetParam();

    EXPECT_EQ(nullptr, response_object->value.get());
}

INSTANTIATE_TEST_CASE_P(response_with_value_responses, response_with_value_test_fixture, testing::Values(
    std::make_shared<value_response>(),
    std::make_shared<parameter_response>(),
    std::make_shared<set_parameter_response>()
));
