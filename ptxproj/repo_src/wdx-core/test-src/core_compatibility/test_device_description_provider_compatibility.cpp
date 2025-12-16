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
///  \brief    Test WAGO Parameter Service Core device description provider compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "device_description_provider_i.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;


namespace {

class test_device_description_provider : public device_description_provider_i
{
private:
    device_selector_response const selector_response_m;
    wdd_response const description_response_m;

public:
    test_device_description_provider(device_selector_response const &selector_response,
                                     wdd_response const &description_response)
    :selector_response_m(selector_response), description_response_m(description_response)
    {}
    ~test_device_description_provider() override = default;
    device_selector_response get_provided_devices() override
    {
        return selector_response_m;
    }
    wago::future<wdd_response> get_device_information(std::string, std::string) override
    {
        wdd_response new_response = description_response_m;
        return wago::resolved_future(std::move(new_response));
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
TEST(device_description_provider_i, SourceCompatibilityAndConstructDelete)
{
    device_selector_response selector_response({});
    wdd_response description_response = wdd_response::from_pure_wdd("My pure test wdd");
    test_device_description_provider provider(selector_response, description_response);
}

TEST(device_description_provider_i, SourceCompatibilityAndGetProvidedDevices)
{
    std::vector<wago::wdx::device_selector> selected_devices = {wago::wdx::device_selector::headstation(), wago::wdx::device_selector::any};
    device_selector_response selector_response(selected_devices);
    wdd_response description_response("My bundle test wdd");
    test_device_description_provider provider(selector_response, description_response);
    device_selector_response current_response = provider.get_provided_devices();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
    ASSERT_EQ(selected_devices.size(), current_response.selected_devices.size());
}

TEST(device_description_provider_i, SourceCompatibilityAndGetProvidedDevicesError)
{
    wago::wdx::status_codes test_status_code = wago::wdx::status_codes::internal_error;
    device_selector_response selector_response(test_status_code);
    wdd_response description_response("My next bundle test wdd");
    test_device_description_provider provider(selector_response, description_response);
    device_selector_response current_response = provider.get_provided_devices();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_TRUE(current_response.has_error());
    ASSERT_EQ(test_status_code, current_response.status);
}

TEST(device_description_provider_i, SourceCompatibilityAndGetDeviceInfo)
{
    device_selector_response selector_response({ wago::wdx::device_selector::any });
    char const * const test_data = "My bundle test to check";
    wdd_response description_response(test_data);
    test_device_description_provider provider(selector_response, description_response);
    wago::future<wdd_response> future = provider.get_device_information("some", "value");
    wdd_response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
    ASSERT_STREQ(test_data, current_response.content.c_str());
}

TEST(device_description_provider_i, SourceCompatibilityAndGetDeviceInfoError)
{
    device_selector_response selector_response({ wago::wdx::device_selector::any });
    wago::wdx::status_codes test_status_code = wago::wdx::status_codes::internal_error;
    wdd_response description_response(test_status_code);
    test_device_description_provider provider(selector_response, description_response);
    wago::future<wdd_response> future = provider.get_device_information("some", "value");
    wdd_response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_TRUE(current_response.has_error());
    ASSERT_EQ(test_status_code, current_response.status);
}


//---- End of source file ------------------------------------------------------
