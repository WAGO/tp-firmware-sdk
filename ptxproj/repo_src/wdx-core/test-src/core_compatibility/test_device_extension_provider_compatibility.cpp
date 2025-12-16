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
///  \brief    Test WAGO Parameter Service Core device extension provider compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "device_extension_provider_i.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;


namespace {

class test_device_extension_provider : public device_extension_provider_i
{
private:
    device_extension_response const response_m;

public:
    test_device_extension_provider(device_extension_response const &response)
    :response_m(response)
    {}
    ~test_device_extension_provider() override = default;
    device_extension_response get_device_extensions() override
    {
        return response_m;
    }
};

}

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(device_extension_provider_i, SourceCompatibilityAndConstructDelete)
{
    device_extension_response response(wago::wdx::device_selector::headstation(), {});
    test_device_extension_provider provider(response);
}

TEST(device_extension_provider_i, SourceCompatibilityAndGetExtensions)
{
    device_extension_response response(wago::wdx::device_selector::headstation(), { "TestFeature1", "TestFeature2" });
    test_device_extension_provider provider(response);
    device_extension_response current_response = provider.get_device_extensions();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
    ASSERT_EQ(2, current_response.extension_features.size());
}

TEST(device_extension_provider_i, SourceCompatibilityAndGetExtensionsError)
{
    wago::wdx::status_codes test_status_code = wago::wdx::status_codes::internal_error;
    device_extension_response response(test_status_code);
    test_device_extension_provider provider(response);
    device_extension_response current_response = provider.get_device_extensions();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_TRUE(current_response.has_error());
    ASSERT_EQ(test_status_code, current_response.status);
}


//---- End of source file ------------------------------------------------------
