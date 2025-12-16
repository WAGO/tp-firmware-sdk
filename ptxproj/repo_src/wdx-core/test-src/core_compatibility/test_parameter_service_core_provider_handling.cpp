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
///  \brief    Test WAGO Parameter Service Core provider handling.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "test_providers.hpp"

#include "parameter_service_core.hpp"
#include <wago/wdx/test/mock_parameter_provider.hpp>
#include "mocks/mock_model_provider.hpp"
#include "mocks/mock_device_description_provider.hpp"
#include "mocks/mock_device_extension_provider.hpp"
#include "mocks/mock_permissions.hpp"

#include <wago/wdx/test/wda_check.hpp>
#include <gtest/gtest.h>

#include <string>
#include <memory>
#include <sstream>
#include <thread>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::test;

namespace {
//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
wago::wdx::register_device_request          const default_device               = { wago::wdx::device_id(), "0000-0000", "00.00.00" };
std::string                                 const default_device_path          = "0-0";
std::string                                 const default_feature_id_1         = "MyTestFeature1";
std::string                                 const default_feature_id_2         = "MyTestFeature2";
wago::wdx::parameter_id_t                   const status_parameter_id_1        = 1;
wago::wdx::parameter_id_t                   const default_parameter_id_1       = 39;
wago::wdx::parameter_id_t                   const default_parameter_id_2       = 323;
wago::wdx::parameter_id_t                   const default_parameter_id_3       = 69;
wago::wdx::parameter_id_t                   const default_method_id_1          = 25;
std::string                                 const status_parameter_path_1      = "Some/Test/StatusPath1";
std::string                                 const default_parameter_path_1     = "Some/Test/Path1";
std::string                                 const default_parameter_path_2     = "Some/Test/Path2";
std::string                                 const default_parameter_path_3     = "Some/Test/Path3";
std::string                                 const default_method_path_1        = "Some/Test/Method1";
std::shared_ptr<wago::wdx::parameter_value> const default_string_value         = wago::wdx::parameter_value::create_string("test value");
std::shared_ptr<wago::wdx::parameter_value> const default_file_id_value        = wago::wdx::parameter_value::create_file_id("filetestid");
wago::wdx::device_extension_response        const default_extension_response_1 = { wago::wdx::device_selector::specific(default_device.device_id),
                                                                                   { default_feature_id_1 } };
wago::wdx::device_extension_response        const default_extension_response_2 = { wago::wdx::device_selector::specific(default_device.device_id),
                                                                                   { default_feature_id_2 } };
wago::wdx::wdm_content_t                    const default_test_model_1         = \
    "{" \
       "\"WDMMVersion\": \"1.0.0\"," \
       "\"Name\": \"WAGO\"," \
       "\"Features\": " \
       "[" \
           "{" \
                "\"ID\": \"" + default_feature_id_1 + "\"," \
                "\"Parameters\":" \
                "[" \
                    "{" \
                        "\"ID\": " + std::to_string(status_parameter_id_1) + "," \
                        "\"Path\": \"" + status_parameter_path_1 + "\"," \
                        //"\"Type\": \"" + wago::wdx::wdmm::to_string(default_string_value->get_type()) + "\""
                        "\"Type\": \"" + "String" + "\"," \
                        "\"Writeable\": false" \
                    "}," \
                    "{" \
                        "\"ID\": " + std::to_string(default_parameter_id_1) + "," \
                        "\"Path\": \"" + default_parameter_path_1 + "\"," \
                        //"\"Type\": \"" + wago::wdx::wdmm::to_string(default_string_value->get_type()) + "\""
                        "\"Type\": \"" + "String" + "\"," \
                        "\"Writeable\": true," \
                        "\"OnlyOnline\": true" \
                    "}," \
                    "{" \
                        "\"ID\": " + std::to_string(default_parameter_id_2) + "," \
                        "\"Path\": \"" + default_parameter_path_2 + "\"," \
                        //"\"Type\": \"" + wago::wdx::wdmm::to_string(default_string_value->get_type()) + "\""
                        "\"Type\": \"" + "String" + "\"," \
                        "\"Writeable\": true," \
                        "\"UserSetting\": true" \
                    "}" \
                "]" \
            "}" \
        "]" \
    "}";
wago::wdx::wdm_content_t                    const default_test_model_2         = \
    "{" \
       "\"WDMMVersion\": \"1.0.0\"," \
       "\"Name\": \"WAGO\"," \
       "\"Features\": " \
       "[" \
           "{" \
                "\"ID\": \"" + default_feature_id_2 + "\"," \
                "\"Parameters\":" \
                "[" \
                    "{" \
                        "\"ID\": " + std::to_string(default_parameter_id_3) + "," \
                        "\"Path\": \"" + default_parameter_path_3 + "\"," \
                        //"\"Type\": \"" + wago::wdx::wdmm::to_string(default_file_id_value->get_type()) + "\""
                        "\"Type\": \"" + "FileID" + "\"," \
                        "\"Writeable\": true," \
                        "\"UserSetting\": true" \
                    "}," \
                    "{" \
                        "\"ID\": " + std::to_string(default_method_id_1) + "," \
                        "\"Path\": \"" + default_method_path_1 + "\"," \
                        "\"Type\": \"Method\"," \
                        "\"InArgs\": []," \
                        "\"OutArgs\": []," \
                        "\"StatusCodes\": []" \
                    "}" \
                "]" \
            "}" \
        "]" \
    "}";
std::string                                 const default_test_description     = \
    "{" \
        "\"WDMMVersion\": \"1.0.0\"," \
        "\"ModelReference\": \"WAGO\"," \
        "\"Features\":" \
        "[" \
            "\"" + default_feature_id_1 + "\"," \
            "\"" + default_feature_id_2 + "\"" \
        "]" \
    "}";


//------------------------------------------------------------------------------
// test helper implementation
//------------------------------------------------------------------------------
void register_devices(wago::wdx::parameter_service_i       * const core,
                      void                           const * const devices)
{
    auto devices_typed = (std::vector<wago::wdx::register_device_request> const *)devices;
    auto future_responses = core->register_devices(*devices_typed);
    ASSERT_FUTURE_VALUE(future_responses);
    auto register_device_responses = future_responses.get();
    ASSERT_EQ(devices_typed->size(), register_device_responses.size());
    EXPECT_TRUE(register_device_responses.at(0).is_determined());
    EXPECT_FALSE(register_device_responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, register_device_responses.at(0).status);
}

void register_parameter_providers(wago::wdx::parameter_service_i       * const core,
                                  void                           const * const providers)
{
    auto providers_typed = (std::vector<wago::wdx::parameter_provider_i*> const *)providers;
    auto future_responses = core->register_parameter_providers(*providers_typed);
    ASSERT_FUTURE_VALUE(future_responses);
    auto parameter_provider_responses = future_responses.get();
    ASSERT_EQ(providers_typed->size(), parameter_provider_responses.size());
    EXPECT_TRUE(parameter_provider_responses.at(0).is_determined());
    EXPECT_FALSE(parameter_provider_responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, parameter_provider_responses.at(0).status);
}

void register_description_providers(wago::wdx::parameter_service_i       * const core,
                                    void                           const * const providers)
{
    auto providers_typed = (std::vector<wago::wdx::device_description_provider_i*> const *)providers;
    auto future_responses = core->register_device_description_providers(*providers_typed);
    ASSERT_FUTURE_VALUE(future_responses);
    auto description_provider_responses = future_responses.get();
    ASSERT_EQ(providers_typed->size(), description_provider_responses.size());
    EXPECT_TRUE(description_provider_responses.at(0).is_determined());
    EXPECT_FALSE(description_provider_responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, description_provider_responses.at(0).status);
}

void register_extension_providers(wago::wdx::parameter_service_i       * const core,
                                  void                           const * const providers)
{
    auto providers_typed = (std::vector<wago::wdx::device_extension_provider_i*> const *)providers;
    auto future_responses = core->register_device_extension_providers(*providers_typed);
    ASSERT_FUTURE_VALUE(future_responses);
    auto description_provider_responses = future_responses.get();
    ASSERT_EQ(providers_typed->size(), description_provider_responses.size());
    EXPECT_TRUE(description_provider_responses.at(0).is_determined());
    EXPECT_FALSE(description_provider_responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, description_provider_responses.at(0).status);
}

void register_model_providers(wago::wdx::parameter_service_i       * const core,
                              void                           const * const providers)
{
    auto providers_typed = (std::vector<wago::wdx::model_provider_i*> const *)providers;
    auto future_responses = core->register_model_providers(*providers_typed);
    ASSERT_FUTURE_VALUE(future_responses);
    auto model_provider_responses = future_responses.get();
    ASSERT_EQ(providers_typed->size(), model_provider_responses.size());
    EXPECT_TRUE(model_provider_responses.at(0).is_determined());
    EXPECT_FALSE(model_provider_responses.at(0).has_error());
    EXPECT_EQ(wago::wdx::status_codes::success, model_provider_responses.at(0).status);
}

class sequence_container{
public:
    sequence_container(char const * type_name,
                       void const * function_data,
                       void (*function)(wago::wdx::parameter_service_i *, void const *))
    : type_name_m(type_name), function_data_m(function_data), function_m(function)
    {}
    char const * type_name_m;
    void const * function_data_m;
    void (*function_m)(wago::wdx::parameter_service_i *, void const *);

