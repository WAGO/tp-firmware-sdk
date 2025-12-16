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
///  \brief    Test WAGO Parameter Service Core parameter provider compatibility.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/base_parameter_provider.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx;
using wago::future;


namespace {

class base_test_provider : public base_parameter_provider
{
public:
    base_test_provider()
    {}
    ~base_test_provider() override = default;
    parameter_selector_response get_provided_parameters() override
    {
        return parameter_selector_response({});
    }
};

class test_provider : public parameter_provider_i
{
private:
    std::string const name_m;
    std::shared_ptr<parameter_value> success_value_m;

public:
    test_provider(std::string const name)
    :name_m(name)
    {}
    test_provider(std::string const name, std::shared_ptr<parameter_value> success_value)
    :name_m(name), success_value_m(success_value)
    {}
    ~test_provider() override = default;
    std::string display_name() override
    {
        return name_m;
    }
    parameter_selector_response get_provided_parameters() override
    {
        return std::vector<parameter_selector>();
    }
    future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id>) override
    {
        wago::promise<std::vector<value_response>> promise;
        std::vector<value_response> responses = { value_response(),
                                                  value_response(wago::wdx::status_codes::internal_error),
                                                  value_response(success_value_m) };
        promise.set_value(std::move(responses));
        return promise.get_future();
    }
    future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> requests) override
    {
        return set_parameter_values_connection_aware(requests, false);
    }
    future<std::vector<set_parameter_response>> set_parameter_values_connection_aware(std::vector<value_request>, bool) override
    {
        wago::promise<std::vector<set_parameter_response>> promise;
        set_parameter_response response_1;
        response_1.status = wago::wdx::status_codes::internal_error;
        set_parameter_response response_2;
        response_2.status = wago::wdx::status_codes::success;
        std::vector<set_parameter_response> responses = { response_1,
                                                            response_2 };
        promise.set_value(std::move(responses));
        return promise.get_future();
    }
    future<method_invocation_response> invoke_method(parameter_instance_id, std::vector<std::shared_ptr<parameter_value>>) override
    {
        wago::promise<method_invocation_response> promise;
        std::vector<std::shared_ptr<parameter_value>> out_args = { success_value_m };
        method_invocation_response response(out_args);
        promise.set_value(std::move(response));
        return promise.get_future();
    }
    future<file_id_response> create_parameter_upload_id(parameter_id_t) override
    {
        wago::promise<file_id_response> promise;
        file_id_response response = file_id_response(success_value_m->get_file_id());
        promise.set_value(std::move(response));
        return promise.get_future();
    }
    future<response> remove_parameter_upload_id(file_id, parameter_id_t) override
    {
        return wago::resolved_future(response(wago::wdx::status_codes::success));
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
TEST(base_parameter_provider, SourceCompatibilityAndConstructDelete)
{
    base_test_provider provider;
}

TEST(base_parameter_provider, SourceCompatibilityAndBasicBehavior)
{
    base_test_provider provider;
    EXPECT_STRNE("", provider.display_name().c_str());
    parameter_selector_response const response = provider.get_provided_parameters();
    EXPECT_TRUE(response.is_determined());
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ(0, response.selected_parameters.size());
}

TEST(parameter_provider_i, SourceCompatibilityAndConstructDelete)
{
    test_provider provider("");
}

TEST(parameter_provider_i, SourceCompatibilityAndBasicBehavior)
{
    std::string const provider_name = "Test-Provider";
    test_provider provider(provider_name);
    EXPECT_STREQ(provider_name.c_str(), provider.display_name().c_str());
}

TEST(parameter_provider_i, SourceCompatibilityAndGetParameterValues)
{
    std::string const test_value("test-value");
    std::shared_ptr<parameter_value> success_value = parameter_value::create_string(test_value);

    std::vector<parameter_instance_id> parameter_ids = { parameter_instance_id(42), parameter_instance_id(53), parameter_instance_id(19) };
    ASSERT_EQ(3, parameter_ids.size());
    EXPECT_EQ(0, parameter_ids.at(0).instance_id);
    EXPECT_EQ(device_id::headstation, parameter_ids.at(0).device);
    EXPECT_EQ(42, parameter_ids.at(0).id);
    EXPECT_EQ(0, parameter_ids.at(1).instance_id);
    EXPECT_EQ(device_id::headstation, parameter_ids.at(1).device);
    EXPECT_EQ(53, parameter_ids.at(1).id);
    EXPECT_EQ(0, parameter_ids.at(2).instance_id);
    EXPECT_EQ(device_id::headstation, parameter_ids.at(2).device);
    EXPECT_EQ(19, parameter_ids.at(2).id);

    test_provider provider("", success_value);
    future<std::vector<value_response>> future = provider.get_parameter_values(parameter_ids);
    std::vector<value_response> responses = future.get();
    ASSERT_EQ(3, responses.size());
    EXPECT_FALSE(responses.at(0).is_determined());
    EXPECT_FALSE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::no_error_yet, responses.at(0).status);
    EXPECT_TRUE(responses.at(1).is_determined());
    EXPECT_TRUE(responses.at(1).has_error());
    EXPECT_EQ(wago::wdx::status_codes::internal_error, responses.at(1).status);
    EXPECT_TRUE(responses.at(2).is_determined());
    EXPECT_FALSE(responses.at(2).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(2).status);
    EXPECT_STREQ(test_value.c_str(), responses.at(2).value->get_string().c_str());
}

