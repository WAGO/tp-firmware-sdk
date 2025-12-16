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
///  \brief    Test data container objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/monitoring_list_data.hpp"
#include "rest/basic_resource.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::rest;
using namespace wago::wdx;
using std::string;
using std::vector;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(monitoring_list_data, basic_values)
{
    monitoring_list_id_t const id      = UINT64_MAX;
    uint16_t             const timeout = UINT16_MAX;
    monitoring_list_data const test_data_object(id, timeout);
    char                 const known_resource_name[] = "parameters";

    EXPECT_EQ(id, test_data_object.get_monitoring_list_id());
    EXPECT_EQ(timeout, test_data_object.get_timeout());
    EXPECT_FALSE(test_data_object.has_related_data(known_resource_name));
    EXPECT_FALSE(test_data_object.has_related_resources(known_resource_name));
}

TEST(monitoring_list_data, related_data)
{
    monitoring_list_id_t    const id      = 38;
    uint16_t                const timeout = 72;
    monitoring_list_data          test_data_object(id, timeout);
    wago::wdx::parameter_response parameter_1;
    parameter_1.definition = std::make_shared<wdmm::parameter_definition>();
    wago::wdx::parameter_response parameter_2;
    parameter_2.definition = std::make_shared<wdmm::parameter_definition>();
    wago::wdx::parameter_response parameter_3;
    parameter_3.definition = std::make_shared<wdmm::parameter_definition>();
    parameter_3.status = wago::wdx::status_codes::internal_error;

    vector<wago::wdx::parameter_response> const parameters              = { parameter_1, parameter_2, parameter_3 };
    char                                  const known_resource_name[]   = "parameters";
    char                                  const invalid_resource_name[] = "some-trash";

    test_data_object.set_related_data(known_resource_name, parameters, false);
    EXPECT_EQ(id, test_data_object.get_monitoring_list_id());
    EXPECT_EQ(timeout, test_data_object.get_timeout());
    EXPECT_TRUE(test_data_object.has_related_data(known_resource_name));
    EXPECT_FALSE(test_data_object.has_related_data(invalid_resource_name));
    EXPECT_EQ(parameters.size(), test_data_object.get_related_data(known_resource_name).size());
    EXPECT_TRUE(test_data_object.has_related_resources(known_resource_name));
    EXPECT_FALSE(test_data_object.has_related_resources(invalid_resource_name));
    EXPECT_EQ(parameters.size() - 1, test_data_object.get_related_resources(known_resource_name).size());
}


//---- End of source file ------------------------------------------------------