    friend bool operator<(const sequence_container &lhs, const sequence_container &rhs)
    {
        return std::string(lhs.type_name_m) < std::string(rhs.type_name_m);
    }
    static void swap(sequence_container &t, sequence_container &u)
    {
        sequence_container temp = t;
        t = u;
        u = temp;
    }
};
}


class core_fixture : public ::testing::Test
{
private:
    std::unique_ptr<mock_permissions> permissions_mock_ptr = std::make_unique<mock_permissions>();

protected:
    mock_permissions &permissions_mock = *permissions_mock_ptr;

    std::unique_ptr<wago::wdx::parameter_service_i> core_m = std::make_unique<wago::wdx::parameter_service_core>(std::move(permissions_mock_ptr));

protected:
    core_fixture() = default;
    ~core_fixture() override = default;
    void SetUp() override
    {
        permissions_mock.set_default_expectations();
    }
    void test_core_parameter_provider_write(wago::wdx::parameter_instance_path const &path_1,
                                            wago::wdx::parameter_instance_path const &path_2,
                                            wago::wdx::status_codes            const  result_1,
                                            wago::wdx::status_codes            const  result_2,
                                            bool                               const  multi_provider_setup);
    void test_core_parameter_provider_read(wago::wdx::parameter_instance_path     const &request,
                                           std::vector<wago::wdx::value_response> const &result_vector,
                                           wago::wdx::status_codes                const  result_status);
};


//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(core_fixture, OptimalProviderSequence)
{
    // Prepare providers
    test_parameter_provider parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Run optimal sequence
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_parameter_providers(  core_m.get(), &parameter_providers);
    register_devices(              core_m.get(), &devices);
    auto parameter_request_responses = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses.size());
    EXPECT_TRUE(parameter_request_responses.at(0).is_determined());
    EXPECT_FALSE(parameter_request_responses.at(0).has_error());
    ASSERT_EQ(wago::wdx::status_codes::success, parameter_request_responses.at(0).status);
    EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
}

TEST(core, ProviderRegisterSequence)
{
    // Prepare providers
    test_parameter_provider parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare vector for permutations
    std::vector<sequence_container> actions =
    {
        sequence_container("register model providers",       &model_providers,       register_model_providers),
        sequence_container("register description providers", &description_providers, register_description_providers),
        sequence_container("register parameter providers",   &parameter_providers,   register_parameter_providers),
        sequence_container("register devices",               &devices,               register_devices)
    };

    // Check permutations
    std::sort(actions.begin(),  actions.end());
    do
    {
        // Use new core for each sequence
        auto              permissions_mock_ptr = std::make_unique<mock_permissions>();
        mock_permissions &permissions_mock = *permissions_mock_ptr;
        auto              core = std::make_unique<wago::wdx::parameter_service_core>(std::move(permissions_mock_ptr));
        bool hard_error = false;
        std::string sequence = "";
        for(auto element : actions)
        {
            if(!sequence.empty())
            {
                sequence.append(", ");
            }
            sequence.append(element.type_name_m);
            std::string exception_message = "Unexpected exception on sequence <" + sequence + ">";
            try
            {
                element.function_m(core.get(), element.function_data_m);
            }
            catch(std::exception &e)
            {
                EXPECT_FALSE(true) << exception_message << ": " << e.what();
                hard_error = true;
                break;
            }
            catch(...)
            {
                EXPECT_FALSE(true) << exception_message;
                hard_error = true;
                break;
            }
        }
        if(hard_error)
        {
            continue;
        }
        auto parameter_request_responses = core->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
        ASSERT_EQ(1, parameter_request_responses.size()) << "Fail sequence: " << sequence;
        EXPECT_TRUE(parameter_request_responses.at(0).is_determined()) << "Fail sequence: " << sequence;
        EXPECT_FALSE(parameter_request_responses.at(0).has_error()) << "Fail sequence: " << sequence;
        
        if(!parameter_request_responses.at(0).has_error())
        {
            EXPECT_EQ(wago::wdx::status_codes::success, parameter_request_responses.at(0).status) << "Fail sequence: " << sequence;
        }
        if(parameter_request_responses.at(0).status == wago::wdx::status_codes::success)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str()) << "Fail sequence: " << sequence;
            if(default_string_value->get_string() == parameter_request_responses.at(0).value->get_string())
            {
                //std::cout << "Sequence OK: " << sequence << std::endl;
            }
        }
    }
    while(std::next_permutation(actions.begin(),  actions.end()));
}