TEST(parameter_provider_i, SourceCompatibilityAndSetParameterValues)
{
    std::string const test_value_1("test-value-set1");
    std::string const test_value_2("test-value-set2");
    std::shared_ptr<parameter_value> set_value_1 = parameter_value::create_string(test_value_1);
    std::shared_ptr<parameter_value> set_value_2 = parameter_value::create_string(test_value_2);

    std::vector<value_request> value_requests = { { parameter_instance_id(71), set_value_1 }, { parameter_instance_id(13), set_value_2 } };
    ASSERT_EQ(2, value_requests.size());
    EXPECT_EQ(0, value_requests.at(0).param_id.instance_id);
    EXPECT_EQ(device_id::headstation, value_requests.at(0).param_id.device);
    EXPECT_EQ(71, value_requests.at(0).param_id.id);
    EXPECT_STREQ(test_value_1.c_str(), value_requests.at(0).value->get_string().c_str());
    EXPECT_EQ(0, value_requests.at(1).param_id.instance_id);
    EXPECT_EQ(device_id::headstation, value_requests.at(1).param_id.device);
    EXPECT_EQ(13, value_requests.at(1).param_id.id);
    EXPECT_STREQ(test_value_2.c_str(), value_requests.at(1).value->get_string().c_str());

    test_provider provider("");
    future<std::vector<set_parameter_response>> future = provider.set_parameter_values(value_requests);
    std::vector<set_parameter_response> responses = future.get();
    ASSERT_EQ(2, responses.size());
    EXPECT_TRUE(responses.at(0).is_determined());
    EXPECT_TRUE(responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::internal_error, responses.at(0).status);
    EXPECT_TRUE(responses.at(1).is_determined());
    EXPECT_FALSE(responses.at(1).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, responses.at(1).status);
}

TEST(parameter_provider_i, SourceCompatibilityAndInvokeMethod)
{
    std::string const test_value("test-value");
    std::shared_ptr<parameter_value> success_value = std::make_shared<parameter_value>(test_value);

    parameter_instance_id method_id(103);
    EXPECT_EQ(0, method_id.instance_id);
    EXPECT_EQ(device_id::headstation, method_id.device);
    EXPECT_EQ(103, method_id.id);

    std::string const test_value_1("test-value-invoke1");
    std::string const test_value_2("test-value-invoke2");
    std::shared_ptr<parameter_value> set_value_1 = parameter_value::create_string(test_value_1);
    std::shared_ptr<parameter_value> set_value_2 = parameter_value::create_string(test_value_2);
    std::vector<std::shared_ptr<parameter_value>> in_args = { set_value_1, set_value_2 };
    ASSERT_EQ(2, in_args.size());
    EXPECT_STREQ(test_value_1.c_str(), in_args.at(0)->get_string().c_str());
    EXPECT_STREQ(test_value_2.c_str(), in_args.at(1)->get_string().c_str());

    test_provider provider("", success_value);
    future<method_invocation_response> future = provider.invoke_method(method_id, in_args);
    method_invocation_response response = future.get();
    EXPECT_TRUE(response.is_determined());
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, response.status);
    ASSERT_EQ(1, response.out_args.size());
    EXPECT_STREQ(test_value.c_str(), response.out_args.at(0)->get_string().c_str());
}

TEST(parameter_provider_i, SourceCompatibilityAndCreateUploadId)
{
    std::string const test_value("test-value");
    std::shared_ptr<parameter_value> success_value = parameter_value::create_file_id(test_value);

    test_provider provider("", success_value);
    future<file_id_response> future = provider.create_parameter_upload_id(42);
    file_id_response response = future.get();
    EXPECT_TRUE(response.is_determined());
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, response.status);
    EXPECT_STREQ(test_value.c_str(), response.registered_file_id.c_str());
}

TEST(parameter_provider_i, SourceCompatibilityAndRemoveUploadId)
{
    std::string const test_value("test-value");

    test_provider provider("");
    future<response> future = provider.remove_parameter_upload_id(test_value, 53);
    response response = future.get();
    EXPECT_TRUE(response.is_determined());
    EXPECT_FALSE(response.has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, response.status);
}


//---- End of source file ------------------------------------------------------
