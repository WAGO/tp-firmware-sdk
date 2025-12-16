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
///  \brief    Test WAGO Parameter Service frontend to support dismissal of 
///            returned futures.
///
///  \author   MaHE: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "parameter_service_core.hpp"
#include "model_provider_i.hpp"
#include "device_description_provider_i.hpp"
#include "wago/wdx/base_parameter_provider.hpp"
#include "mocks/mock_permissions.hpp"

#include <gtest/gtest.h>

#include <string>
#include <memory>
#include <sstream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;


namespace {

class test_model_provider : public model_provider_i
{
private:
    wago::wdx::wdm_content_t const model_m;

public:
    test_model_provider(wago::wdx::wdm_content_t const model)
    : model_m(model)
    {}
    ~test_model_provider() override = default;

    wago::future<wago::wdx::wdm_response> get_model_information() override
    {
        return wago::resolved_future(wago::wdx::wdm_response(std::move(wago::wdx::wdm_content_t(model_m))));
    }
};

class test_description_provider : public device_description_provider_i
{
private:
    wago::wdx::device_selector const selector_m;
    wago::wdx::wdd_content_t   const description_m;

public:
    test_description_provider(wago::wdx::device_selector const selector,
                              wago::wdx::wdd_content_t   const description)
    : selector_m(selector), description_m(description)
    {}
    ~test_description_provider() override = default;

    wago::wdx::device_selector_response get_provided_devices() override
    {
        return wago::wdx::device_selector_response({ selector_m });
    }
    wago::future<wago::wdx::wdd_response> get_device_information(std::string const, std::string const) override
    {
        return wago::resolved_future(wago::wdx::wdd_response::from_pure_wdd(std::move(wago::wdx::wdd_content_t(description_m))));
    }
};

class test_parameter_provider : public base_parameter_provider
{
private:
    wago::wdx::parameter_selector const selector_m;

public:
    std::shared_ptr<wago::promise<std::vector<wago::wdx::value_response>>>         last_get_parameter_values_promise_m = nullptr;
    std::shared_ptr<wago::promise<std::vector<wago::wdx::set_parameter_response>>> last_set_parameter_values_promise_m = nullptr;
    std::shared_ptr<wago::promise<method_invocation_response>>                     last_invoke_method_promise_m = nullptr;

public:
    test_parameter_provider(wago::wdx::parameter_selector const selector)
    : selector_m(selector)
    {}
    ~test_parameter_provider() override = default;

    wago::wdx::parameter_selector_response get_provided_parameters() override
    {
        return wago::wdx::parameter_selector_response({ selector_m });
    }
    wago::future<std::vector<wago::wdx::value_response>> get_parameter_values(std::vector<wago::wdx::parameter_instance_id>) override
    {
        last_get_parameter_values_promise_m = std::make_shared<wago::promise<std::vector<wago::wdx::value_response>>>();
        return last_get_parameter_values_promise_m->get_future();
    }
    wago::future<std::vector<wago::wdx::set_parameter_response>> set_parameter_values(std::vector<wago::wdx::value_request>) override
    {
        last_set_parameter_values_promise_m = std::make_shared<wago::promise<std::vector<wago::wdx::set_parameter_response>>>();
        return last_set_parameter_values_promise_m->get_future();
    }
    wago::future<wago::wdx::method_invocation_response> invoke_method(wago::wdx::parameter_instance_id, std::vector<std::shared_ptr<wago::wdx::parameter_value>>) override
    {
        last_invoke_method_promise_m = std::make_shared<wago::promise<method_invocation_response>>();
        return last_invoke_method_promise_m->get_future();
    }
};

auto constexpr default_model_content = R"(
    {
        "Name": "WAGO",
        "WDMMVersion": "1.0.0",
        "Features": [{
            "ID": "MyFeature",
            "Parameters": [{
                "ID": 42,
                "Path": "MyFeature/MyParameter",
                "Type": "Boolean",
                "Writeable": true
            }, {
                "ID": 43,
                "Path": "MyFeature/MyMethod",
                "Type": "Method"
            }]
        }]
    }
)";