void core_fixture::test_core_parameter_provider_write(wago::wdx::parameter_instance_path const &path_1,
                                                      wago::wdx::parameter_instance_path const &path_2,
                                                      wago::wdx::status_codes            const  result_1,
                                                      wago::wdx::status_codes            const  result_2,
                                                      bool                               const  multi_provider_setup)
{
    wago::wdx::value_path_request request_1(path_1, default_string_value);
    wago::wdx::value_path_request request_2(path_2, default_string_value);
    std::vector<wago::wdx::set_parameter_response> response_vector_1;
    std::vector<wago::wdx::set_parameter_response> response_vector_2;
    if(multi_provider_setup)
    {
        response_vector_1 = { wago::wdx::set_parameter_response(result_1) };
        response_vector_2 = { wago::wdx::set_parameter_response(result_2) };
    }
    else
    {
        response_vector_1 = { wago::wdx::set_parameter_response(result_1), wago::wdx::set_parameter_response(result_2) };
    }

    mock_model_provider            model_provider_mock;
    mock_device_extension_provider extension_provider_mock;
    mock_parameter_provider        provider_mock_1;
    mock_parameter_provider        provider_mock_2;

    model_provider_mock.set_default_expectations();
    EXPECT_CALL(model_provider_mock, get_model_information())
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(wago::wdx::wdm_response(default_test_model_1)))));

    extension_provider_mock.set_default_expectations();
    EXPECT_CALL(extension_provider_mock, get_device_extensions())
        .Times(1)
        .WillRepeatedly(testing::Return(default_extension_response_1));

    provider_mock_1.set_default_expectations();
    EXPECT_CALL(provider_mock_1, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    if(multi_provider_setup)
    {
        EXPECT_CALL(provider_mock_1, get_provided_parameters())
            .Times(1)
            .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1) })));
    }
    else
    {
        EXPECT_CALL(provider_mock_1, get_provided_parameters())
            .Times(1)
            .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1),
                                                                                     wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2)
                                                                                   })));
    }
    EXPECT_CALL(provider_mock_1, set_parameter_values_connection_aware(::testing::_, true))
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(std::move(response_vector_1)))));

    provider_mock_2.set_default_expectations();
    if(multi_provider_setup)
    {
        EXPECT_CALL(provider_mock_2, display_name())
            .Times(testing::AnyNumber())
            .WillRepeatedly(testing::Return("Provider test mock"));
        EXPECT_CALL(provider_mock_2, get_provided_parameters())
            .Times(1)
            .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2) })));
        EXPECT_CALL(provider_mock_2, set_parameter_values_connection_aware(::testing::_, true))
            .Times(1)
            .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(std::move(response_vector_2)))));
    }

    core_m->register_device(default_device);
    core_m->register_model_provider(&model_provider_mock);
    core_m->register_device_extension_provider(&extension_provider_mock);
    core_m->register_parameter_provider(&provider_mock_1);
    if(multi_provider_setup)
    {
        core_m->register_parameter_provider(&provider_mock_2);
    }
    auto future = core_m->set_parameter_values_by_path_connection_aware({ request_1, request_2 }, true);
    ASSERT_FUTURE_VALUE(future);
    auto responses = future.get();
    ASSERT_EQ(2, responses.size());
    EXPECT_EQ(   (result_1 != wago::wdx::status_codes::success)
              && (result_1 != wago::wdx::status_codes::wda_connection_changes_deferred), responses.at(0).has_error());
    EXPECT_EQ(   (result_2 != wago::wdx::status_codes::success)
              && (result_2 != wago::wdx::status_codes::wda_connection_changes_deferred), responses.at(1).has_error());
    if(result_1 == wago::wdx::status_codes::status_value_unavailable)
    {
        EXPECT_TRUE(responses.at(0).has_error());
    }
    if(result_2 == wago::wdx::status_codes::status_value_unavailable)
    {
        EXPECT_TRUE(responses.at(1).has_error());
    }
    // FIXME: New Core version with fix needed
//    auto expected_result_1 = (result_1 == wago::wdx::status_codes::status_value_unavailable) ? wago::wdx::status_codes::internal_error : result_1;
//    auto expected_result_2 = (result_2 == wago::wdx::status_codes::status_value_unavailable) ? wago::wdx::status_codes::internal_error : result_2;
//    EXPECT_WDA_STATUS(expected_result_1, responses.at(0).status);
//    EXPECT_WDA_STATUS(expected_result_2, responses.at(1).status);
}

TEST_F(core_fixture, WriteSuccessfulWithSingleParameterProvider)
{
    wago::wdx::parameter_instance_path path_1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path path_2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result_1 = wago::wdx::status_codes::success;
    wago::wdx::status_codes            result_2 = wago::wdx::status_codes::success;

    test_core_parameter_provider_write(path_1, path_2, result_1, result_2, false);
}

TEST_F(core_fixture, WriteSuccessfulWithTwoParameterProviders)
{
    wago::wdx::parameter_instance_path path_1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path path_2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result_1 = wago::wdx::status_codes::success;
    wago::wdx::status_codes            result_2 = wago::wdx::status_codes::success;

    test_core_parameter_provider_write(path_1, path_2, result_1, result_2, true);
}

TEST_F(core_fixture, WriteDeferredWithSingleParameterProvider)
{
    wago::wdx::parameter_instance_path path_1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path path_2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result_1 = wago::wdx::status_codes::wda_connection_changes_deferred;
    wago::wdx::status_codes            result_2 = wago::wdx::status_codes::wda_connection_changes_deferred;

    test_core_parameter_provider_write(path_1, path_2, result_1, result_2, false);
}

TEST_F(core_fixture, WriteDeferredWithTwoParameterProviders)
{
    wago::wdx::parameter_instance_path path_1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path path_2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result_1 = wago::wdx::status_codes::wda_connection_changes_deferred;
    wago::wdx::status_codes            result_2 = wago::wdx::status_codes::wda_connection_changes_deferred;

    test_core_parameter_provider_write(path_1, path_2, result_1, result_2, true);
}

TEST_F(core_fixture, WriteSuccessfulAndDeferredWithSingleParameterProvider)
{
    wago::wdx::parameter_instance_path path_1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path path_2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result_1 = wago::wdx::status_codes::success;
    wago::wdx::status_codes            result_2 = wago::wdx::status_codes::wda_connection_changes_deferred;

    test_core_parameter_provider_write(path_1, path_2, result_1, result_2, false);
}

TEST_F(core_fixture, WriteSuccessfulAndDeferredWithTwoParameterProviders)
{
    wago::wdx::parameter_instance_path path_1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path path_2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result_1 = wago::wdx::status_codes::success;
    wago::wdx::status_codes            result_2 = wago::wdx::status_codes::wda_connection_changes_deferred;

    test_core_parameter_provider_write(path_1, path_2, result_1, result_2, true);
}

