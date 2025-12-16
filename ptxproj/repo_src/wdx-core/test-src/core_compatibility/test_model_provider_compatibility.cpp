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
///  \brief    Test WAGO Parameter Service Core model provider compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "model_provider_i.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;


namespace {

class test_model_provider : public model_provider_i
{
private:
    wdm_response const response_m;

public:
    test_model_provider(wdm_response const &response)
    :response_m(response)
    {}
    ~test_model_provider() override = default;
    wago::future<wdm_response> get_model_information() override
    {
        wdm_response new_response = response_m;
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
TEST(model_provider_i, SourceCompatibilityAndConstructDelete)
{
    std::string test_data  = "My Model Extension";
    wdm_content_t wdm_data = test_data;
    wdm_response response(std::move(wdm_data));
    test_model_provider provider(response);
}

TEST(model_provider_i, SourceCompatibilityAndGetModel)
{
    char const * const test_data = "My next Model Extension";
    wdm_response response(std::move(std::string(test_data)));
    test_model_provider provider(response);
    wago::future<wdm_response> future = provider.get_model_information();
    wdm_response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_FALSE(current_response.has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, current_response.status);
    ASSERT_STREQ(test_data, current_response.wdm_content.c_str());
}

TEST(model_provider_i, SourceCompatibilityAndGetModelError)
{
    wago::wdx::status_codes test_status_code = wago::wdx::status_codes::internal_error;
    wdm_response response(test_status_code);
    test_model_provider provider(response);
    wago::future<wdm_response> future = provider.get_model_information();
    wdm_response current_response = future.get();
    EXPECT_TRUE(current_response.is_determined());
    EXPECT_TRUE(current_response.has_error());
    ASSERT_EQ(test_status_code, current_response.status);
}


//---- End of source file ------------------------------------------------------