auto constexpr default_device_description = R"(
    {
        "ModelReference": "WAGO",
        "WDMMVersion": "1.0.0",
        "Features": [
            "MyFeature"
        ]
    }
)";

register_device_request const default_device = { device_id::headstation, "0123-4567", "00.11.22" };

}

class parameter_service_dissmiss_fixture : public ::testing::Test
{
private:
    std::unique_ptr<mock_permissions> permissions_mock_ptr = std::make_unique<mock_permissions>();

protected:
    mock_permissions &permissions_mock = *permissions_mock_ptr;

    std::unique_ptr<wago::wdx::parameter_service_i> service_m = std::make_unique<parameter_service_core>(std::move(permissions_mock_ptr));

    test_model_provider       model_provider_m     = test_model_provider(default_model_content);
    test_description_provider device_provider_m    = test_description_provider(device_selector::any, default_device_description);
    test_parameter_provider   parameter_provider_m = test_parameter_provider(parameter_selector::all_of_feature("MyFeature"));

protected:
    parameter_service_dissmiss_fixture() = default;
    ~parameter_service_dissmiss_fixture() override = default;
    void SetUp() override
    {
        permissions_mock.set_default_expectations();
        service_m->register_device(default_device);
        service_m->register_model_provider(&model_provider_m);
        service_m->register_device_description_provider(&device_provider_m);
        service_m->register_parameter_provider(&parameter_provider_m);
    }

    template<typename FrontendFutureValueType, typename ProviderPromiseValueType>
    void testProviderDismissAfterFrontendDismiss(wago::future<FrontendFutureValueType> &frontend_future,
                                                 std::shared_ptr<wago::promise<ProviderPromiseValueType>> provider_promise)
    {
        ASSERT_TRUE(provider_promise != nullptr) << "Parameter provider has not been called";
        EXPECT_TRUE(provider_promise->execute()) << "Parameter provider owned promise is not set to be execute initially";
        frontend_future.dismiss();
        EXPECT_FALSE(provider_promise->execute()) << "Parameter provider owned promise is set to be execute despite frontend future beeing dismissed";
    }
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(parameter_service_dissmiss_fixture, DISABLED_get_all_parameters)
{
    auto frontend_future = service_m->get_all_parameters(parameter_filter::any);
    auto provider_promise = parameter_provider_m.last_get_parameter_values_promise_m;
    testProviderDismissAfterFrontendDismiss(frontend_future, provider_promise);
}

TEST_F(parameter_service_dissmiss_fixture, DISABLED_get_parameters)
{
    auto frontend_future = service_m->get_parameters({
        parameter_instance_id(42)
    });
    auto provider_promise = parameter_provider_m.last_get_parameter_values_promise_m;
    testProviderDismissAfterFrontendDismiss(frontend_future, provider_promise);
}

TEST_F(parameter_service_dissmiss_fixture, DISABLED_set_parameter_values)
{
    auto frontend_future = service_m->set_parameter_values({ 
        wago::wdx::value_request(parameter_instance_id(42), 
        wago::wdx::parameter_value::create_boolean(true)) 
    });
    auto provider_promise = parameter_provider_m.last_set_parameter_values_promise_m;
    testProviderDismissAfterFrontendDismiss(frontend_future, provider_promise);
}

TEST_F(parameter_service_dissmiss_fixture, DISABLED_invoke_method)
{
    auto frontend_future = service_m->invoke_method(
        parameter_instance_id(43),
        { /* no method args */ }
    );
    auto provider_promise = parameter_provider_m.last_invoke_method_promise_m;
    testProviderDismissAfterFrontendDismiss(frontend_future, provider_promise);
}

//---- End of source file ------------------------------------------------------