TEST_F(core_fixture, WriteSetWithInvalidValueWithSingleParameterProvider)
{
    std::shared_ptr<wago::wdx::parameter_value> const invalid_value = wago::wdx::parameter_value::create_boolean(false);
    wago::wdx::value_path_request request_1(wago::wdx::parameter_instance_path(default_parameter_path_1, default_device_path), default_string_value);
    wago::wdx::value_path_request request_2(wago::wdx::parameter_instance_path(default_parameter_path_2, default_device_path), invalid_value);

    mock_model_provider            model_provider_mock;
    mock_device_extension_provider extension_provider_mock;
    mock_parameter_provider        provider_mock;

    model_provider_mock.set_default_expectations();
    EXPECT_CALL(model_provider_mock, get_model_information())
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(wago::wdx::wdm_response(default_test_model_1)))));

    extension_provider_mock.set_default_expectations();
    EXPECT_CALL(extension_provider_mock, get_device_extensions())
        .Times(1)
        .WillRepeatedly(testing::Return(default_extension_response_1));

    provider_mock.set_default_expectations();
    EXPECT_CALL(provider_mock, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    EXPECT_CALL(provider_mock, get_provided_parameters())
        .Times(1)
        .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1),
                                                                                 wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2)
                                                                               })));

    core_m->register_device(default_device);
    core_m->register_model_provider(&model_provider_mock);
    core_m->register_device_extension_provider(&extension_provider_mock);
    core_m->register_parameter_provider(&provider_mock);
    auto future = core_m->set_parameter_values_by_path_connection_aware({ request_1, request_2 }, true);
    ASSERT_FUTURE_VALUE(future);
    auto responses = future.get();
    ASSERT_EQ(2, responses.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::other_invalid_value_in_set, responses.at(0).status);
    EXPECT_WDA_STATUS(wago::wdx::status_codes::wrong_value_type,           responses.at(1).status);
}

TEST_F(core_fixture, WriteIndependendValuesWithInvalidValueWithTwoParameterProviders)
{
    std::shared_ptr<wago::wdx::parameter_value> const invalid_value = wago::wdx::parameter_value::create_boolean(false);
    wago::wdx::value_path_request request_1(wago::wdx::parameter_instance_path(default_parameter_path_1, default_device_path), default_string_value);
    wago::wdx::value_path_request request_2(wago::wdx::parameter_instance_path(default_parameter_path_2, default_device_path), invalid_value);
    wago::wdx::status_codes result = wago::wdx::status_codes::success;
    std::vector<wago::wdx::set_parameter_response> response_vector;
    response_vector = { wago::wdx::set_parameter_response(result) };

    mock_model_provider            model_provider_mock;
    mock_device_extension_provider extension_provider_mock;
    mock_parameter_provider        provider_mock_1;
    mock_parameter_provider        provider_mock_2;

    model_provider_mock.set_default_expectations();
    EXPECT_CALL(model_provider_mock, get_model_information())
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(wago::wdx::wdm_response(default_test_model_1)))));

    extension_provider_mock.set_default_expectations();
    EXPECT_CALL(extension_provider_mock, get_device_extensions())
        .Times(1)
        .WillRepeatedly(testing::Return(default_extension_response_1));

    provider_mock_1.set_default_expectations();
    EXPECT_CALL(provider_mock_1, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    EXPECT_CALL(provider_mock_1, get_provided_parameters())
        .Times(1)
        .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1) })));
    EXPECT_CALL(provider_mock_1, set_parameter_values_connection_aware(::testing::_, true))
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(std::move(response_vector)))));

    provider_mock_2.set_default_expectations();
    EXPECT_CALL(provider_mock_2, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    EXPECT_CALL(provider_mock_2, get_provided_parameters())
        .Times(1)
        .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2) })));

    core_m->register_device(default_device);
    core_m->register_model_provider(&model_provider_mock);
    core_m->register_device_extension_provider(&extension_provider_mock);
    core_m->register_parameter_provider(&provider_mock_1);
    core_m->register_parameter_provider(&provider_mock_2);
    auto future = core_m->set_parameter_values_by_path_connection_aware({ request_1, request_2 }, true);
    ASSERT_FUTURE_VALUE(future);
    auto responses = future.get();
    ASSERT_EQ(2, responses.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::success,          responses.at(0).status);
    EXPECT_WDA_STATUS(wago::wdx::status_codes::wrong_value_type, responses.at(1).status);
}

TEST_F(core_fixture, ReadValueFromUnknownFeatureWithSingleParameterProvider)
{
    wago::wdx::parameter_instance_path request(default_parameter_path_1, default_device_path);

    mock_model_provider            model_provider_mock;
    mock_parameter_provider        provider_mock;

    model_provider_mock.set_default_expectations();
    EXPECT_CALL(model_provider_mock, get_model_information())
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(wago::wdx::wdm_response(default_test_model_1)))));

    provider_mock.set_default_expectations();
    EXPECT_CALL(provider_mock, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    EXPECT_CALL(provider_mock, get_provided_parameters())
        .Times(1)
        .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1),
                                                                                 wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2)
                                                                               })));

    core_m->register_device(default_device);
    core_m->register_model_provider(&model_provider_mock);
    core_m->register_parameter_provider(&provider_mock);
    auto future = core_m->get_parameters_by_path({ request });
    ASSERT_FUTURE_VALUE(future);
    auto responses = future.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::unknown_parameter_path, responses.at(0).status);
}

void core_fixture::test_core_parameter_provider_read(wago::wdx::parameter_instance_path     const &request,
                                                     std::vector<wago::wdx::value_response> const &result_vector,
                                                     wago::wdx::status_codes                const  result_status)
{
    auto response_vector = result_vector;

    mock_model_provider            model_provider_mock;
    mock_device_extension_provider extension_provider_mock;
    mock_parameter_provider        provider_mock;

    model_provider_mock.set_default_expectations();
    EXPECT_CALL(model_provider_mock, get_model_information())
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(wago::wdx::wdm_response(default_test_model_1)))));

    extension_provider_mock.set_default_expectations();
    EXPECT_CALL(extension_provider_mock, get_device_extensions())
        .Times(1)
        .WillRepeatedly(testing::Return(default_extension_response_1));

    provider_mock.set_default_expectations();
    EXPECT_CALL(provider_mock, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    EXPECT_CALL(provider_mock, get_provided_parameters())
        .Times(1)
        .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(status_parameter_id_1),
                                                                                 wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1),
                                                                                 wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2)
                                                                               })));
    EXPECT_CALL(provider_mock, get_parameter_values(::testing::_))
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(std::move(response_vector)))));

    core_m->register_device(default_device);
    core_m->register_model_provider(&model_provider_mock);
    core_m->register_device_extension_provider(&extension_provider_mock);
    core_m->register_parameter_provider(&provider_mock);
    auto future = core_m->get_parameters_by_path({ request });
    ASSERT_FUTURE_VALUE(future);
    auto responses = future.get();
    ASSERT_EQ(1, responses.size());
    EXPECT_EQ(   (result_status != wago::wdx::status_codes::success)
              && (result_status != wago::wdx::status_codes::status_value_unavailable), responses.at(0).has_error());
    if(result_status == wago::wdx::status_codes::wda_connection_changes_deferred)
    {
        EXPECT_TRUE(responses.at(0).has_error());
    }
    EXPECT_WDA_STATUS(result_status, responses.at(0).status);
    if(responses.at(0).is_success() && (result_status == responses.at(0).status))
    {
        EXPECT_EQ(result_vector.at(0).value, responses.at(0).value);
    }
}

