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
///  \brief    Test example parameter provider.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "parameter_provider/example_parameter_provider.hpp"
#include <wago/wdx/test/provider_smoke_test.hpp>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::examples::example_parameter_provider;
using namespace wago::wdx;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(example_parameter_provider, construct_delete)
{
    {
        // Prepare provider
        example_parameter_provider provider;
    }
}

TEST(example_parameter_provider, smoke_test)
{
    {
        // Prepare provider
        example_parameter_provider provider;

        // Smoke test
        wago::wdx::test::parameter_provider_smoke_test(provider);
    }
}

TEST(example_parameter_provider, basic_checks)
{
    {
        // Prepare provider
        example_parameter_provider provider;

        // Check provided parameters
        auto const selector_response = provider.get_provided_parameters();
        EXPECT_TRUE(selector_response.is_determined());
        EXPECT_FALSE(selector_response.has_error());
        ASSERT_EQ(status_codes::success, selector_response.status);
        EXPECT_EQ(1U, selector_response.selected_parameters.size());
        ASSERT_LT(0U, selector_response.selected_parameters.size());
        EXPECT_NE(0U, selector_response.selected_parameters.at(0).parameter_id);
    }
}

TEST(example_parameter_provider, invalid_id)
{
    {
        // Prepare provider
        example_parameter_provider provider;

        // Prepare invalid ID
        auto const selector_response = provider.get_provided_parameters();
        auto const provided_id = selector_response.selected_parameters.at(0).parameter_id;
        parameter_instance_id const invalid_id((short unsigned)(provided_id + 1));

        // Send invalid request
        std::vector<parameter_instance_id> requests = { invalid_id };
        auto response_future = provider.get_parameter_values(requests);

        // Check response
        ASSERT_TRUE(response_future.valid());
        ASSERT_TRUE(response_future.ready());
        EXPECT_FALSE(response_future.has_exception());
        ASSERT_TRUE(response_future.has_value());
        auto const responses = response_future.get();
        ASSERT_EQ(requests.size(), responses.size());
        auto const response = responses.at(0);
        EXPECT_TRUE(response.is_determined());
        EXPECT_TRUE(response.has_error());
        ASSERT_EQ(status_codes::parameter_not_provided, response.status);
    }
}

TEST(example_parameter_provider, invalid_value)
{
    {
        // Prepare provider
        example_parameter_provider provider;

        // Get ID
        auto const selector_response = provider.get_provided_parameters();
        auto const provided_id = parameter_instance_id(selector_response.selected_parameters.at(0).parameter_id);

        // Send valid request
        std::vector<parameter_instance_id> requests = { provided_id };
        auto response_future = provider.get_parameter_values(requests);

        // Check response
        ASSERT_TRUE(response_future.valid());
        ASSERT_TRUE(response_future.ready());
        EXPECT_FALSE(response_future.has_exception());
        ASSERT_TRUE(response_future.has_value());
        auto const responses = response_future.get();
        ASSERT_EQ(requests.size(), responses.size());
        auto const response = responses.at(0);
        EXPECT_TRUE(response.is_determined());
        EXPECT_TRUE(response.has_error());
        ASSERT_EQ(status_codes::parameter_value_unavailable, response.status);
    }
}

TEST(example_parameter_provider, provided_value)
{
    {
        // Prepare provider
        example_parameter_provider provider;
        uint16_t const test_value = 815;
        provider.set_my_value(test_value);

        // Get ID
        auto const selector_response = provider.get_provided_parameters();
        auto const provided_id = parameter_instance_id(selector_response.selected_parameters.at(0).parameter_id);

        // Send valid request
        std::vector<parameter_instance_id> requests = { provided_id };
        auto response_future = provider.get_parameter_values(requests);

        // Check response
        ASSERT_TRUE(response_future.valid());
        ASSERT_TRUE(response_future.ready());
        EXPECT_FALSE(response_future.has_exception());
        ASSERT_TRUE(response_future.has_value());
        auto const responses = response_future.get();
        ASSERT_EQ(requests.size(), responses.size());
        auto const response = responses.at(0);
        EXPECT_TRUE(response.is_determined());
        EXPECT_FALSE(response.has_error());
        ASSERT_EQ(status_codes::success, response.status);
        ASSERT_EQ(test_value, response.value->get_uint16());
    }
}


//---- End of source file ------------------------------------------------------