TEST_F(core_fixture, ReadParameter)
{
    wago::wdx::parameter_instance_path request(default_parameter_path_1, default_device_path);
    std::vector<wago::wdx::value_response> response_vector;
    response_vector = { wago::wdx::value_response(wago::wdx::parameter_value::create_string("Success!")) };

    test_core_parameter_provider_read(request, response_vector, wago::wdx::status_codes::success);
}

TEST_F(core_fixture, ReadUnavailableStatusParameter)
{
    wago::wdx::parameter_instance_path request(status_parameter_path_1, default_device_path);
    wago::wdx::status_codes result = wago::wdx::status_codes::status_value_unavailable;
    std::vector<wago::wdx::value_response> response_vector;
    response_vector = { wago::wdx::value_response(result) };

    test_core_parameter_provider_read(request, response_vector, wago::wdx::status_codes::status_value_unavailable);
}

TEST_F(core_fixture, ReadUnavailableOnlyOnlineParameter)
{
    wago::wdx::parameter_instance_path request(default_parameter_path_1, default_device_path);
    wago::wdx::status_codes result = wago::wdx::status_codes::status_value_unavailable;
    std::vector<wago::wdx::value_response> response_vector;
    response_vector = { wago::wdx::value_response(result) };

    test_core_parameter_provider_read(request, response_vector, wago::wdx::status_codes::status_value_unavailable);
}

TEST_F(core_fixture, ReadUnavailableUserSetting)
{
    wago::wdx::parameter_instance_path request(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes result = wago::wdx::status_codes::status_value_unavailable;
    std::vector<wago::wdx::value_response> response_vector;
    response_vector = { wago::wdx::value_response(result) };

    test_core_parameter_provider_read(request, response_vector, wago::wdx::status_codes::internal_error);
}

TEST_F(core_fixture, WriteUnavailableOnlyOnlineParameter)
{
    wago::wdx::parameter_instance_path request1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path request2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result1 = wago::wdx::status_codes::status_value_unavailable;
    wago::wdx::status_codes            result2 = wago::wdx::status_codes::success;

    test_core_parameter_provider_write(request1, request2, result1, result2, false);
}

TEST_F(core_fixture, WriteUnavailableUserSetting)
{
    wago::wdx::parameter_instance_path request1(default_parameter_path_1, default_device_path);
    wago::wdx::parameter_instance_path request2(default_parameter_path_2, default_device_path);
    wago::wdx::status_codes            result1 = wago::wdx::status_codes::success;
    wago::wdx::status_codes            result2 = wago::wdx::status_codes::status_value_unavailable;

    test_core_parameter_provider_write(request1, request2, result1, result2, false);
}

TEST_F(core_fixture, InvokeUnavailableMethod)
{
    wago::wdx::parameter_instance_path    request(default_method_path_1, default_device_path);
    wago::wdx::status_codes               result = wago::wdx::status_codes::status_value_unavailable;
    wago::wdx::method_invocation_response provider_response(result);

    mock_model_provider            model_provider_mock;
    mock_device_extension_provider extension_provider_mock;
    mock_parameter_provider        provider_mock;

    model_provider_mock.set_default_expectations();
    EXPECT_CALL(model_provider_mock, get_model_information())
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(wago::wdx::wdm_response(default_test_model_2)))));

    extension_provider_mock.set_default_expectations();
    EXPECT_CALL(extension_provider_mock, get_device_extensions())
        .Times(1)
        .WillRepeatedly(testing::Return(default_extension_response_2));

    provider_mock.set_default_expectations();
    EXPECT_CALL(provider_mock, display_name())
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Return("Provider test mock"));
    EXPECT_CALL(provider_mock, get_provided_parameters())
        .Times(1)
        .WillRepeatedly(testing::Return(wago::wdx::parameter_selector_response({ wago::wdx::parameter_selector::all_with_definition(default_method_id_1) })));
    EXPECT_CALL(provider_mock, invoke_method(::testing::_, ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future(std::move(provider_response)))));

    core_m->register_device(default_device);
    core_m->register_model_provider(&model_provider_mock);
    core_m->register_device_extension_provider(&extension_provider_mock);
    core_m->register_parameter_provider(&provider_mock);
    auto future = core_m->invoke_method_by_path(request, {});
    ASSERT_FUTURE_VALUE(future);
    auto response = future.get();
    EXPECT_TRUE(response.has_error());
    // FIXME: New Core version with fix needed
//    EXPECT_WDA_STATUS(wago::wdx::status_codes::internal_error, response.status);
}

class test_parameter_provider_read_base : public test_parameter_provider
{
public:
    using test_parameter_provider::test_parameter_provider;

    wago::future<std::vector<wago::wdx::value_response>> get_parameter_values(std::vector<wago::wdx::parameter_instance_id> parameter_ids) override
    {
        return wago::wdx::base_parameter_provider::get_parameter_values(parameter_ids);
    }
};

TEST_F(core_fixture, HandleMissingReadImplementation)
{
    // Prepare providers
    test_extension_provider extension_provider(wago::wdx::device_selector::any, default_feature_id_1);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers = { &extension_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };
    test_parameter_provider_read_base parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };

    // Prepare model & incomplete parameter provider
    register_model_providers(    core_m.get(), &model_providers);
    register_devices(            core_m.get(), &devices);
    register_extension_providers(core_m.get(), &extension_providers);
    register_parameter_providers(core_m.get(), &parameter_providers);

    auto parameter_request_responses = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses.size());
    EXPECT_FALSE(parameter_request_responses.at(0).is_success());
    EXPECT_TRUE (parameter_request_responses.at(0).has_error());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses.at(0).status);
}

class test_parameter_provider_write_base : public test_parameter_provider
{
public:
    using test_parameter_provider::test_parameter_provider;

    wago::future<std::vector<wago::wdx::set_parameter_response>> set_parameter_values(std::vector<wago::wdx::value_request> value_requests) override
    {
        return wago::wdx::base_parameter_provider::set_parameter_values(value_requests);
    }
};

TEST_F(core_fixture, HandleMissingWriteImplementation)
{
    // Prepare providers
    test_extension_provider extension_provider(wago::wdx::device_selector::any, default_feature_id_1);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers = { &extension_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };
    test_parameter_provider_write_base parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };

    // Prepare model & incomplete parameter provider
    register_model_providers(    core_m.get(), &model_providers);
    register_devices(            core_m.get(), &devices);
    register_extension_providers(core_m.get(), &extension_providers);
    register_parameter_providers(core_m.get(), &parameter_providers);

    auto parameter_request_responses = core_m->set_parameter_values({ wago::wdx::value_request(wago::wdx::parameter_instance_id(default_parameter_id_1), default_string_value) }).get();
    ASSERT_EQ(1, parameter_request_responses.size());
    EXPECT_FALSE(parameter_request_responses.at(0).is_success());
    EXPECT_TRUE (parameter_request_responses.at(0).has_error());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses.at(0).status);
}

class test_parameter_provider_invoke_base : public test_parameter_provider
{
public:
    using test_parameter_provider::test_parameter_provider;

    wago::future<wago::wdx::method_invocation_response> invoke_method(wago::wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wago::wdx::parameter_value>> in_args) override
    {
        return wago::wdx::base_parameter_provider::invoke_method(method_id, in_args);
    }
};

TEST_F(core_fixture, HandleMissingInvokeImplementation)
{
    // Prepare providers
    test_extension_provider extension_provider(wago::wdx::device_selector::any, default_feature_id_2);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers = { &extension_provider };
    test_model_provider model_provider(default_test_model_2);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };
    test_parameter_provider_invoke_base parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_method_id_1), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };

    // Prepare model & incomplete parameter provider
    register_model_providers(    core_m.get(), &model_providers);
    register_devices(            core_m.get(), &devices);
    register_extension_providers(core_m.get(), &extension_providers);
    register_parameter_providers(core_m.get(), &parameter_providers);

    auto parameter_request_response = core_m->invoke_method(wago::wdx::parameter_instance_id(default_method_id_1), {}).get();
    EXPECT_FALSE(parameter_request_response.is_success());
    EXPECT_TRUE (parameter_request_response.has_error());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_response.status);
}

class test_parameter_provider_create_upload_base : public test_parameter_provider
{
public:
    using test_parameter_provider::test_parameter_provider;

    wago::future<wago::wdx::file_id_response> create_parameter_upload_id(wago::wdx::parameter_id_t context) override
    {
        return wago::wdx::base_parameter_provider::create_parameter_upload_id(context);
    }
};

TEST_F(core_fixture, HandleMissingCreateUploadImplementation)
{
    // Prepare providers
    test_extension_provider extension_provider(wago::wdx::device_selector::any, default_feature_id_2);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers = { &extension_provider };
    test_model_provider model_provider(default_test_model_2);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };
    test_parameter_provider_create_upload_base parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_3), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };

    // Prepare model & incomplete parameter provider
    register_model_providers(    core_m.get(), &model_providers);
    register_devices(            core_m.get(), &devices);
    register_extension_providers(core_m.get(), &extension_providers);
    register_parameter_providers(core_m.get(), &parameter_providers);

    auto parameter_request_response = core_m->create_parameter_upload_id(default_parameter_path_3, 123).get();
    EXPECT_FALSE(parameter_request_response.is_success());
    EXPECT_TRUE (parameter_request_response.has_error());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_response.status);
}

class test_parameter_provider_remove_upload_base : public test_parameter_provider
{
public:
    bool        removed_id  = false;
    std::string new_file_id = "filexy54";
    using test_parameter_provider::test_parameter_provider;

    wago::future<wago::wdx::file_id_response> create_parameter_upload_id(wago::wdx::parameter_id_t context) override
    {
        EXPECT_EQ(default_parameter_id_3, context);
        return wago::resolved_future(wago::wdx::file_id_response(new_file_id));
    }
    wago::future<wago::wdx::response> remove_parameter_upload_id(wago::wdx::file_id id, wago::wdx::parameter_id_t context) override
    {
        EXPECT_EQ(new_file_id, id);
        removed_id = true;
        auto remove_response = wago::wdx::base_parameter_provider::remove_parameter_upload_id(id, context).get();
        EXPECT_FALSE(remove_response.is_success());
        EXPECT_FALSE(remove_response.is_determined());
        EXPECT_WDA_STATUS(wago::wdx::status_codes::no_error_yet, remove_response.status);
        return wago::resolved_future(std::move(remove_response));
    }
};

TEST_F(core_fixture, HandleMissingRemoveUploadImplementation)
{
    // Prepare providers
    test_extension_provider extension_provider(wago::wdx::device_selector::any, default_feature_id_2);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers = { &extension_provider };
    test_model_provider model_provider(default_test_model_2);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };
    test_parameter_provider_remove_upload_base parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_3), default_string_value);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };

    // Prepare model & incomplete parameter provider
    register_model_providers(    core_m.get(), &model_providers);
    register_devices(            core_m.get(), &devices);
    register_extension_providers(core_m.get(), &extension_providers);
    register_parameter_providers(core_m.get(), &parameter_providers);

    auto parameter_request_response = core_m->create_parameter_upload_id(default_parameter_path_3, 1).get();
    EXPECT_WDA_SUCCESS(parameter_request_response);

    // Wait for timeout
    sleep(2);
    core_m->trigger_lapse_checks();

    EXPECT_TRUE(parameter_provider.removed_id);
}

TEST_F(core_fixture, MultiThreadingSimpleProviderCall)
{
    // Prepare providers
    test_parameter_provider parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value, false);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_complete;

    // Register in optimal sequence
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_parameter_providers(  core_m.get(), &parameter_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread for request on provider
    std::thread request_thread([this, &request_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) });
        request_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_WDA_SUCCESS(parameter_request_responses.at(0));
        if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
        }
    });

    // Unlock response
    request_complete.wait();
    parameter_provider.set_response();

    request_thread.join();

    EXPECT_FALSE(parameter_provider.has_error()) << parameter_provider.get_error_message();
}

TEST_F(core_fixture, MultiThreadingParallelFrontendCallsToSameProvider)
{
    // Prepare providers
    test_parameter_provider parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value, false);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register in optimal sequence
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_parameter_providers(  core_m.get(), &parameter_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread for request 1
    std::thread request_1_thread([this, &request_1_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) });
        request_1_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_TRUE(parameter_request_responses.at(0).is_determined());
        EXPECT_FALSE(parameter_request_responses.at(0).has_error());
        EXPECT_WDA_STATUS(wago::wdx::status_codes::success, parameter_request_responses.at(0).status);
        if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
        }
    });

    // Start thread for request 2
    std::thread request_2_thread([this, &request_2_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) });
        request_2_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_TRUE(parameter_request_responses.at(0).is_determined());
        EXPECT_FALSE(parameter_request_responses.at(0).has_error());
        EXPECT_WDA_STATUS(wago::wdx::status_codes::success, parameter_request_responses.at(0).status);
        if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
        }
    });

    // Unlock responses
    request_1_complete.wait();
    parameter_provider.set_response();
    request_2_complete.wait();
    parameter_provider.set_response();

    request_1_thread.join();
    request_2_thread.join();

    EXPECT_FALSE(parameter_provider.has_error()) << parameter_provider.get_error_message();
}

TEST_F(core_fixture, MultiThreadingParallelFrontendCallsToDifferentProviders)
{
    // Prepare providers
    test_parameter_provider parameter_provider_1(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value, false);
    test_parameter_provider parameter_provider_2(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2), default_string_value, false);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider_1, &parameter_provider_2 };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register in optimal sequence
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_parameter_providers(  core_m.get(), &parameter_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread for request on provider 1
    std::thread request_1_thread([this, &request_1_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) });
        request_1_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_WDA_SUCCESS(parameter_request_responses.at(0));
        if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
        }
    });

    // Start thread for request on provider 2
    std::thread request_2_thread([this, &request_2_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_2) });
        request_2_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_WDA_SUCCESS(parameter_request_responses.at(0));
        if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
        }
    });

    // Unlock responses in reverse order
    request_2_complete.wait();
    parameter_provider_2.set_response();
    request_1_complete.wait();
    parameter_provider_1.set_response();

    request_2_thread.join();
    request_1_thread.join();

    EXPECT_FALSE(parameter_provider_1.has_error()) << parameter_provider_1.get_error_message();
    EXPECT_FALSE(parameter_provider_2.has_error()) << parameter_provider_2.get_error_message();
}

TEST_F(core_fixture, MultiThreadingProviderUnregisterWhileFrontendCallToThisProviderIsActive)
{
    // Prepare providers
    test_parameter_provider parameter_provider(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value, false);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;

    // Register in optimal sequence
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_parameter_providers(  core_m.get(), &parameter_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread for request
    std::thread request_1_thread([this, &request_1_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) });
        request_1_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_TRUE(parameter_request_responses.at(0).is_determined());
        if(parameter_request_responses.at(0).has_error())
        {
            EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses.at(0).status);
        }
        else
        {
            EXPECT_WDA_STATUS(wago::wdx::status_codes::success, parameter_request_responses.at(0).status);
            if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
            {
                EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
            }
        }
    });

    // Unregister provider
    core_m->unregister_parameter_providers( { &parameter_provider } );

    // Unlock responses
    request_1_complete.wait();
    parameter_provider.set_response();

    request_1_thread.join();

    EXPECT_FALSE(parameter_provider.has_error()) << parameter_provider.get_error_message();
}

TEST_F(core_fixture, MultiThreadingUnregisterAnotherProviderWhileFrontendCallToAProviderIsActive)
{
    // Prepare providers
    test_parameter_provider parameter_provider_1(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value, false);
    test_parameter_provider parameter_provider_2(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2), default_string_value, false);
    std::vector<wago::wdx::parameter_provider_i*> const parameter_providers = { &parameter_provider_1, &parameter_provider_2 };
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register in optimal sequence
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_parameter_providers(  core_m.get(), &parameter_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread for request on provider 1
    std::thread request_1_thread([this, &request_1_complete](){
        auto future = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) });
        request_1_complete.notify();
        auto parameter_request_responses = future.get();
        ASSERT_EQ(1, parameter_request_responses.size());
        EXPECT_WDA_SUCCESS(parameter_request_responses.at(0));
        if(wago::wdx::status_codes::success == parameter_request_responses.at(0).status)
        {
            EXPECT_STREQ(default_string_value->get_string().c_str(), parameter_request_responses.at(0).value->get_string().c_str());
        }
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to unregister provider 2
    std::thread request_2_thread([this, &request_2_complete, &parameter_provider_2](){
        core_m->unregister_parameter_providers( { &parameter_provider_2 } );
        request_2_complete.notify();
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    // Unlock response on first provider
    parameter_provider_1.set_response();

    request_2_thread.join();
    request_1_thread.join();

    EXPECT_FALSE(parameter_provider_1.has_error()) << parameter_provider_1.get_error_message();
    EXPECT_FALSE(parameter_provider_2.has_error()) << parameter_provider_2.get_error_message();
}

TEST_F(core_fixture, MultiThreadingProviderRegistrationsAndUnregistrationsInParallel)
{
    // Prepare providers
    test_parameter_provider parameter_provider_1(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_1), default_string_value);
    test_parameter_provider parameter_provider_2(wago::wdx::parameter_selector::all_with_definition(default_parameter_id_2), default_string_value);
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Register in optimal sequence without parameter providers
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread for registration of provider 1
    std::thread request_1_thread([this, &parameter_provider_1](){
        auto future_responses = core_m->register_parameter_providers( { &parameter_provider_1 } );
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        EXPECT_WDA_SUCCESS(responses.at(0));
    });

    // Start thread for registration of provider 2
    std::thread request_2_thread([this, &parameter_provider_2](){
        auto future_responses = core_m->register_parameter_providers( { &parameter_provider_2 } );
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        EXPECT_WDA_SUCCESS(responses.at(0));
    });

    request_2_thread.join();
    request_1_thread.join();

    // Start thread for unregistration of provider 1
    std::thread request_3_thread([this, &parameter_provider_1](){
        core_m->unregister_parameter_providers( { &parameter_provider_1 } );
    });

    // Start thread for unregistration of provider 2
    std::thread request_4_thread([this, &parameter_provider_2](){
        core_m->unregister_parameter_providers( { &parameter_provider_2 } );
    });

    request_3_thread.join();
    request_4_thread.join();

    EXPECT_FALSE(parameter_provider_1.has_error()) << parameter_provider_1.get_error_message();
    EXPECT_FALSE(parameter_provider_2.has_error()) << parameter_provider_2.get_error_message();
}

TEST_F(core_fixture, MultiThreadingModelProviderRegistrationsInParallel)
{
    // Prepare providers
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider_1(default_test_model_1);
    test_model_provider model_provider_2(default_test_model_2);
    std::vector<wago::wdx::model_provider_i*> const model_providers_1 = { &model_provider_1 };
    std::vector<wago::wdx::model_provider_i*> const model_providers_2 = { &model_provider_2 };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register needed stuff
    register_description_providers(core_m.get(), &description_providers);
    register_devices(              core_m.get(), &devices);

    // Start thread to register part 1
    std::thread request_1_thread([this, &request_1_complete, &model_providers_1](){
        auto future_responses = core_m->register_model_providers(model_providers_1);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_1_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to register part 2
    std::thread request_2_thread([this, &request_2_complete, &model_providers_2](){
        auto future_responses = core_m->register_model_providers(model_providers_2);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_2_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    request_2_thread.join();
    request_1_thread.join();

    auto parameter_request_responses_1 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses_1.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_1.at(0).status);
    auto parameter_request_responses_2 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_3) }).get();
    ASSERT_EQ(1, parameter_request_responses_2.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_2.at(0).status);
}

TEST_F(core_fixture, MultiThreadingDeviceDescriptionProviderRegistrationsInParallel)
{
    // Prepare providers
    test_description_provider description_provider_1(wago::wdx::device_selector::specific(default_device.device_id), default_test_description);
    test_description_provider description_provider_2(wago::wdx::device_selector::all_of(wago::wdx::device_collections::kbus), default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers_1 = { &description_provider_1 };
    std::vector<wago::wdx::device_description_provider_i*> const description_providers_2 = { &description_provider_2 };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register needed stuff
    register_model_providers(core_m.get(), &model_providers);
    register_devices(        core_m.get(), &devices);

    // Start thread to register part 1
    std::thread request_1_thread([this, &request_1_complete, &description_providers_1](){
        auto future_responses = core_m->register_device_description_providers(description_providers_1);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_1_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to register part 2
    std::thread request_2_thread([this, &request_2_complete, &description_providers_2](){
        auto future_responses = core_m->register_device_description_providers(description_providers_2);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_2_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    request_2_thread.join();
    request_1_thread.join();

    auto parameter_request_responses_1 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses_1.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_1.at(0).status);
}

TEST_F(core_fixture, MultiThreadingDeviceExtensionProviderRegistrationsInParallel)
{
    // Prepare providers
    test_extension_provider extension_provider_1(wago::wdx::device_selector::any, default_feature_id_1);
    test_extension_provider extension_provider_2(wago::wdx::device_selector::any, default_feature_id_2);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers_1 = { &extension_provider_1 };
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers_2 = { &extension_provider_2 };
    test_model_provider model_provider_1(default_test_model_1);
    test_model_provider model_provider_2(default_test_model_2);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider_1, &model_provider_2 };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register needed stuff
    register_model_providers(core_m.get(), &model_providers);
    register_devices(        core_m.get(), &devices);

    // Start thread to register part 1
    std::thread request_1_thread([this, &request_1_complete, &extension_providers_1](){
        auto future_responses = core_m->register_device_extension_providers(extension_providers_1);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_1_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to register part 2
    std::thread request_2_thread([this, &request_2_complete, &extension_providers_2](){
        auto future_responses = core_m->register_device_extension_providers(extension_providers_2);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_2_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    request_2_thread.join();
    request_1_thread.join();

    auto parameter_request_responses_1 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses_1.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_1.at(0).status);
    auto parameter_request_responses_2 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_3) }).get();
    ASSERT_EQ(1, parameter_request_responses_2.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_2.at(0).status);
}

// FIXME: Registration and unregistration changes status code of parameter requests
TEST_F(core_fixture, DISABLED_MultiThreadingDeviceExtensionProviderRegistrationAndUnregistrationInParallel)
{
    // Prepare providers
    test_extension_provider extension_provider_1(wago::wdx::device_selector::any, default_feature_id_1);
    test_extension_provider extension_provider_2(wago::wdx::device_selector::any, default_feature_id_2);
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers_1 = { &extension_provider_1 };
    std::vector<wago::wdx::device_extension_provider_i*> const extension_providers_2 = { &extension_provider_2 };
    test_model_provider model_provider_1(default_test_model_1);
    test_model_provider model_provider_2(default_test_model_2);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider_1, &model_provider_2 };
    std::vector<wago::wdx::register_device_request> const devices = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register needed stuff
    register_model_providers(    core_m.get(), &model_providers);
    register_devices(            core_m.get(), &devices);
    register_extension_providers(core_m.get(), &extension_providers_1);

    // Start thread to unregister part 1
    std::thread request_1_thread([this, &request_1_complete, &extension_providers_1](){
        core_m->unregister_device_extension_providers(extension_providers_1);
        request_1_complete.notify();
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to register part 2
    std::thread request_2_thread([this, &request_2_complete, &extension_providers_2](){
        auto future_responses = core_m->register_device_extension_providers(extension_providers_2);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_2_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    request_2_thread.join();
    request_1_thread.join();

    auto parameter_request_responses_1 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses_1.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::unknown_parameter_id, parameter_request_responses_1.at(0).status);
    auto parameter_request_responses_2 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_3) }).get();
    ASSERT_EQ(1, parameter_request_responses_2.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_2.at(0).status);
}

TEST_F(core_fixture, MultiThreadingDeviceRegistrationsInParallel)
{
    // Prepare providers
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    wago::wdx::register_device_request const second_device = { wago::wdx::device_id(4, wago::wdx::device_collections::kbus ), "0000-1234", "01.00.00" };
    std::vector<wago::wdx::register_device_request> const devices_1 = { default_device };
    std::vector<wago::wdx::register_device_request> const devices_2 = { second_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register needed stuff
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);

    // Start thread to register part 1
    std::thread request_1_thread([this, &request_1_complete, &devices_1](){
        auto future_responses = core_m->register_devices(devices_1);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_1_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to register part 2
    std::thread request_2_thread([this, &request_2_complete, &devices_2](){
        auto future_responses = core_m->register_devices(devices_2);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_2_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    request_2_thread.join();
    request_1_thread.join();

    auto parameter_request_responses_1 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses_1.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_1.at(0).status);
}

TEST_F(core_fixture, MultiThreadingDeviceRegistrationAndUnregistrationInParallel)
{
    // Prepare providers
    test_description_provider description_provider(wago::wdx::device_selector::any, default_test_description);
    std::vector<wago::wdx::device_description_provider_i*> const description_providers = { &description_provider };
    test_model_provider model_provider(default_test_model_1);
    std::vector<wago::wdx::model_provider_i*> const model_providers = { &model_provider };
    wago::wdx::register_device_request const second_device = { wago::wdx::device_id(4, wago::wdx::device_collections::kbus ), "0000-1234", "01.00.00" };
    std::vector<wago::wdx::register_device_request> const devices_1     = { second_device };
    std::vector<wago::wdx::device_id>               const devices_1_ids = { second_device.device_id };
    std::vector<wago::wdx::register_device_request> const devices_2     = { default_device };

    // Prepare thread sync stuff
    posix_convar request_1_complete;
    posix_convar request_2_complete;

    // Register needed stuff
    register_model_providers(      core_m.get(), &model_providers);
    register_description_providers(core_m.get(), &description_providers);
    register_devices(              core_m.get(), &devices_1);

    // Start thread to unregister part 1
    std::thread request_1_thread([this, &request_1_complete, &devices_1_ids](){
        auto future_responses = core_m->unregister_devices(devices_1_ids);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_1_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 1 to have request active
    request_1_complete.wait();

    // Start thread to register part 2
    std::thread request_2_thread([this, &request_2_complete, &devices_2](){
        auto future_responses = core_m->register_devices(devices_2);
        ASSERT_FUTURE_VALUE(future_responses);
        auto responses = future_responses.get();
        request_2_complete.notify();
        for(auto response : responses)
        {
            EXPECT_WDA_SUCCESS(response);
        }
    });

    // Wait for thread 2 to have unregister request active
    request_2_complete.wait();

    request_2_thread.join();
    request_1_thread.join();

    auto parameter_request_responses_1 = core_m->get_parameters({ wago::wdx::parameter_instance_id(default_parameter_id_1) }).get();
    ASSERT_EQ(1, parameter_request_responses_1.size());
    EXPECT_WDA_STATUS(wago::wdx::status_codes::parameter_not_provided, parameter_request_responses_1.at(0).status);
}


//---- End of source file ------------------------------------------------------
