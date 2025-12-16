//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
// introduction to googletest:
//      https://github.com/google/googletest/blob/master/googletest/docs/primer.md

// commands for starting tests in a terminal (examples)
//  all tests:                      build\test\ExampleTests
//  filtered by regular expression: build\test\ExampleTests --gtest_filter=*Accumulate


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <regex>
#include <chrono>
#include <thread>
#include <limits>

#include <nlohmann/json.hpp>

#include "wago/wdx/wdmm/method_definition.hpp"
#include "serial_parameter_provider.hpp"
#include "parameter_service_core.hpp"
#include "wda_ipc/representation.hpp"
#include "wda_ipc/ipc.hpp"
#include "utils/algo_helpers.hpp"
#include "utils/string_util.hpp"

#include "test_providers.hpp"
#include "common/log.hpp"
#include "mocks/mock_permissions.hpp"
#include "mocks/mock_model_provider.hpp"
#include "mocks/mock_device_description_provider.hpp"
#include "wdm.hpp"

using json = nlohmann::json;

using namespace std;
using namespace wago::wdx;
using namespace wago::wda_ipc;

// ************************************************************************************************************************************************
// Start of tests
// ************************************************************************************************************************************************
/*
TEST(ParameterServiceTests, CheckIfGoogleTestWorks)
{
    const bool result = true;

    EXPECT_EQ(true, result);                                        // Comparison equality
}
*/

struct parameter_service_test_fixture : public ::testing::Test
{
private:
    std::unique_ptr<mock_permissions> permissions_mock_ptr = std::make_unique<mock_permissions>();

public:
    rocket_headstation_provider provider;
    unique_ptr<parameter_service_i> service;
    mock_permissions &permissions_mock = *permissions_mock_ptr;
    mock_model_provider model_provider_mock;
    mock_device_description_provider device_description_provider_mock;

    // set up the test fixture.    
    virtual void SetUp() override 
    {
        cout << "Setting up the Test Fixure" << endl;
        permissions_mock.set_default_expectations();
        model_provider_mock.set_default_expectations();
        device_description_provider_mock.set_default_expectations();

        current_log_mode = strict;
        service = std::make_unique<parameter_service_core>(std::move(permissions_mock_ptr));
        ASSERT_EQ(true, service != nullptr) << "Create Service failed!";

        wago::wdx::wdm_response wdm(test_wdm);
        EXPECT_CALL(model_provider_mock, get_model_information())
        .WillOnce(testing::Return(testing::ByMove(wago::resolved_future<wago::wdx::wdm_response>(std::move(wdm)))));

        EXPECT_CALL(device_description_provider_mock, get_provided_devices())
        .WillRepeatedly(testing::Return(wago::wdx::device_selector_response({wago::wdx::device_selector::any})));
        EXPECT_CALL(device_description_provider_mock, get_device_information(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([](std::string order_number, std::string version){
            wago::wdx::wdd_response wdd;
            if(order_number == "0768-3301")
            {
                wdd = wago::wdx::wdd_response(test_wdd_0768_3301);
            }
            else if(order_number == "0763-1108")
            {
                wdd = wago::wdx::wdd_response(test_wdd_0763_1108);
            }
            else if(order_number == "0763-1508")
            {
                wdd = wago::wdx::wdd_response(test_wdd_0763_1508);
            }
            else if(order_number == "0999-0999")
            {
                //No description available
            }
            else
            {
                throw std::runtime_error(("Order number " + order_number + " not supported by mock").c_str());
            }
            return wago::resolved_future<wago::wdx::wdd_response>(std::move(wdd));
        }));

        service->register_model_providers({ &model_provider_mock });
        service->register_device_description_providers({ &device_description_provider_mock});
        service->register_parameter_providers({&provider});

        service->register_devices({ register_device_request{device_id::headstation, "0768-3301", "01.02.03"}, register_device_request{device_id(1, device_collections::rlb), "0763-1108", "01.00.00"} });
    }

    // Tears down the test fixture.
    virtual void TearDown() override 
    {
        cout << "Tearing down the Test Fixure" << endl;
    }
};

TEST_F(parameter_service_test_fixture, check_feature_instance_detection)
{
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Extended/Param", "0-0"),
            parameter_instance_path("Base/Param", "0-0"),
            parameter_instance_path("TestClass2", "0-0"),
            parameter_instance_path("TestClass2/1/ClassParamC2", "0-0"),
            parameter_instance_path("TestClass3", "0-0"),
            parameter_instance_path("TestClass3/1/ClassParamC3", "0-0"),
            parameter_instance_path("TestClass4", "0-0"),
            parameter_instance_path("TestClass4/1/ClassParamC4", "0-0"),
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[3].status, status_codes::success);
        EXPECT_EQ(r[4].status, status_codes::success);
        EXPECT_EQ(r[5].status, status_codes::success);
        EXPECT_EQ(r[6].status, status_codes::success);
        EXPECT_EQ(r[7].status, status_codes::success);
        EXPECT_FALSE(r[0].definition->feature_def.expired());
        EXPECT_FALSE(r[1].definition->feature_def.expired());
        EXPECT_FALSE(r[2].definition->feature_def.expired());
        EXPECT_FALSE(r[3].definition->feature_def.expired());
        EXPECT_FALSE(r[3].definition->class_def.expired());
        EXPECT_FALSE(r[4].definition->feature_def.expired());
        EXPECT_FALSE(r[5].definition->feature_def.expired());
        EXPECT_FALSE(r[5].definition->class_def.expired());
        EXPECT_FALSE(r[6].definition->feature_def.expired());
        EXPECT_FALSE(r[7].definition->feature_def.expired());
        EXPECT_FALSE(r[7].definition->class_def.expired());
    }
}

TEST_F(parameter_service_test_fixture, register_single_RLBT)
{
    register_device_request request;
    request.device_id = device_id(2, device_collections::rlb);
    request.order_number = "0763-1108";
    request.firmware_version = "01.00.00";
    auto response = service->register_devices({ request }).get();

    for (auto& res: response)
    {
        EXPECT_FALSE(res.has_error()) << "Register RLBT '" << request.order_number << "' at slot '" << request.device_id.slot << "' failed!";
    }

    {
        auto r = service->register_devices({register_device_request{device_id(270, device_collections::rlb), "0763-1108", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::invalid_device_slot);
    }

    {
        auto r = service->register_devices({register_device_request{device_id(2, 4), "0763-1108", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::invalid_device_collection);
    }
    {
        auto r = service->unregister_devices({device_id(2, 4)}).get();
        EXPECT_EQ(r[0].status, status_codes::invalid_device_collection);
    }
    {
        auto r = service->get_device({device_id(2, device_collections::rlb)}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->unregister_devices({device_id(2, device_collections::rlb)}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_device({device_id(2, device_collections::rlb)}).get();
        EXPECT_EQ(r.status, status_codes::unknown_device);
    }

    // not treating warnings as errors for the following part, because we expect there to be warnings
    current_log_mode = lax;
    {
        auto r = service->register_devices({register_device_request{device_id(3, device_collections::rlb), "0763-1108", "meineFwVersion"}}).get();
        EXPECT_EQ(r[0].status, status_codes::wrong_value_pattern);
    }
}

TEST_F(parameter_service_test_fixture, register_parameter_providers)
{
    auto prov1 = ThatDeviceProvider();
    auto prov2 = ThatDeviceProvider();
    auto res1 = service->register_parameter_providers({ &prov1, &prov2 }).get();

    for (auto& res: res1)
    {
        EXPECT_EQ(res.status, status_codes::success);
    }

    current_log_mode = lax; // Lax logger necessary because ThatDeviceProvider does not provide anything
    service->unregister_parameter_providers({ &prov1 });
    auto pp = ThatDeviceProvider();
    auto pp2 = ThatDeviceProvider();
    auto res4 = service->register_parameter_providers({ &pp2, &pp }).get();
    service->unregister_parameter_providers({ &pp });
    auto pp3 = ThatDeviceProvider();
    auto pp4 = ThatDeviceProvider();
    auto res6 = service->register_parameter_providers({ &pp3, &pp4 });
    service->unregister_parameter_providers({ &prov2, &pp2, &pp3, &pp4 });
    current_log_mode = strict;

    {
        auto r = service->register_devices(
            {
                register_device_request{device_id(3, device_collections::rlb), "0763-1508", "01.00.00"},
                register_device_request{device_id(4, device_collections::rlb), "0768-3301", "01.00.00"},
            }
        ).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    auto prov3 = test_provider(); // this one returns parameter_selectors at start_providing
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Channels/1/Value/IoValue", "2-3"),
            parameter_instance_path("Identity/SerialNumber", "2-3"),
        }).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_not_provided); // because prov3 hasn't been registered yet
        EXPECT_EQ(r[1].status, status_codes::parameter_not_provided); // because prov3 hasn't been registered yet
    }
    {
        // prov3 will use parameter_selectors to provider for parameters:
        // parameter_selector::for_all_devices_with("0763-1108") -> a device that hasn't been registered yet
        // parameter_selector::for_all_parameters_with(1100, "0763-1508") -> this device is already present
        auto r = service->register_parameter_providers({&prov3}).get(); 
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Channels/1/Value/IoValue", "2-3"), // this has the definition id 1100 that was selected by prov3
            parameter_instance_path("Identity/SerialNumber", "2-4"), // this belongs to WAGODevice that was selected by prov3
            parameter_instance_path("Network/Switch/SeparateInterfaces", "2-4"), // this does not belong to WAGODevice
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::parameter_not_provided);
    }
    {
        // now a device is registered later than prov3,
        // but since it matches the parameter_selector from prov3,
        // prov3 will by used for all parameters of this device
        auto r = service->register_devices({
            register_device_request{device_id(1, device_collections::kbus), "0763-1108", "01.00.00"},
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Identity/SerialNumber", "1-1"), // prov3 provides for all the kbus
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        service->unregister_parameter_providers({&prov3});
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Channels/1/Value/IoValue", "2-3"),
            parameter_instance_path("Identity/SerialNumber", "2-3"),
            }).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_not_provided);
        EXPECT_EQ(r[1].status, status_codes::parameter_not_provided);
    }
}

TEST_F(parameter_service_test_fixture, get_subdevices) {
    {
        auto r = service->get_all_devices().get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.devices[0].id, device_id::headstation);
        EXPECT_EQ(r.devices[0].order_number, "0768-3301");
        EXPECT_EQ(r.devices[0].firmware_version, "01.02.03");

        EXPECT_EQ(r.devices[1].id, device_id(1,2));
        EXPECT_EQ(r.devices[1].order_number, "0763-1108");
        EXPECT_EQ(r.devices[1].firmware_version, "01.00.00"); // firmware-version was not set at register_devices but mocked
    }
    
    {
        auto r = service->get_subdevices(device_collections::root).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.devices[0].id, device_id::headstation);
        EXPECT_EQ(r.devices[0].order_number, "0768-3301");
        EXPECT_EQ(r.devices[0].firmware_version, "01.02.03");
    }

    {
        auto r = service->get_subdevices(device_collections::rlb).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.devices[0].id, device_id(1,2));
        EXPECT_EQ(r.devices[0].order_number, "0763-1108");
        EXPECT_EQ(r.devices[0].firmware_version, "01.00.00"); // firmware-version was not set at register_devices but mocked
    }

    {
        auto r = service->get_subdevices_by_collection_name("RLB").get();
        EXPECT_EQ(r.devices[0].id, wago::wdx::device_id(1,2));
    }

    {
        auto r = service->get_subdevices_by_collection_name("rLb").get(); // case invariant
        EXPECT_EQ(r.devices[0].id, wago::wdx::device_id(1,2));
    }

    {
        auto r = service->get_subdevices_by_collection_name("RLBA").get();
        EXPECT_EQ(r.status, status_codes::unknown_device_collection);
    }

    {
        auto r = service->get_device(device_id(1,2)).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.id, device_id(1,2));
        EXPECT_EQ(r.order_number, "0763-1108");
        EXPECT_EQ(r.firmware_version, "01.00.00");
    }
    {
        auto r = service->get_device(device_id(1,3)).get();
        EXPECT_EQ(r.status, status_codes::invalid_device_collection);
    }
    {
        auto r = service->get_device(device_id(5,2)).get();
        EXPECT_EQ(r.status, status_codes::unknown_device);
    }
}

TEST_F(parameter_service_test_fixture, get_parameter_values) {
    {
        auto result = service->get_parameters({ parameter_instance_id(1), parameter_instance_id(2001, 2), parameter_instance_id(1110, 2, {1,2}), parameter_instance_id(1040, 1) }).get();
        EXPECT_EQ(result[0].value->get_string(), "0768-3301");
        EXPECT_EQ(result[1].status, status_codes::unknown_parameter_id); // this device will not have this parameter according to model
        EXPECT_EQ(result[1].id, parameter_instance_id(2001, 2)); // but id will be mirrored
        EXPECT_EQ(result[2].status, status_codes::parameter_not_provided); // this parameter exists according to model, but noone has provided for it
        EXPECT_TRUE(result[2].definition);
        EXPECT_EQ(result[2].id.id, 1110);
        EXPECT_EQ(result[2].id.instance_id, 2);
        EXPECT_EQ(result[2].id.device, device_id(1,2));
        EXPECT_EQ(result[2].path, parameter_instance_path("Channels/2/Value/IoValue", "2-1"));
        EXPECT_EQ(result[3].status, status_codes::parameter_value_unavailable); // this parameter exists according to model, but rocket_headstation_provider can't retrieve value
    }

    { 
        auto result = service->get_parameters_by_path({
            parameter_instance_path("Identity/OrderNumber"),
            parameter_instance_path("Connectors/2/CrossSection"),
            parameter_instance_path("Channels/2/Value/IoValue", "2-1"),
            parameter_instance_path("Channels/2/Value/IoValue", "2-1"),
            parameter_instance_path("Channels/2/Value/IoValue", "3-1"),
            parameter_instance_path("Channels/2/Value/IoValue", "2-300"),
            parameter_instance_path("Channels/2/Value/IoValue", "2-2"),
            parameter_instance_path("Network/EthernetPorts/1/MacAddresses"),
            parameter_instance_path("Test/InstanceRefParam"),
            parameter_instance_path("Test/IpParam"),
            parameter_instance_path("Test/InstanceRefArray"),
            parameter_instance_path("Test/InstanceRefArrayProvided"),
            parameter_instance_path("Test/ParamWithDomainSpecificError"),
            }).get();
        EXPECT_EQ(result[0].value->get_string(), "0768-3301");
        EXPECT_EQ(result[1].status, status_codes::unknown_parameter_path); // this device will not have this parameter according to model
        EXPECT_EQ(result[1].path, parameter_instance_path("Connectors/2/CrossSection")); // but path will be mirrored
        EXPECT_EQ(result[2].status, status_codes::parameter_not_provided); // this parameter exists according to model, but noone has provided for it
        EXPECT_EQ(result[3].status, status_codes::parameter_not_provided); // case invariant device path
        EXPECT_EQ(result[4].status, status_codes::invalid_device_collection);
        EXPECT_EQ(result[5].status, status_codes::invalid_device_slot);
        EXPECT_EQ(result[6].status, status_codes::unknown_device);
        EXPECT_EQ(result[7].status, status_codes::parameter_value_unavailable); // this parameter exists according to model, but rocket_headstation_provider can't retrieve value
        EXPECT_EQ(result[8].status, status_codes::parameter_value_unavailable);
        ASSERT_TRUE(result[8].definition);
        EXPECT_EQ(result[8].definition->value_type, parameter_value_types::instance_ref);
        EXPECT_EQ(result[8].definition->ref_classes.at(0), "TestClass");
        EXPECT_FALSE(result[8].definition->ref_classes_def.at(0).expired());
        // TODO: RefSemantics
        EXPECT_EQ(result[9].status, status_codes::success);
        EXPECT_EQ(result[9].value->get_ipv4address(), "2.3.4.5");
        EXPECT_EQ(result[9].definition->overrideables.default_value->get_ipv4address(), "1.2.3.4");
        EXPECT_EQ(result[10].status, status_codes::success);
        EXPECT_EQ(result[10].value->get_items()[1].get_instance_ref(), 3);
        EXPECT_EQ(result[11].status, status_codes::success);
        EXPECT_EQ(result[11].value->get_items().size(), 0);
        EXPECT_EQ(result[11].value->get_json(), "[]");
        EXPECT_EQ(result[12].status, status_codes::parameter_value_unavailable);
        EXPECT_EQ(result[12].domain_specific_status_code, 42);
        EXPECT_EQ(result[12].message, "Ups");
        
    }

    { 
        auto result = service->get_parameter_definitions_by_path({
            parameter_instance_path("Channels/2/Value/IoValue", "2-1"),
            parameter_instance_path("Network/EthernetPorts/1/MacAddresses"),
            parameter_instance_path("Test/IpParam")
            }).get();
        EXPECT_EQ(result[0].status, status_codes::success); // noone has provided for it, but the definition is known
        EXPECT_EQ(result[1].status, status_codes::success); // rocket_headstation_provider won't be asked
        EXPECT_EQ(result[2].status, status_codes::success);
        EXPECT_FALSE(result[2].value);
        EXPECT_EQ(result[2].definition->overrideables.default_value->get_ipv4address(), "1.2.3.4");
    }

    {
        auto r1 = service->get_parameters({}).get();
        auto r2 = service->get_parameters_by_path({}).get();
        EXPECT_EQ(r1.size(), 0);
        EXPECT_EQ(r2.size(), 0);
    }

    {
        auto r = service->get_parameters_by_path({ parameter_instance_path("Network/IPv4/IpAddress/Static") }).get();
        EXPECT_EQ(r[0].status, status_codes::unknown_parameter_path); // the parameter is in a class instance, but path contains no instance id
    }

    {
        auto r = service->get_parameters_by_path({ parameter_instance_path("Test/ReadonlyParamWithDefault") }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_uint16(), 42); // the default value is given
    }

    // not treating warnings as errors for the following part, because we expect there to be warnings
    current_log_mode = lax;

    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Test/ProvidedParam"),
            parameter_instance_path("Test/WriteableParamWithPattern"),
            parameter_instance_path("Test/ParamWithEvilAnswer"),
        }).get();
        EXPECT_EQ(r[0].status, status_codes::internal_error); // the parameter provider returns wrong data type
        EXPECT_EQ(r[1].status, status_codes::internal_error); // the parameter provider returns wrong pattern
        EXPECT_EQ(r[2].status, status_codes::internal_error); // the parameter provider returns a "successful" response with no value
    }

    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Test/ExceptionParam")
        }).get();
        EXPECT_EQ(r[0].status, status_codes::internal_error); // the parameter provider throws an exception
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Test/ExceptionParam", "0-300"),
            parameter_instance_path("Test/ExceptionParam", "200-1"),
            parameter_instance_path("Test/ExceptionParam", "2-65536"),
            parameter_instance_path("Test/ExceptionParam", "65536-1"),
            parameter_instance_path("Test/ExceptionParam", "2-4294967296"),
            parameter_instance_path("Test/ExceptionParam", "4294967296-1"),
        }).get();
        EXPECT_EQ(r[0].status, status_codes::invalid_device_slot);
        EXPECT_EQ(r[1].status, status_codes::invalid_device_collection);
        EXPECT_EQ(r[2].status, status_codes::invalid_device_slot);
        EXPECT_EQ(r[3].status, status_codes::invalid_device_collection);
        EXPECT_EQ(r[4].status, status_codes::invalid_device_slot);
        EXPECT_EQ(r[5].status, status_codes::invalid_device_collection);
    }
}

TEST_F(parameter_service_test_fixture, set_parameter_values) {

    EXPECT_EQ(service->job_count, 0);

    {
        auto r = service->set_parameter_values_by_path({
            {{"Connectors/2/CrossSection"}, parameter_value::create_float32(2.5)},
            {{"ManualOverrideActive", "2-1"}, parameter_value::create(true)},
            {{"Services/1/Enable"}, parameter_value::create(true)},
            {{"Services/1/Enable"}, parameter_value::create_with_unknown_type("true")},
            {{"Test/WriteableParamWithPattern"}, parameter_value::create("Hallo Du")},
            {{"Test/IpParamArray"}, parameter_value::create_ipv4address_array({"255.143.1.2", "0.0.0.0", "192.168.1.28"})},
            }).get();
            EXPECT_EQ(r[0].status, status_codes::unknown_parameter_path); // this device will not have this parameter according to model
            EXPECT_EQ(r[1].status, status_codes::parameter_not_provided); // this parameter exists according to model, but noone has provided for it
            EXPECT_EQ(r[2].status, status_codes::could_not_set_parameter); // this parameter exists according to model, but rocket_headstation_provider can't set value
            EXPECT_EQ(r[3].status, status_codes::could_not_set_parameter); // value type will be complemented by model, but rocket_headstation_provider can't set value
            EXPECT_EQ(r[3].domain_specific_status_code, 0); // nothing domain specific known
            EXPECT_EQ(r[4].status, status_codes::could_not_set_parameter); // valid according to pattern, but rocket_headstation_provider can't set value
            EXPECT_EQ(r[4].domain_specific_status_code, 43); // domain specific status code of rocket_headstation_provider
            EXPECT_EQ(r[4].message, "Mein Fehlertext"); // domain specific status message of rocket_headstation_provider
            EXPECT_EQ(r[5].status, status_codes::could_not_set_parameter); // valid, but provider not completely implemented
    }
    {
        auto r = service->set_parameter_values_by_path({
            {{"Connectors/2/CrossSection"}, parameter_value::create_float32(2.5)},
            {{"ManualOverrideActive", "2-1"}, parameter_value::create(true)},
            {{"Services/1/Enable"}, parameter_value::create(true)},
            {{"Services/1/Enable"}, parameter_value::create_with_unknown_type("true")},
            {{"Services/1/Enable"}, parameter_value::create("Welt")},
            {{"Network/EthernetPorts/1/MacAddresses"}, parameter_value::create("Welt")},
            {{"Test/WriteableParamWithPattern"}, parameter_value::create("Hallo Welt")},
            {{"Test/WriteableParamWithPattern"}, parameter_value::create("Hallo Du")},
            {{"Test/IpParam"}, parameter_value::create_ipv4address("255.256.1.2")},
            {{"Test/IpParam"}, parameter_value::create_ipv4address("255.2541.2")},
            {{"Test/IpParamArray"}, parameter_value::create_ipv4address_array({"255.143.1.2", "0.0.0.0", "192.168.1.28"})},
            {{"Test/IpParamArray"}, parameter_value::create_ipv4address_array({"255.143.1.2", "0.0.0.256", "192.168.1.28"})}
            }).get();
            EXPECT_EQ(r[0].status, status_codes::unknown_parameter_path);
            EXPECT_EQ(r[1].status, status_codes::parameter_not_provided);
            EXPECT_EQ(r[2].status, status_codes::other_invalid_value_in_set);
            EXPECT_EQ(r[3].status, status_codes::other_invalid_value_in_set);
            EXPECT_EQ(r[4].status, status_codes::wrong_value_type); // type of given value does not match model
            EXPECT_EQ(r[5].status, status_codes::parameter_not_writeable); // not writeable according to model
            EXPECT_EQ(r[6].status, status_codes::wrong_value_pattern); // invalid according to pattern
            EXPECT_EQ(r[7].status, status_codes::other_invalid_value_in_set);
            EXPECT_EQ(r[8].status, status_codes::wrong_value_representation); // invalid ip address format
            EXPECT_EQ(r[9].status, status_codes::wrong_value_representation); // invalid ip address format
            EXPECT_EQ(r[10].status, status_codes::other_invalid_value_in_set);
            EXPECT_EQ(r[11].status, status_codes::wrong_value_representation); // one value in the array has invalid ip address format
    }

    {
        auto r = service->set_parameter_values({
            {parameter_instance_id{2004, 0, {1,2}}, parameter_value::create(true)},
            {parameter_instance_id{2300, 1}, parameter_value::create(true)}}).get();
            EXPECT_EQ(r[0].status, status_codes::parameter_not_provided); // this parameter exists according to model, but noone has provided for it
            EXPECT_EQ(r[1].status, status_codes::could_not_set_parameter); // this parameter exists according to model, but rocket_headstation_provider can't set value
    }

    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/EnumParam"}, parameter_value::create_uint16(2)},
            {{"Test/EnumParam"}, parameter_value::create_enum_value(2)},
            {{"Test/EnumParam"}, parameter_value::create_enum_value(3)}
            }).get();
            EXPECT_EQ(r[0].status, status_codes::wrong_value_type); // an uint16 parameter is not an enum parameter even if its value is held by an uint16
            EXPECT_EQ(r[1].status, status_codes::value_not_possible); // this device will not have this parameter according to model
            EXPECT_EQ(r[2].status, status_codes::other_invalid_value_in_set);
    }

    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/ParamWithAllowedValues"}, parameter_value::create_int8(-2)},
            {{"Test/ParamWithAllowedValues"}, parameter_value::create_int8(101)},
            {{"Test/ParamWithAllowedValues"}, parameter_value::create_int8(2)},
            {{"Test/ParamWithAllowedValues"}, parameter_value::create_int8(6)},
            {{"Test/ParamWithAllowedValues"}, parameter_value::create_int8(3)},
            }).get();
            EXPECT_EQ(r[0].status, status_codes::invalid_value);
            EXPECT_EQ(r[1].status, status_codes::invalid_value);
            EXPECT_EQ(r[2].status, status_codes::invalid_value);
            EXPECT_EQ(r[3].status, status_codes::invalid_value);
            EXPECT_EQ(r[4].status, status_codes::other_invalid_value_in_set);
    }

    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/ArrayWithAllowedValues"}, parameter_value::create_int8_array({3, -2})},
            {{"Test/ArrayWithAllowedValues"}, parameter_value::create_int8_array({3, 101})},
            {{"Test/ArrayWithAllowedValues"}, parameter_value::create_int8_array({3, 2})},
            {{"Test/ArrayWithAllowedValues"}, parameter_value::create_int8_array({3, 6})},
            {{"Test/ArrayWithAllowedValues"}, parameter_value::create_int8_array({3, 3})},
        }).get();
            EXPECT_EQ(r[0].status, status_codes::invalid_value);
            EXPECT_EQ(r[1].status, status_codes::invalid_value);
            EXPECT_EQ(r[2].status, status_codes::invalid_value);
            EXPECT_EQ(r[3].status, status_codes::invalid_value);
            EXPECT_EQ(r[4].status, status_codes::other_invalid_value_in_set);
    }

    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/ParamWithAllowedValuesInWDD"}, parameter_value::create_enum_value(1)},
            {{"Test/ParamWithAllowedValuesInWDD"}, parameter_value::create_enum_value(5)},
            }).get();
            EXPECT_EQ(r[0].status, status_codes::invalid_value);
            EXPECT_EQ(r[1].status, status_codes::other_invalid_value_in_set);
    }

    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/ParamWithAllowedLengthOverriddenInWDD"}, parameter_value::create_string_array({"Hallo", "Du"})},
            {{"Test/ParamWithAllowedLengthOverriddenInWDD"}, parameter_value::create_string_array({"Hallo"})},
            }).get();
            EXPECT_EQ(r[0].status, status_codes::invalid_value); // AllowedLength exceeded
            EXPECT_EQ(r[1].status, status_codes::other_invalid_value_in_set);
    }

    {
        auto r = service->set_parameter_values({
            {parameter_instance_id{2300, 1}, nullptr}}).get(); // null check
            EXPECT_EQ(r[0].status, status_codes::value_null);
    }

    autarc_parameter_provider app;
    {
        auto r = service->register_model_provider(&app).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(&app).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_parameter_provider(&app).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/ParamWithAllowedValuesInWDD"}, parameter_value::create_enum_value(1)}, // enum value not allowed
            {{"WriteableParam1"}, parameter_value::create_string("Hallo")}, // valid value for another parameterProvider
        }).get();
        EXPECT_EQ(r[0].status, status_codes::invalid_value);
        EXPECT_EQ(r[1].status, status_codes::success); // because param belongs to different parameterProvider, it can be set independently from the other parameter
    }

    // not treating warnings as errors for the following part, because we expect there to be warnings
    current_log_mode = lax;
    
    {
        auto r = service->set_parameter_values_by_path({
            {{"Test/ExceptionParam"}, parameter_value::create("Hallo")}
            }).get();
        EXPECT_EQ(r[0].status, status_codes::internal_error); // parameter provider throws an exception
    }

    EXPECT_EQ(service->job_count, 0);
}

TEST_F(parameter_service_test_fixture, overrides) {

    {
        auto r = service->get_parameters_by_path({ parameter_instance_path("Test/OverriddenByFeature") }).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_value_unavailable);
        EXPECT_EQ(r[0].definition->overrideables.inactive, true);
    }

    {
        auto r = service->get_parameters_by_path({ parameter_instance_path("Test/OverriddenByDevice") }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->overrideables.inactive, true);
        EXPECT_EQ(r[0].value->get_uint16(), 5);
    }

}

TEST_F(parameter_service_test_fixture, invoke_method) {
    {
        auto r = service->invoke_method(parameter_instance_id(4012), {}).get();
        EXPECT_EQ(r.status, status_codes::missing_argument);
    }

    {
        auto r = service->invoke_method(parameter_instance_id(4012), { {"WrongArgumentName", parameter_value::create("Hallo")} }).get();
        EXPECT_EQ(r.status, status_codes::missing_argument);
    }

    {
        auto r = service->invoke_method(parameter_instance_id(4012), { {"RequestData", parameter_value::create("Hallo")} }).get();
        EXPECT_EQ(r.status, status_codes::wrong_value_type);
    }

    {
        auto r = service->invoke_method(parameter_instance_id(4012), { {"RequestData", nullptr} }).get();
        EXPECT_EQ(r.status, status_codes::value_null);
    }

    {
        auto r = service->invoke_method(parameter_instance_id(4012), { {"RequestData", parameter_value::create_with_unknown_type("noJSON")} }).get();
        EXPECT_EQ(r.status, status_codes::wrong_value_representation); // parameterservice will try to supplement the correct argument type (bytes), but parameter_value::set_type_internal fails
    }

    {
        auto r = service->invoke_method(parameter_instance_id(4012), { {"RequestData", parameter_value::create_with_unknown_type("\"noBase64!\"")} }).get();
        EXPECT_EQ(r.status, status_codes::wrong_value_representation); // parameterservice will supplement the correct argument type (bytes), but parameter_value::get_bytes will fail
    }

    current_log_mode = lax;

    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithOutArgs"), {}).get();
        EXPECT_EQ(r.status, status_codes::internal_error); // parameter provider returns wrong data type
    }

    current_log_mode = strict;
    
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithDefaultArg"), { {"Arg1", parameter_value::create("Kuckuck")} }).get(); // omitting Arg2, which has a DefaultValue
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.out_args["Result"]->get_string(), "KuckuckMyDefault");
    }
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithEnumArgs"), { {"Ping", parameter_value::create_enum_value(1)} }).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.out_args["Pongs"]->get_items().at(0).get_enum_value(), 3);
        EXPECT_EQ(r.out_args["PingID"]->get_uint16(), 1);
    }
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithInstanceRefArgs"), { {"Ping", parameter_value::create_instance_ref(1)} }).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.out_args["Pongs"]->get_items().at(0).get_instance_ref(), 2);
        EXPECT_EQ(r.out_args["PingID"]->get_uint16(), 1);
    }
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithEnumArgs"), { {"Ping", parameter_value::create_enum_value(2)} }).get();
        EXPECT_EQ(r.status, status_codes::value_not_possible);
    }
    current_log_mode = lax;
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithEnumArgs"), { {"Ping", parameter_value::create_enum_value(100)} }).get(); // this will provoke invalid Arg
        EXPECT_EQ(r.status, status_codes::internal_error);
    }
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("Test/MethodWithException"), {}).get();
        EXPECT_EQ(r.status, status_codes::internal_error);
    }
    current_log_mode = strict;
}

TEST_F(parameter_service_test_fixture, invoke_method_with_bytes)
{
    {
        bytes_t a({1, 5, 65, 200});
        auto pv = parameter_value::create_bytes(a);
        
        // rocket_headstation_provider should be able to call get_bytes without error
        auto result = service->invoke_method_by_path(parameter_instance_path("ServiceCommunication750/Protocol1288Request"), {{"RequestData", pv }}).get();
        auto b = result.out_args["ResponseData"]->get_bytes();
        EXPECT_EQ(b[0], a[0]);
        EXPECT_EQ(b[1], a[1]);
        EXPECT_EQ(b[2], a[2]);
        EXPECT_EQ(b[3], a[3]);
        EXPECT_EQ(b[4], 255); // rocket_headstation_provider will append 255 to bytes
    }
    {
        bytes_t a({1, 5, 65, 200});
        auto pv = parameter_value::create_bytes(a);
        auto pv2 = parameter_value::create_with_unknown_type(pv->get_json()); // bytes should be base64 encoded
        
        // service should detect the correct type
        // rocket_headstation_provider should be able to call get_bytes without error
        auto result = service->invoke_method_by_path(parameter_instance_path("ServiceCommunication750/Protocol1288Request"), {{"RequestData", pv2 }}).get();
        auto b = result.out_args["ResponseData"]->get_bytes();
        EXPECT_EQ(b[0], a[0]);
        EXPECT_EQ(b[1], a[1]);
        EXPECT_EQ(b[2], a[2]);
        EXPECT_EQ(b[3], a[3]);
        EXPECT_EQ(b[4], 255); // rocket_headstation_provider will append 255 to bytes
    }
}

TEST_F(parameter_service_test_fixture, get_all_parameters) {
    // not treating warnings as errors for the following part, because we expect there to be warnings
    current_log_mode = lax;
    {
        auto r = service->get_all_parameters(device_selector::headstation()).get();
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_FALSE(p.definition->value_type == parameter_value_types::method);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_methods()).get();
        EXPECT_EQ(r.param_responses.size(), 0);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_usersettings() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_TRUE(p.definition->user_setting && !p.definition->overrideables.inactive);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::without_usersettings() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_FALSE(p.definition->user_setting && !p.definition->overrideables.inactive);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_writeable() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_TRUE(p.definition->writeable && !p.definition->overrideables.inactive);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::without_writeable() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_FALSE(p.definition->writeable && !p.definition->overrideables.inactive);
        }
    }
    {
        bool found_dev1, found_dev2;
        auto r = service->get_all_parameters(parameter_filter::any).get();
        for(auto& p : r.param_responses) {
            if(p.id.device == device_id::headstation)
                found_dev1 = true;
            else if(p.id.device == device_id(1, device_collections::rlb))
                found_dev2 = true;
            
            EXPECT_FALSE(p.definition->value_type == parameter_value_types::method);
            EXPECT_EQ(dynamic_pointer_cast<method_definition>(p.definition), nullptr);
        }
        EXPECT_TRUE(found_dev1);
        EXPECT_TRUE(found_dev2);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::without_beta() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_FALSE(p.definition->is_beta);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_beta() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_TRUE(p.definition->is_beta);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::without_deprecated() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_FALSE(p.definition->is_deprecated);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_deprecated() | device_selector::headstation()).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.id.device, device_id::headstation);
            EXPECT_TRUE(p.definition->is_deprecated);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_feature("hasipv4")).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.definition->feature_def.lock()->name, "HasIPv4");
        }
    }
    {
        auto r = service->get_all_parameter_definitions(parameter_filter::only_feature("hasipv4")).get();
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            EXPECT_EQ(p.value, nullptr);
            EXPECT_EQ(p.definition->feature_def.lock()->name, "HasIPv4");
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("identity/orderNumber")).get();
        EXPECT_EQ(r.param_responses.size(), 2);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "Identity/OrderNumber"));
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("dynamics/1/param0")).get();
        EXPECT_EQ(r.param_responses.size(), 1);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "Dynamics/1/Param0"));
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("dynamics/1/param0") | device_selector::specific(device_id(1,2))).get();
        EXPECT_EQ(r.param_responses.size(), 0);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("identity") | device_selector::headstation()).get();
        EXPECT_EQ(r.param_responses.size(), 3);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "Identity"));
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("dynamics")).get();
        EXPECT_GT(r.param_responses.size(), 1);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "Dynamics"));
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("dynamics")).get();
        EXPECT_GT(r.param_responses.size(), 1);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "Dynamics"));
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("dynamicstuff/DynamicReadonly/1001")).get();
        EXPECT_EQ(r.param_responses.size(), 1);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "DynamicStuff/DynamicReadonly/1001"));
        }
    }
    {
        auto r = service->get_all_method_definitions(parameter_filter::only_subpath("dynamicstuff/DynamicReadonly/1001")).get();
        EXPECT_EQ(r.param_responses.size(), 1);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "DynamicStuff/DynamicReadonly/1001"));
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::only_subpath("dynamicstuff/DynamicReadonly/1001")).get();
        EXPECT_EQ(r.param_responses.size(), 1);
        for(auto& p : r.param_responses) {
            EXPECT_TRUE(starts_with(p.path.parameter_path, "DynamicStuff/DynamicReadonly/1001"));
            EXPECT_TRUE(p.definition->value_type != parameter_value_types::method);
        }
    }
    {
        auto r = service->get_all_parameters(parameter_filter::without_beta() | parameter_filter::only_beta()).get();
        EXPECT_EQ(r.param_responses.size(), 0);
    }
    size_t all_parameters_count;
    {
        auto r = service->get_all_parameters(parameter_filter::any).get();
        all_parameters_count = r.param_responses.size();
    }
    EXPECT_LT(3, all_parameters_count);
    {
        auto r = service->get_all_parameters(parameter_filter::any, 0, 3).get();
        EXPECT_EQ(r.param_responses.size(), 3);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, 2, 3).get();
        EXPECT_EQ(r.param_responses.size(), 3);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, 2, 0).get();
        EXPECT_EQ(r.param_responses.size(), 0);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, 0, SIZE_MAX).get();
        EXPECT_EQ(r.param_responses.size(), all_parameters_count);
        EXPECT_EQ(r.total_entries, all_parameters_count);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, 2, SIZE_MAX).get();
        EXPECT_EQ(r.param_responses.size(), all_parameters_count-2);
        EXPECT_EQ(r.total_entries, all_parameters_count);
    }

    EXPECT_LT(10, all_parameters_count);
    size_t current_startidx = 0;
    size_t page_size = all_parameters_count / 2 - 5; // this assumes we have more than 10 params
    {
        auto r = service->get_all_parameters(parameter_filter::any, current_startidx, page_size).get();
        EXPECT_EQ(r.param_responses.size(), page_size);
        current_startidx += page_size;
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, current_startidx, page_size).get();
        EXPECT_EQ(r.param_responses.size(), page_size);
        current_startidx += page_size;
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, current_startidx, page_size).get();
        EXPECT_EQ(r.param_responses.size(), all_parameters_count - current_startidx);
        current_startidx += page_size;
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any, current_startidx, page_size).get();
        EXPECT_EQ(r.param_responses.size(), 0);
    }
}

TEST_F(parameter_service_test_fixture, representation)
{
    {
        auto id1 = device_id(3, 2);
        auto s = to_string(id1);
        auto id2 = from_string<device_id>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<device_id>(""));
        EXPECT_ANY_THROW(from_string<device_id>("0"));
        EXPECT_ANY_THROW(from_string<device_id>("ab"));
        EXPECT_ANY_THROW(from_string<device_id>("a-b"));
        EXPECT_ANY_THROW(from_string<device_id>("-0-0"));
        EXPECT_ANY_THROW(from_string<device_id>("256-0"));
        EXPECT_ANY_THROW(from_string<device_id>("0-65536"));
    }
    {
        auto id1 = parameter_instance_id(100, 50, device_id(1, 2));
        auto s = to_string(id1);
        auto id2 = from_string<parameter_instance_id>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<parameter_instance_id>(""));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("a-b-c"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("abc"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("0-0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("0-0-0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("-0-0-0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("256-0-0-0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("0-65536-0-0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("0-0-4294967296-0"));
        EXPECT_ANY_THROW(from_string<parameter_instance_id>("0-0-0-65536"));
    }
    {
        auto p1 = parameter_instance_path("Test/Param/Value", "2-3");
        auto s = to_string(p1);
        auto p2 = from_string<parameter_instance_path>(s);
        EXPECT_EQ(p2, parameter_instance_path("test/param/value", "2/3"));
        EXPECT_EQ(p2, from_string<parameter_instance_path>("2-3-test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>(""));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("---"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("hallo"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("2-65536-test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("--test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("-1--test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("2-65536-test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("256-1-test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("2-4294967296-test-param-value"));
        EXPECT_ANY_THROW(from_string<parameter_instance_path>("4294967296-1-test-param-value"));
    }
    {
        parameter_id_t id1 = 145;
        auto s = wago::wda_ipc::to_string(id1);
        auto id2 = from_string<parameter_id_t>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<parameter_id_t>(""));
        EXPECT_ANY_THROW(from_string<parameter_id_t>("hallo"));
        EXPECT_ANY_THROW(from_string<parameter_id_t>("-3"));
        EXPECT_ANY_THROW(from_string<parameter_id_t>("4294967296"));
    }
    {
        instance_id_t id1 = 145;
        auto s = wago::wda_ipc::to_string(id1);
        auto id2 = from_string<instance_id_t>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<instance_id_t>(""));
        EXPECT_ANY_THROW(from_string<instance_id_t>("hallo"));
        EXPECT_ANY_THROW(from_string<instance_id_t>("-3"));
        EXPECT_ANY_THROW(from_string<instance_id_t>("65536"));
    }

    {
        device_collection_id_t id1 = 145;
        auto s = wago::wda_ipc::to_string(id1);
        auto id2 = from_string<device_collection_id_t>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<device_collection_id_t>(""));
        EXPECT_ANY_THROW(from_string<device_collection_id_t>("hallo"));
        EXPECT_ANY_THROW(from_string<device_collection_id_t>("-3"));
        EXPECT_ANY_THROW(from_string<device_collection_id_t>("256"));
    }

    {
        slot_index_t id1 = 145;
        auto s = wago::wda_ipc::to_string(id1);
        auto id2 = from_string<slot_index_t>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<slot_index_t>(""));
        EXPECT_ANY_THROW(from_string<slot_index_t>("hallo"));
        EXPECT_ANY_THROW(from_string<slot_index_t>("-3"));
        EXPECT_ANY_THROW(from_string<slot_index_t>("65536"));
    }

    {
        monitoring_list_id_t id1 = 0xffffffffffffffff;
        auto s = wago::wda_ipc::to_string(id1);
        auto id2 = from_string<monitoring_list_id_t>(s);
        EXPECT_EQ(id1, id2);
        EXPECT_ANY_THROW(from_string<monitoring_list_id_t>(""));
        EXPECT_ANY_THROW(from_string<monitoring_list_id_t>("hallo"));
        EXPECT_ANY_THROW(from_string<monitoring_list_id_t>("-3"));
    }

    {
        status_codes st1 = status_codes::value_not_possible;
        auto s = wago::wda_ipc::to_string(st1);
        auto st2 = from_string<status_codes>(s);
        EXPECT_EQ(st1, st2);
        EXPECT_EQ(wago::wdx::to_string(st2), "VALUE_NOT_POSSIBLE");
        EXPECT_ANY_THROW(from_string<status_codes>(""));
        EXPECT_ANY_THROW(from_string<status_codes>("hallo"));
        EXPECT_ANY_THROW(from_string<status_codes>("-3"));
    }

    {
        auto p1 = parameter_value_types::uint64;
        auto s = to_string(p1);
        auto p2 = from_string<parameter_value_types>(s);
        EXPECT_EQ(p2, parameter_value_types::uint64);
    }

    {
        auto p1 = parameter_value_rank::array;
        auto s = to_string(p1);
        auto p2 = from_string<parameter_value_rank>(s);
        EXPECT_EQ(p2, parameter_value_rank::array);
    }
}


TEST_F(parameter_service_test_fixture, ipc_representation)
{
    {
        auto o = from_ipc_string<class_instantiation>(R"({"Id": 1, "Classes": ["Hallo", "Du"]})");
        EXPECT_EQ(to_ipc_string<class_instantiation>(o), R"({"Classes":["Hallo","Du"],"Id":1})");
    }
    {
        auto o = from_ipc_string<vector<response>>(R"([{"Status": "SUCCESS"}, {"Status": "NOT_IMPLEMENTED"}])");
        EXPECT_EQ(to_ipc_string<vector<response>>(o), R"([{"Status":"SUCCESS"},{"Status":"NOT_IMPLEMENTED"}])");
    }
    {
        auto o = from_ipc_string<file_id_response>(R"({"Status": "SUCCESS", "FileId": "1243"})");
        EXPECT_EQ(to_ipc_string<file_id_response>(o), R"({"FileId":"1243","Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<device_id>(R"({"Slot": 1, "Collection": 2})");
        EXPECT_EQ(to_ipc_string<device_id>(o), R"({"Collection":2,"Slot":1})");
    }
    {
        auto o = from_ipc_string<register_device_request>(R"({"DeviceId": {"Slot": 1, "Collection": 2}, "OrderNumber": "0750-8202", "FirmwareVersion": "01.01.01"})");
        EXPECT_EQ(to_ipc_string<register_device_request>(o), R"({"DeviceId":{"Collection":2,"Slot":1},"FirmwareVersion":"01.01.01","OrderNumber":"0750-8202"})");
    }
    {
        auto o = from_ipc_string<device_collection_id_t>(R"(4)");
        EXPECT_EQ(to_ipc_string<device_collection_id_t>(o), R"(4)");
    }
    {
        auto o = from_ipc_string<size_t>(R"(4)");
        EXPECT_EQ(to_ipc_string<size_t>(o), R"(4)");
    }
    {
        auto o = from_ipc_string<parameter_selector_response>(R"({"Status": "SUCCESS", "Selectors": [{"Type": 2, "Name": "Hallo", "ParameterId": 123, "Device": {"Any": false, "WholeCollection": true, "Device": {"Slot": 1, "Collection": 3}}}]})");
        EXPECT_EQ(to_ipc_string<parameter_selector_response>(o), R"({"Selectors":[{"Device":{"Any":false,"Device":{"Collection":3,"Slot":1},"WholeCollection":true},"Name":"Hallo","ParameterId":123,"Type":2}],"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<parameter_selector_response>(R"({"Status": "INTERNAL_ERROR"})");
        EXPECT_EQ(to_ipc_string<parameter_selector_response>(o), R"({"Status":"INTERNAL_ERROR"})");
    }
    {
        auto o = from_ipc_string<value_response>(R"({"Status": "SUCCESS", "Value": {"Value": 123, "Type": 5}})");
        EXPECT_EQ(o.value->get_uint32(), 123);
        EXPECT_EQ(to_ipc_string<value_response>(o), R"({"Status":"SUCCESS","Value":{"Type":5,"Value":123}})");
    }
    {
        auto o = value_response();
        EXPECT_EQ(to_ipc_string<value_response>(o), R"({"Status":"NO_ERROR_YET"})");
    }
    {
        auto o1 = value_response();
        o1.set_error(status_codes::success, "evil");
        auto s1 = to_ipc_string<value_response>(o1);
        auto o2 = from_ipc_string<value_response>(s1);
        EXPECT_EQ(o1.status, o2.status);
        EXPECT_EQ(o1.message, o2.message);
    }
    {
        auto o = from_ipc_string<value_response>(R"({"Status":"PARAMETER_VALUE_UNAVAILABLE","Code":34,"Message":"Bad stuff"})");
        EXPECT_EQ(to_ipc_string<value_response>(o), R"({"Code":34,"Message":"Bad stuff","Status":"PARAMETER_VALUE_UNAVAILABLE"})");
    }
    {
        auto o = from_ipc_string<value_response>(R"({"Status": "NO_ERROR_YET", "Value": {"Value": 123, "Type": 5}})");
        EXPECT_EQ(o.value->get_uint32(), 123);
        EXPECT_EQ(to_ipc_string<value_response>(o), R"({"Status":"NO_ERROR_YET","Value":{"Type":5,"Value":123}})");
    }
    {
        auto o1 = set_parameter_response();
        o1.set_deferred();
        auto s1 = to_ipc_string<set_parameter_response>(o1);
        printf("STATUSCODE TO_STRING >>>>> %s", s1.c_str());
        auto o2 = from_ipc_string<set_parameter_response>(s1);
        EXPECT_EQ(o1.status, o2.status);
        EXPECT_EQ(o2.status, status_codes::wda_connection_changes_deferred);
    }

    {
        auto o1 = value_request(parameter_instance_id(1,3,device_id(7, 5)), parameter_value::create_uint16(123));
        auto s1 = to_ipc_string<value_request>(o1);
        auto o2 = from_ipc_string<value_request>(s1);
        EXPECT_EQ(o2.param_id, parameter_instance_id(1,3,device_id(7, 5)));
        EXPECT_EQ(o1.value->get_type(), o2.value->get_type());
        EXPECT_EQ(o2.value->get_uint16(), 123);
    }
    {
        auto o1 = value_path_request(parameter_instance_path("hallo","2-2"), parameter_value::create_uint16(123));
        auto s1 = to_ipc_string<value_path_request>(o1);
        auto o2 = from_ipc_string<value_path_request>(s1);
        EXPECT_EQ(o2.param_path, parameter_instance_path("hallo","2-2"));
        EXPECT_EQ(o1.value->get_type(), o2.value->get_type());
        EXPECT_EQ(o2.value->get_uint16(), 123);
    }

    {
        auto o1 = parameter_response(status_codes::success);
        o1.id = parameter_instance_id(1,3,device_id(7, 5));
        o1.path = parameter_instance_path("hallo","2-2");
        o1.value = parameter_value::create_uint16(123);
        auto s1 = to_ipc_string<parameter_response>(o1);
        auto o2 = from_ipc_string<parameter_response>(s1);
        EXPECT_EQ(o2.status, o1.status);
        EXPECT_EQ(o2.id, o1.id);
        EXPECT_EQ(o2.path, o1.path);
        EXPECT_EQ(o2.value->get_type(), o1.value->get_type());
        EXPECT_EQ(o2.value->get_uint16(), 123);
    }
    {
        auto o1 = parameter_response(status_codes::parameter_not_provided);
        o1.id = parameter_instance_id(1,3,device_id(7, 5));
        o1.path = parameter_instance_path("hallo","2-2");
        auto s1 = to_ipc_string<parameter_response>(o1);
        auto o2 = from_ipc_string<parameter_response>(s1);
        EXPECT_EQ(o2.status, o1.status);
        EXPECT_EQ(o2.id, o1.id);
        EXPECT_EQ(o2.path, o1.path);
        EXPECT_EQ(o2.value, nullptr);
    }
    {
        auto o1 = parameter_response(status_codes::parameter_value_unavailable);
        o1.message = "Hallo";
        o1.domain_specific_status_code = 42;
        o1.id = parameter_instance_id(1,3,device_id(7, 5));
        o1.path = parameter_instance_path("hallo","2-2");
        auto s1 = to_ipc_string<parameter_response>(o1);
        auto o2 = from_ipc_string<parameter_response>(s1);
        EXPECT_EQ(o2.status, o1.status);
        EXPECT_EQ(o2.id, o1.id);
        EXPECT_EQ(o2.path, o1.path);
        EXPECT_EQ(o2.value, nullptr);
        EXPECT_EQ(o2.message, o1.message);
        EXPECT_EQ(o2.domain_specific_status_code, o1.domain_specific_status_code);
    }
    {
        auto o1 = parameter_response_list_response();
        o1.status = status_codes::success;
        auto pr = parameter_response();
        pr.id = parameter_instance_id(1,3,device_id(7, 5));
        pr.path = parameter_instance_path("hallo","2-2");
        pr.value = parameter_value::create_uint16(123);
        o1.param_responses.push_back(pr);
        
        auto s1 = to_ipc_string<parameter_response_list_response>(o1);
        auto o2 = from_ipc_string<parameter_response_list_response>(s1);
        EXPECT_EQ(o2.status, o1.status);
        EXPECT_EQ(o2.param_responses[0].id, pr.id);
        EXPECT_EQ(o2.param_responses[0].path, pr.path);
        EXPECT_EQ(o2.param_responses[0].value->get_uint16(), 123);
    }
    
    {
        auto o = from_ipc_string<method_invocation_response>(R"({"Status": "SUCCESS", "Out": [{"Type": 5, "Value":12}, {"Type":1,"Value":"Hallo"}, {"Type":5,"Rank":1,"Value":[1,2,3]}]})");
        EXPECT_EQ(to_ipc_string<method_invocation_response>(o), R"({"Out":[{"Type":5,"Value":12},{"Type":1,"Value":"Hallo"},{"Rank":1,"Type":5,"Value":[1,2,3]}],"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<method_invocation_response>(R"({"Status": "INVALID_VALUE", "Code": 34, "Message": "Dat schlecht"})");
        EXPECT_EQ(to_ipc_string<method_invocation_response>(o), R"({"Code":34,"Message":"Dat schlecht","Status":"INVALID_VALUE"})");
    }
    {
        auto o = from_ipc_string<method_invocation_named_response>(R"({"Status": "SUCCESS", "Out": {"A": {"Type": 5, "Value":12}, "B":{"Type":1,"Value":"Hallo"}, "C": {"Type":5,"Rank":1,"Value":[1,2,3]}}})");
        EXPECT_EQ(to_ipc_string<method_invocation_named_response>(o), R"({"Out":{"A":{"Type":5,"Value":12},"B":{"Type":1,"Value":"Hallo"},"C":{"Rank":1,"Type":5,"Value":[1,2,3]}},"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<method_invocation_named_response>(R"({"Status": "INVALID_VALUE", "Code": 34, "Message": "Bad stuff"})");
        EXPECT_EQ(to_ipc_string<method_invocation_named_response>(o), R"({"Code":34,"Message":"Bad stuff","Status":"INVALID_VALUE"})");
    }
    {
        auto o = from_ipc_string<set_parameter_response>(R"({"Status": "INVALID_VALUE", "Code": 34, "Message": "Schlecht"})");
        EXPECT_EQ(to_ipc_string<set_parameter_response>(o), R"({"Code":34,"Message":"Schlecht","Status":"INVALID_VALUE"})");
    }
    {
        auto o = from_ipc_string<device_selector_response>(R"({"Status": "SUCCESS", "Selectors": [{"Any": true, "WholeCollection": true, "Device": {"Slot": 1, "Collection": 3}}]})");
        EXPECT_EQ(to_ipc_string<device_selector_response>(o), R"({"Selectors":[{"Any":true,"Device":{"Collection":3,"Slot":1},"WholeCollection":true}],"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<wdd_response>(R"({"Status": "SUCCESS", "Content": "Hallo"})");
        EXPECT_EQ(to_ipc_string<wdd_response>(o), R"({"Content":"Hallo","Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<wdm_response>(R"({"Status": "SUCCESS", "Content": "Hallo"})");
        EXPECT_EQ(to_ipc_string<wdm_response>(o), R"({"Content":"Hallo","Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<device_extension_response>(R"({"Status": "SUCCESS", "Selector": {"Any":true,"Device":{"Collection":2,"Slot":4},"WholeCollection":true}, "Features": ["Test"]})");
        EXPECT_EQ(to_ipc_string<device_extension_response>(o), R"({"Features":["Test"],"Selector":{"Any":true,"Device":{"Collection":2,"Slot":4},"WholeCollection":true},"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<file_read_response>(R"({"Status": "SUCCESS", "Data": [1,2,3]})");
        EXPECT_EQ(to_ipc_string<file_read_response>(o), R"({"Data":[1,2,3],"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<file_info_response>(R"({"Status": "SUCCESS", "Size": 123})");
        EXPECT_EQ(to_ipc_string<file_info_response>(o), R"({"Size":123,"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<device_collection_response>(R"({"Status": "SUCCESS", "Devices": [{"Status": "SUCCESS","Id": {"Slot": 1, "Collection": 2}, "OrderNumber": "Hallo", "FirmwareVersion": "01.01.01"}]})");
        EXPECT_EQ(to_ipc_string<device_collection_response>(o), R"({"Devices":[{"FirmwareVersion":"01.01.01","Id":{"Collection":2,"Slot":1},"OrderNumber":"Hallo","Status":"SUCCESS"}],"Status":"SUCCESS"})");
    }
    {
        auto o = from_ipc_string<device_extension_response>(R"({"Status": "SUCCESS", "Selector": {"Any":true,"Device":{"Collection":2,"Slot":4},"WholeCollection":true}, "Features": ["Test"]})");
        auto s = to_bytes<device_extension_response>(o, serialization_method::BINARY);
        auto o2 = from_bytes<device_extension_response>(s, serialization_method::BINARY);
        EXPECT_EQ(o.status, o2.status);
        EXPECT_EQ(o.extension_features, o2.extension_features);
    }
    {
        auto o1 = parameter_filter::only_device(device_selector::all_of(device_collections::rlb)) | parameter_filter::only_usersettings() | parameter_filter::without_methods();
        auto s1 = to_ipc_string<parameter_filter>(o1);
        auto o2 = from_ipc_string<parameter_filter>(s1);
        EXPECT_EQ(o2._only_usersettings, true);
        EXPECT_EQ(o2._without_methods, true);
        EXPECT_EQ(o2._device.is_collection_selector(), true);
        EXPECT_EQ(o2._device.get_selected_device().device_collection_id, device_collections::rlb);
    }
}

TEST_F(parameter_service_test_fixture, enums) {
    {
        auto r = service->get_all_enum_definitions().get();
        EXPECT_EQ(r.size(), 2);
    }
    {
        auto r = service->get_enum_definition("ipsources").get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_NE(r.definition, nullptr);
        EXPECT_EQ(r.definition->name, "IPSources");
    }
}

TEST_F(parameter_service_test_fixture, features) {
    {
        auto r = service->get_features_of_all_devices().get();
        EXPECT_EQ(r.size(), 2);
        auto& r0 = r[0];
        EXPECT_EQ(r0.device_path, "0-0");
        EXPECT_EQ(r0.status, status_codes::success);
        EXPECT_EQ(r0.features.size(), 16);
        EXPECT_TRUE(contains<device_feature_information>(r0.features, [](auto& el) {
            if(el.name != "BetaFeature")
                return false;
            EXPECT_TRUE(el.is_beta);
            EXPECT_EQ(el.includes.size(), 1);
            EXPECT_FALSE(el.parameter_definitions.empty());
            return true;
        }));
        EXPECT_TRUE(contains<device_feature_information>(r0.features, [](auto& el) {
            if(el.name != "DeprecatedFeature")
                return false;
            EXPECT_TRUE(el.is_deprecated);
            EXPECT_EQ(el.includes.size(), 1);
            EXPECT_FALSE(el.parameter_definitions.empty());
            return true;
        }));
        
        r0 = r[1];
        EXPECT_EQ(r0.device_path, "2-1");
        EXPECT_EQ(r0.status, status_codes::success);
        EXPECT_EQ(r0.features.size(), 5);
        EXPECT_TRUE(contains<device_feature_information>(r0.features, [](auto& el) { return el.name == "RocketLBT"; }));
    }
    {
        auto r = service->get_features({"2-1"}).get();
        EXPECT_EQ(r.size(), 1);
        auto& r0 = r[0];
        EXPECT_EQ(r0.status, status_codes::success);
        EXPECT_EQ(r0.device_path, "2-1");
        EXPECT_EQ(r0.features.size(), 5);
    }
    {
        auto r = service->get_feature_definition("0-0", "BetaFeature").get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_TRUE(r.feature.is_beta);
        EXPECT_EQ(r.feature.includes.size(), 1);
        EXPECT_FALSE(r.feature.parameter_definitions.empty());
    }
    {
        auto r = service->get_feature_definition("0-0", "DeprecatedFeature").get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_TRUE(r.feature.is_deprecated);
        EXPECT_EQ(r.feature.includes.size(), 1);
        EXPECT_FALSE(r.feature.parameter_definitions.empty());
    }
    {
        auto r = service->get_feature_definition("0-0", "NotExistingFeatureName").get();
        EXPECT_EQ(r.status, status_codes::unknown_feature_name);
    }
    {
        auto r = service->get_feature_definition("0-0", "RocketLBT").get();
        EXPECT_EQ(r.status, status_codes::feature_not_available);
    }
}

struct fragments_test_fixture : public ::testing::Test
{
private:
    std::unique_ptr<mock_permissions> permissions_mock_ptr = std::make_unique<mock_permissions>();

public:
    mock_permissions &permissions_mock = *permissions_mock_ptr;
    unique_ptr<parameter_service_i> service;

    // set up the test fixture.    
    virtual void SetUp() override 
    {
        cout << "Setting up the Test Fixure" << endl;
        current_log_mode = strict;
        service = std::make_unique<parameter_service_core>(std::move(permissions_mock_ptr));
        ASSERT_EQ(true, service != nullptr) << "Create Service failed!";

        permissions_mock.set_default_expectations();
    }

    // Tears down the test fixture.
    virtual void TearDown() override 
    {
        cout << "Tearing down the Test Fixure" << endl;
    }
};

TEST_F(fragments_test_fixture, unavailable_values) {
    autarc_parameter_provider app;
    {
        auto r = service->register_model_provider(&app).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(&app).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_parameter_provider(&app).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_devices({register_device_request{device_id::headstation, "0763-1508", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("UnavailableStatusParam1")}).get();
        EXPECT_EQ(r[0].status, status_codes::status_value_unavailable);
        EXPECT_EQ(r[0].get_message(), "SomeReason");
    }
    current_log_mode = lax;
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("UnavailableUserSetting1")}).get();
        EXPECT_EQ(r[0].status, status_codes::internal_error);
        EXPECT_TRUE(r[0].get_message().empty());
    }
    current_log_mode = strict;
}

TEST_F(fragments_test_fixture, register_ordering)
{
    auto p = autarc_parameter_provider();
   
    current_log_mode = lax;
    {
        auto r = service->register_device_description_providers({ &p}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_devices({register_device_request{device_id::headstation, "0763-1508", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_parameter_providers({ &p }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_model_providers({ &p }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    current_log_mode = strict;

    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param1")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "MeinWert");
    }
}

TEST_F(fragments_test_fixture, model_providers)
{
    auto p1 = model_provider1();
    auto p2 = model_provider2();
    auto p3 = clueless_model_provider();
   
    {
        auto r = service->register_devices({register_device_request{device_id::headstation, "0763-1508", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    current_log_mode = lax;
    {
        auto r = service->register_device_description_providers({ &p1, &p2, &p3 }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
    }
    {
        auto r = service->register_model_providers({ &p1, &p2, &p3 }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
    }
    current_log_mode = strict;
    {
        auto r = service->get_features({"0-0"}).get();
        EXPECT_EQ(r[0].features.size(), 3);
    }
    {
        auto r = service->get_all_parameters(device_selector::headstation()).get();
        EXPECT_EQ(r.param_responses.size(), 6);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param1"), parameter_instance_path("Param3"), parameter_instance_path("Param5")}).get();
        ASSERT_EQ(r.size(), 3);
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo");
        EXPECT_EQ(r[1].value->get_string(), "Wurst");
        EXPECT_EQ(r[2].value->get_string(), "Klappt");
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("TestClasses/1/Param2"), parameter_instance_path("TestClasses/1/Param4")}).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_not_provided);
        EXPECT_EQ(r[1].status, status_codes::parameter_not_provided);
    }
    current_log_mode = lax; // this shouldnt be necessary but model contains classes without features (this should be fixed sometime)
    auto p4 = new parameter_provider3();
    {
        auto r = service->register_model_providers({ p4 }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    current_log_mode = strict;
    {
        auto r = service->register_device_extension_providers({ p4 }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses")}).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_not_provided);
    }
    {
        auto r = service->register_parameter_providers({ p4 }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param7")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "EsGeht");
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 2);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses/1/Param8"),parameter_instance_path("DynClasses/2/Param8")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "DynParam1");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "DynParam2");
    }
    {
        auto r = service->get_features({"0-0"}).get();
        EXPECT_EQ(r[0].features.size(), 4);
    }
    {
        auto r = service->get_all_parameters(device_selector::headstation()).get();
        EXPECT_EQ(r.param_responses.size(), 10);
        // 8-0      "Param1" 
        // 2-1      "TestClasses/1/Param2"
        // 11231-0  "TestClasses"
        // 3-0      "Param3"
        // 5-0      "Param5"
        // 4-1      "TestClasses/1/Param4"
        // 7-0      "Param7"
        // 10017-0  "DynClasses"
        // 8-1      "DynClasses/1/Param8"
        // 8-2      "DynClasses/2/Param8"
    }

    {
        auto r = service->get_all_parameters(device_selector::headstation(), 1, 255).get();
        EXPECT_EQ(r.param_responses.size(), 9);
    }

    {
        service->unregister_device_description_providers({ &p1, &p2, &p3 });
        service->unregister_device_extension_providers({ p4 });
        service->unregister_parameter_providers({ p4 });
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses")}).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_not_provided);
    }
    {
        service->unregister_model_providers({ &p1, &p2, &p3, p4 });
        current_log_mode = lax;
        service->unregister_model_providers({ &p1, &p2, &p3, p4 });
        current_log_mode = strict;
    }
    {
        auto r = service->register_devices({register_device_request{device_id(1,1), "0763-1508", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_features({"1-1"}).get();
        EXPECT_EQ(r[0].features.size(), 0);
    }

    delete p4;
    p4 = new parameter_provider3(); // testing behavior when a parameter_provider goes out of scope

    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses/1/Param8"),parameter_instance_path("DynClasses/2/Param8")}).get();
        EXPECT_EQ(r[0].status, status_codes::parameter_not_provided);
        EXPECT_EQ(r[1].status, status_codes::parameter_not_provided);
    }

    // not treating warnings as errors for the following part, because we expect there to be warnings
    current_log_mode = lax;

    {
        auto r = service->register_device_extension_providers({ p4 }).get(); // atm, this generates a warning because information from extension_providers is not cleaned up yet. Later, this should not generate a warning.
        EXPECT_EQ(r[0].status, status_codes::success);
    }

    current_log_mode = strict;

    {
        auto r = service->register_parameter_providers({ p4 }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_features({"1-1"}).get();
        EXPECT_EQ(r[0].features.size(), 1);
    }
    {
        auto r = service->get_all_parameters(device_selector::specific(device_id(1,1))).get();
        EXPECT_EQ(r.param_responses.size(), 4);
        // 7-0      "Param7"
        // 10017-0  "DynClasses"
        // 8-1      "DynClasses/1/Param8"
        // 8-2      "DynClasses/2/Param8"
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param7", "1-1")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "EsGeht");
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 2);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses/1/Param8"),parameter_instance_path("DynClasses/2/Param8")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "DynParam1");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "DynParam2");
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("DynClasses", "1-1")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 2);
    }
    {
        auto r = service->get_parameter_providers({parameter_instance_path("DynClasses")});
        EXPECT_EQ(r[0], p4);
    }

    delete p4;
}

TEST_F(fragments_test_fixture, partial_model_providers) {
    partial_model_provider1 p1;
    partial_model_provider2 p2;
    partial_description_provider p3;

    {
        auto r = service->register_model_provider(&p1).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(&p3).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    current_log_mode = lax;
    {
        auto r = service->register_device(register_device_request{device_id(0,0), "0763-1508", "01.00.00"}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param1"), parameter_instance_path("Param2")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo");
        EXPECT_EQ(r[1].status, status_codes::unknown_parameter_path);
    }
    {
        auto r = service->register_model_provider(&p2).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param1"), parameter_instance_path("Param2")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "Du");
    }
    current_log_mode = strict;
}

TEST_F(fragments_test_fixture, builtin_parameters)
{
    mock_model_provider model_provider_mock;
    model_provider_mock.set_default_expectations();
    wago::wdx::wdm_response wdm(test_wdm);
    EXPECT_CALL(model_provider_mock, get_model_information())
    .WillOnce(testing::Return(testing::ByMove(wago::resolved_future<wago::wdx::wdm_response>(std::move(wdm)))));


    service->register_model_providers({ &model_provider_mock }).get();
    {
        auto r = service->register_devices({register_device_request{device_id::headstation, "0750-1111", "01.02.03"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Identity/OrderNumber"), parameter_instance_path("Version/SoftwareVersion")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "0750-1111");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "01.02.03");
    }
}

TEST_F(fragments_test_fixture, configfiles)
{
    configfile_parameter_provider fpp(service.get());

    service->register_model_provider(&fpp);
    service->register_device_extension_provider(&fpp);
    service->register_parameter_provider(&fpp, parameter_provider_call_mode::serialized);
    {
        auto r = service->register_devices({register_device_request{device_id::headstation, "0750-1111", "01.02.03"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->create_parameter_upload_id(parameter_instance_path("NormalParam")).get();
        EXPECT_EQ(r.status, status_codes::not_a_file_id);
    }
    file_id fid;
    {
        auto r = service->create_parameter_upload_id(parameter_instance_path("ConfigFileParam"), 1).get();
        EXPECT_EQ(r.status, status_codes::success);
        fid = r.registered_file_id;
        auto content = service->file_read(fid, 3, 7).get();
        EXPECT_EQ(content.status, status_codes::success);
        EXPECT_EQ(content.data[0], 3);
        EXPECT_EQ(content.data[1], 7);
    }
    // testing timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    {
        auto content = service->file_read(fid, 3, 7).get(); // refreshes the timeout
        EXPECT_EQ(content.status, status_codes::success);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    {
        auto content = service->file_read(fid, 3, 7).get(); // refreshes the timeout
        EXPECT_EQ(content.status, status_codes::success);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    {
        auto content = service->file_read(fid, 3, 7).get(); // refreshes the timeout
        EXPECT_EQ(content.status, status_codes::success);
    }
    EXPECT_EQ(fpp.cleanup_called, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(2100));
    current_log_mode = lax;
    {
        auto content = service->file_read("somefid", 3, 7).get(); // any call to the file-api will trigger the cleanup
        EXPECT_EQ(content.status, status_codes::unknown_file_id);
    }
    EXPECT_EQ(fpp.cleanup_called, true);

    {
        auto r = service->create_parameter_upload_id(parameter_instance_path("ConfigFileParam"), 1).get();
        EXPECT_EQ(r.status, status_codes::success);
        fid = r.registered_file_id;
    }
    {
        auto f = service->cleanup();
        f.get();
    }
    {
        auto content = service->file_read(fid, 3, 7).get();
        EXPECT_EQ(content.status, status_codes::unknown_file_id);
        EXPECT_EQ(service->job_count, 0);
    }
}

TEST_F(fragments_test_fixture, dynamic_providers) {
    dynamic_provider dp;
    {
        auto r = service->register_model_provider(&dp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(&dp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_parameter_provider(&dp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device(register_device_request{device_id(0,0), "0763-1508", "01.00.00"}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::any).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.total_entries, 5);
        EXPECT_EQ(r.total_entries, r.param_responses.size());

        bool found0 = false;
        bool found1 = false;
        bool found2 = false;
        bool found3 = false;
        for(auto& p : r.param_responses) {
            if(p.path.parameter_path == "FeatureParam") {
                EXPECT_EQ(p.status, status_codes::success);
                EXPECT_EQ(p.value->get_string(), "Huhu");
                found0 = true;
            }
            else if(p.path.parameter_path == "Dynamics/1/ClassParam0") {
                EXPECT_EQ(p.status, status_codes::success);
                EXPECT_EQ(p.value->get_string(), "Haha1");
                found1 = true;
            }
            else if(p.path.parameter_path == "Dynamics/2/ClassParam0") {
                EXPECT_EQ(p.status, status_codes::success);
                EXPECT_EQ(p.value->get_string(), "Haha2");
                found2 = true;
            }
            else if(p.path.parameter_path == "Dynamics/2/ClassParam1") {
                EXPECT_EQ(p.status, status_codes::success);
                EXPECT_EQ(p.value->get_string(), "Hoho2");
                found3 = true;
            }
        }
        EXPECT_TRUE(found0);
        EXPECT_TRUE(found1);
        EXPECT_TRUE(found2);
        EXPECT_TRUE(found3);
    }
}

TEST_F(fragments_test_fixture, delayed_providers) {
    delayed_provider dp;
    current_log_mode = lax;
    {
        auto r = service->register_parameter_provider(&dp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_model_provider(&dp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(&dp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device(register_device_request{device_id(0,0), "0763-1508", "01.00.00"}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    current_log_mode = strict;
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Param1")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo");
    }
}

TEST_F(fragments_test_fixture, connection_aware_providers) {
    connection_aware_parameter_provider pp;
    autarc_parameter_provider ap;
    {
        auto r = service->register_model_providers({&pp,&ap}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
    }
    {
        auto r = service->register_device_description_providers({&pp,&ap}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
    }
    {
        auto r = service->register_parameter_providers({&pp,&ap}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
    }
    {
        auto r = service->register_device(register_device_request{device_id(0,0), "0763-1508", "01.00.00"}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("ConnectionChangingParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Startwert");
    }
    {
        
        auto r = service->set_parameter_values_by_path({
            value_path_request(parameter_instance_path("ConnectionChangingParam"), parameter_value::create("Hallo")),
            value_path_request(parameter_instance_path("NonConnectionChangingParam"), parameter_value::create("Hallo")),
            value_path_request(parameter_instance_path("WriteableParam1"), parameter_value::create("Hallo"))
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("ConnectionChangingParam"),
            parameter_instance_path("NonConnectionChangingParam")
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "HalloSuffix");
    }
    {
        auto r = service->set_parameter_values_by_path_connection_aware({
            value_path_request(parameter_instance_path("ConnectionChangingParam"), parameter_value::create("Hallo2")),
            value_path_request(parameter_instance_path("NonConnectionChangingParam"), parameter_value::create("Hallo2")),
            value_path_request(parameter_instance_path("WriteableParam1"), parameter_value::create("Hallo"))
            }, true).get();
        EXPECT_EQ(r[0].status, status_codes::wda_connection_changes_deferred);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_FALSE(r[0].has_error());
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("ConnectionChangingParam"),
            parameter_instance_path("NonConnectionChangingParam")
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "HalloSuffix");
    }
    {
        auto r = service->set_parameter_values_by_path_connection_aware({
            value_path_request(parameter_instance_path("ConnectionChangingParam"), parameter_value::create("Hallo2")),
            value_path_request(parameter_instance_path("NonConnectionChangingParam"), parameter_value::create("Hallo2")),
            value_path_request(parameter_instance_path("WriteableParam1"), parameter_value::create("Hallo"))
            }, false).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("ConnectionChangingParam"),
            parameter_instance_path("NonConnectionChangingParam")
            }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].value->get_string(), "Hallo2");
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].value->get_string(), "Hallo2Suffix");
    }
}

TEST_F(parameter_service_test_fixture, instances_parameter)
{
    // non-dynamic case
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("TestClasses")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->id, 2007);
        EXPECT_EQ(r[0].definition->path, "TestClasses");
        EXPECT_EQ(r[0].definition->value_type, parameter_value_types::instantiations);
        EXPECT_TRUE(r[0].value);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 2);
        EXPECT_EQ(r[0].value->get_instantiations()[1].id, 2);
        EXPECT_EQ(r[0].value->get_instantiations()[1].classes.size(), 2);
        EXPECT_EQ(r[0].value->get_instantiations()[1].classes[0], "TestBaseClass");
        EXPECT_EQ(r[0].value->get_instantiations()[1].classes[1], "TestClass");
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("TestClasses/2/OtherParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    // dynamic case
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Dynamics"), parameter_instance_path("Dynamics/1/Param0"), parameter_instance_path("Dynamics/1/Param1")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->id, 100001);
        EXPECT_EQ(r[0].definition->path, "Dynamics");
        EXPECT_EQ(r[0].definition->value_type, parameter_value_types::instantiations);
        EXPECT_TRUE(r[0].value);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 1);
        EXPECT_EQ(r[0].value->get_instantiations()[0].id, 1);
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicClass"));
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicSpecialClass"));
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_TRUE(r[1].value);
        EXPECT_EQ(r[1].value->get_string(), "Jep");
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[2].value->get_string(), "Blabla");
    }
    {
        auto r = service->get_parameters({parameter_instance_id(0,0,device_id::headstation)}).get();
        EXPECT_EQ(r[0].status, status_codes::unknown_parameter_id);
    }
    {
        auto r = service->set_parameter_values_by_path({
            {{"Dynamics/2/Param0"}, parameter_value::create("Mann")}, // purposely giving unfortunate order for parameter_provider, parameter_service should reorder
            {{"Dynamics"}, parameter_value::create_instantiations({class_instantiation(1, "DynamicClass", {"DynamicSpecialClass"}), class_instantiation(2, "DynamicClass", {"DynamicSpecialClass2"})})},
            {{"Dynamics/1/Param0"}, parameter_value::create("Ja")},
            {{"Dynamics/1/Param1"}, parameter_value::create("Ja")},
            {{"Dynamics/2/Param1"}, parameter_value::create("Ja")}
        }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[3].status, status_codes::success);
        EXPECT_EQ(r[4].status, status_codes::success);
    }
    {
        auto r = service->set_parameter_values_by_path({
            {{"Dynamics/2/Param0"}, parameter_value::create("Mann")}, // purposely giving unfortunate order for parameter_provider, parameter_service should reorder
            {{"Dynamics/2/Param1"}, parameter_value::create("Ja")},
            {{"Dynamics"}, parameter_value::create_instantiations({class_instantiation(1, "DynamicClass"), class_instantiation(2, "DynamicClass")})},
            {{"Dynamics/1/Param0"}, parameter_value::create("Ja")},
            {{"Dynamics/1/Param1"}, parameter_value::create("Ja")}
        }).get();
        EXPECT_EQ(r[0].status, status_codes::other_invalid_value_in_set);
        EXPECT_EQ(r[1].status, status_codes::not_existing_for_instance);
        EXPECT_EQ(r[2].status, status_codes::other_invalid_value_in_set);
        EXPECT_EQ(r[3].status, status_codes::other_invalid_value_in_set);
        EXPECT_EQ(r[4].status, status_codes::not_existing_for_instance);
    }
    {
        auto r = service->set_parameter_values_by_path({
            {{"Dynamics/2/Param1"}, parameter_value::create("Mann")}, // purposely giving unfortunate order for parameter_provider, parameter_service should reorder
            {{"Dynamics"}, parameter_value::create_instantiations({class_instantiation(1, "DynamicClass", {"DynamicSpecialClass"}), class_instantiation(2, "DynamicSpecialClass")})},
            {{"Dynamics/1/Param1"}, parameter_value::create("Ja")},
            {{"Dynamics/1/Param0"}, parameter_value::create("Ja")},
        }).get();
        EXPECT_EQ(r[0].status, status_codes::other_invalid_value_in_set) << "error message: " << r[0].message;
        EXPECT_EQ(r[1].status, status_codes::missing_parameter_for_instantiation) << "error message: " << r[1].message;
        EXPECT_EQ(r[2].status, status_codes::other_invalid_value_in_set) << "error message: " << r[2].message;
        EXPECT_EQ(r[3].status, status_codes::other_invalid_value_in_set) << "error message: " << r[3].message;
    }
    {
        auto r = service->set_parameter_values_by_path({
            {{"Dynamics/2/Param1"}, parameter_value::create("Mann")}, // purposely giving unfortunate order for parameter_provider, parameter_service should reorder
            {{"Dynamics"}, parameter_value::create_instantiations({class_instantiation(1, "DynamicClass", {"InvalidSpecialClass"}), class_instantiation(2, "DynamicSpecialClass2")})},
            {{"Dynamics/1/Param1"}, parameter_value::create("Ja")},
            {{"Dynamics/1/Param0"}, parameter_value::create("Ja")},
            {{"Dynamics/2/Param0"}, parameter_value::create("Ja")},
        }).get();
        EXPECT_EQ(r[0].status, status_codes::other_invalid_value_in_set) << "error message: " << r[0].message;
        EXPECT_EQ(r[1].status, status_codes::invalid_value)              << "error message: " << r[1].message;
        EXPECT_EQ(r[2].status, status_codes::other_invalid_value_in_set) << "error message: " << r[2].message;
        EXPECT_EQ(r[3].status, status_codes::other_invalid_value_in_set) << "error message: " << r[3].message;
        EXPECT_EQ(r[4].status, status_codes::other_invalid_value_in_set) << "error message: " << r[4].message;
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Dynamics"), parameter_instance_path("Dynamics/1/Param0"), parameter_instance_path("Dynamics/2/Param0")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->id, 100001);
        EXPECT_EQ(r[0].definition->path, "Dynamics");
        EXPECT_EQ(r[0].definition->value_type, parameter_value_types::instantiations);
        EXPECT_TRUE(r[0].value);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 2);
        EXPECT_EQ(r[0].value->get_instantiations()[0].id, 1);
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicClass"));
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicSpecialClass"));
        EXPECT_EQ(r[0].value->get_instantiations()[1].id, 2);
        EXPECT_TRUE(r[0].value->get_instantiations()[1].has_class("DynamicClass"));
        EXPECT_TRUE(r[0].value->get_instantiations()[1].has_class("DynamicSpecialClass"));
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_TRUE(r[1].value);
        EXPECT_EQ(r[1].value->get_string(), "Ja");
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_TRUE(r[2].value);
        EXPECT_EQ(r[2].value->get_string(), "Mann");
    }
    // same parameters but with numeric access
    {
        auto r = service->set_parameter_values({
            {parameter_instance_id{100001, 0}, parameter_value::create_instantiations({class_instantiation(1, "DynamicClass")})},
            {parameter_instance_id{20003, 1}, parameter_value::create("Alles zurück")},
        }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
    }
    {
        auto r = service->get_parameters({parameter_instance_id(100001, 0), parameter_instance_id(20003, 1)}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_TRUE(r[0].value);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 1);
        EXPECT_EQ(r[0].value->get_instantiations()[0].id, 1);
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicClass"));
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_TRUE(r[1].value);
        EXPECT_EQ(r[1].value->get_string(), "Alles zurück");
    }

    // testing polymorphic dynamic instances
    {
        auto r = service->set_parameter_values_by_path({
            {{"Dynamics"}, parameter_value::create_instantiations({class_instantiation(1, "DynamicSpecialClass"), class_instantiation(2, "DynamicSpecialClassB")})},
            {{"Dynamics/1/Param0"}, parameter_value::create("WertA")},
            {{"Dynamics/1/Param1"}, parameter_value::create("KuckuckA")},
            {{"Dynamics/2/Param0"}, parameter_value::create("WertB")},
            {{"Dynamics/2/Param2"}, parameter_value::create("KuckuckB")},
        }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[3].status, status_codes::success);
        EXPECT_EQ(r[4].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("Dynamics"),
            parameter_instance_path("Dynamics/1/Param0"),
            parameter_instance_path("Dynamics/1/Param1"),
            parameter_instance_path("Dynamics/2/Param0"),
            parameter_instance_path("Dynamics/2/Param1"),
        }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->id, 100001);
        EXPECT_EQ(r[0].definition->path, "Dynamics");
        EXPECT_EQ(r[0].definition->value_type, parameter_value_types::instantiations);
        ASSERT_TRUE(r[0].value);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 2);
        EXPECT_EQ(r[0].value->get_instantiations()[0].id, 1);
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicClass"));
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicSpecialClass"));
        EXPECT_EQ(r[0].value->get_instantiations()[1].id, 2);
        EXPECT_TRUE(r[0].value->get_instantiations()[1].has_class("DynamicClass"));
        EXPECT_TRUE(r[0].value->get_instantiations()[1].has_class("DynamicSpecialClassB"));
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_TRUE(r[1].value);
        EXPECT_EQ(r[1].value->get_string(), "WertA");
        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_TRUE(r[2].value);
        EXPECT_EQ(r[2].value->get_string(), "KuckuckA");
        EXPECT_EQ(r[3].status, status_codes::success);
        EXPECT_TRUE(r[3].value);
        EXPECT_EQ(r[3].value->get_string(), "WertB");
        EXPECT_EQ(r[4].status, status_codes::success);
        EXPECT_TRUE(r[4].value);
        EXPECT_EQ(r[4].value->get_string(), "KuckuckB");
    }
    {
        auto r = service->get_parameters_by_path({
            parameter_instance_path("DynamicStuff/DynamicReadonly"),
            parameter_instance_path("DynamicStuff/DynamicReadonly/1/SomeLevel/StatusParam"),
            parameter_instance_path("DynamicStuff/DynamicReadonly/1/SomeLevel/Action"),
            parameter_instance_path("DynamicStuff/DynamicReadonly/2/SomeLevel/StatusParam"),
            parameter_instance_path("DynamicStuff/DynamicReadonly/2/SomeLevel/Action"),
            parameter_instance_path("DynamicStuff/DynamicReadonly/1001/SomeLevel/StatusParam"),
            parameter_instance_path("DynamicStuff/DynamicReadonly/1001/SomeLevel/Action"),
        }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->id, 100002);
        EXPECT_EQ(r[0].definition->path, "DynamicStuff/DynamicReadonly");
        EXPECT_EQ(r[0].definition->value_type, parameter_value_types::instantiations);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 3);
        EXPECT_EQ(r[0].value->get_instantiations()[0].id, 1);
        EXPECT_EQ(r[0].value->get_instantiations()[1].id, 1001);
        EXPECT_EQ(r[0].value->get_instantiations()[2].id, 1002);
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicReadOnlyClass"));
        EXPECT_EQ(r[1].status, status_codes::parameter_value_unavailable);
        EXPECT_EQ(r[2].status, status_codes::methods_do_not_have_value);
        EXPECT_EQ(r[2].definition->value_type, parameter_value_types::method);
        EXPECT_EQ(r[3].status, status_codes::parameter_value_unavailable);
        EXPECT_EQ(r[4].status, status_codes::methods_do_not_have_value);
        EXPECT_EQ(r[5].status, status_codes::parameter_value_unavailable);
        EXPECT_EQ(r[6].status, status_codes::methods_do_not_have_value);
    }

    current_log_mode = lax;

    // get_all_parameters
    {
        auto r = service->get_all_parameters(parameter_filter::any).get();
        EXPECT_EQ(r.status, status_codes::success);
        parameter_response inst_param;
        parameter_response inst1param0;
        parameter_response inst1param1;
        parameter_response inst2param0;
        parameter_response inst2param1;
        parameter_response inst2param2;
        parameter_response roinst_param;
        parameter_response roinst1param0;
        parameter_response roinst1param1;
        parameter_response roinst2param0;
        parameter_response roinst2param1;
        for(auto& r : r.param_responses) {
            if(r.path.parameter_path == "Dynamics")
                inst_param = r;
            else if(r.path.parameter_path == "Dynamics/1/Param0")
                inst1param0 = r;
            else if(r.path.parameter_path == "Dynamics/1/Param1")
                inst1param1 = r;
            else if(r.path.parameter_path == "Dynamics/2/Param0")
                inst2param0 = r;
            else if(r.path.parameter_path == "Dynamics/2/Param1")
                inst2param1 = r;
            else if(r.path.parameter_path == "Dynamics/2/Param2")
                inst2param2 = r;
            else if(r.path.parameter_path == "DynamicStuff/DynamicReadonly")
                roinst_param = r;
            else if(r.path.parameter_path == "DynamicStuff/DynamicReadonly/1/SomeLevel/StatusParam")
                roinst1param0 = r;
            else if(r.path.parameter_path == "DynamicStuff/DynamicReadonly/1/SomeLevel/Action")
                roinst1param1 = r;
            else if(r.path.parameter_path == "DynamicStuff/DynamicReadonly/1001/SomeLevel/StatusParam")
                roinst2param0 = r;
            else if(r.path.parameter_path == "DynamicStuff/DynamicReadonly/1001/SomeLevel/Action")
                roinst2param1 = r;
        }
        EXPECT_TRUE(inst_param.is_determined());
        EXPECT_TRUE(inst1param0.is_determined());
        EXPECT_TRUE(inst1param1.is_determined());
        EXPECT_TRUE(inst2param0.is_determined());
        EXPECT_FALSE(inst2param1.is_determined());
        EXPECT_TRUE(inst2param2.is_determined());
        EXPECT_TRUE(roinst_param.is_determined());
        EXPECT_TRUE(roinst1param0.is_determined());
        EXPECT_FALSE(roinst1param1.is_determined());
        EXPECT_TRUE(roinst2param0.is_determined());
        EXPECT_FALSE(roinst2param1.is_determined());
    }

    current_log_mode = strict;

    // device specific override of dynamic class: static instantiations
    description_provider1 dp;
    {
        auto r = service->register_device_description_providers({&dp}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_devices({register_device_request{device_id(4, device_collections::rlb), "0999-0999", "01.01.01"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Dynamics", "2-4"), parameter_instance_path("Dynamics/1/StaticParam3", "2-4")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->value_type, parameter_value_types::instantiations);
        EXPECT_TRUE(r[0].value);
        EXPECT_EQ(r[0].value->get_instantiations().size(), 1);
        EXPECT_EQ(r[0].value->get_instantiations()[0].id, 1);
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicClass"));
        EXPECT_TRUE(r[0].value->get_instantiations()[0].has_class("DynamicSpecialClassB"));
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_TRUE(r[1].value);
        EXPECT_EQ(r[1].value->get_string(), "Vorgabewert1");
    }
}

TEST_F(parameter_service_test_fixture, beta_flag) {
    {
        auto r = service->get_parameter_definitions_by_path({{parameter_instance_path("Test/BetaParam"), parameter_instance_path("BetaClasses/1/BetaClassParam"), parameter_instance_path("Test/ProvidedParam")}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->is_beta, true);

        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].definition->is_beta, true);

        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[2].definition->is_beta, false);
    }
    {
        auto r = service->get_device(device_id::headstation).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.has_beta_parameters, true);
    }
    {
        auto r = service->get_device(device_id(1, 2)).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.has_beta_parameters, false);
    }
}

TEST_F(parameter_service_test_fixture, deprecated_flag) {
    {
        auto r = service->get_parameter_definitions_by_path({{parameter_instance_path("Test/DeprecatedParam"),
                                                              parameter_instance_path("DeprecatedClasses/1/DeprecatedClassParam"),
                                                              parameter_instance_path("NotDeprecatedClasses/1/NotDeprecatedClassParam"),
                                                              parameter_instance_path("Test/ProvidedParam")}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[0].definition->is_deprecated, true);

        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[1].definition->is_deprecated, false); // FIXME: This should be true when classes are markable as deprecated

        EXPECT_EQ(r[2].status, status_codes::success);
        EXPECT_EQ(r[2].definition->is_deprecated, false);

        EXPECT_EQ(r[3].status, status_codes::success);
        EXPECT_EQ(r[3].definition->is_deprecated, false);
    }
    {
        auto r = service->get_device(device_id::headstation).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.has_deprecated_parameters, true);
    }
    {
        auto r = service->get_device(device_id(1, 2)).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.has_deprecated_parameters, false);
    }
}

TEST_F(parameter_service_test_fixture, file_api)
{
    {
        auto r = service->file_read("file1", 10, 3).get();
        EXPECT_EQ(r.status, status_codes::unknown_file_id);
    }
    file_provider fp;
    file_id fid;
    {
        auto r = service->register_file_provider(&fp, 1).get();
        EXPECT_EQ(r.status, status_codes::success);
        fid = r.registered_file_id;
    }
    {
        auto r = service->file_read(fid, 10, 3).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.data[0], 10);
        EXPECT_EQ(r.data[1], 3);
    }
    {
        auto r = service->file_write(fid, 10, {1, 2, 3}).get();
        EXPECT_EQ(r.status, status_codes::not_implemented);
    }
    {
        auto r = service->file_get_info(fid).get();
        EXPECT_EQ(r.status, status_codes::not_implemented);
    }
    {
        auto r = service->file_create(fid, 1000).get();
        EXPECT_EQ(r.status, status_codes::not_implemented);
    }
    {
        service->unregister_file_provider(&fp);
        auto r = service->file_read(fid, 10, 3).get();
        EXPECT_EQ(r.status, status_codes::unknown_file_id);
    }
    {
        auto r = service->register_file_provider(&fp, 1).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_NE(r.registered_file_id, fid);
    }
    {
        service->unregister_file_provider(&fp);
        auto r = service->reregister_file_provider(&fp, fid, 1).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.registered_file_id, fid);
    }
    {
        auto r = service->file_read(fid, 10, 3).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.data[0], 10);
        EXPECT_EQ(r.data[1], 3);
    }
    {
        current_log_mode = lax;
        auto r = service->reregister_file_provider(&fp, fid, 1).get();
        EXPECT_EQ(r.status, status_codes::provider_not_operational);
        current_log_mode = strict;
    }
    {
        service->unregister_file_provider(&fp);
        auto r = service->file_read(fid, 10, 3).get();
        EXPECT_EQ(r.status, status_codes::unknown_file_id);
    }
    file_parameter_provider fpp(service.get());
    {
        auto r = service->register_parameter_providers({&fpp}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Test/FileParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        fid = r[0].value->get_file_id();
    }
    {
        auto r = service->file_read(fid, 9, 5).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.data[0], 9);
        EXPECT_EQ(r.data[1], 5);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Test/FileParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        fid = r[0].value->get_file_id();
    }
    {
        auto r = service->file_read(fid, 2, 4).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.data[0], 2);
        EXPECT_EQ(r.data[1], 4);
    }
    current_log_mode = lax;
    {
        auto r = service->get_all_parameters(parameter_filter::only_file_ids()).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_EQ(r.total_entries, 1);
        EXPECT_EQ(r.param_responses.at(0).value->get_type(), parameter_value_types::file_id);
    }
    {
        auto r = service->get_all_parameters(parameter_filter::without_file_ids()).get();
        EXPECT_EQ(r.status, status_codes::success);
        EXPECT_GT(r.param_responses.size(), 0);
        for(auto& p : r.param_responses) {
            if(p.has_error()) continue;
            EXPECT_NE(p.value->get_type(), parameter_value_types::file_id);
        }
    }
}

TEST_F(parameter_service_test_fixture, asynch) {
    auto pp = make_shared<async_pp>();
    {
        auto r = service->register_model_providers({ pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_device_extension_providers({ pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_parameter_providers({ pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    bool passed = false;
    int notifications = 0;
    {
        auto f = service->get_parameters_by_path({ parameter_instance_path("AsyncParam") });
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r[0].status, status_codes::success);
            EXPECT_EQ(r[0].value->get_string(), "EsGeht");
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }
    {
        auto f = service->get_parameters_by_path({ parameter_instance_path("AsyncParam"), parameter_instance_path("AsyncErrorParam") });
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r[0].status, status_codes::internal_error);
            EXPECT_EQ(r[1].status, status_codes::internal_error);
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }

    // test the same for set_parameters
    {
        auto f = service->set_parameter_values_by_path({ value_path_request(parameter_instance_path("AsyncParam"), parameter_value::create("Hallo")) });
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r[0].status, status_codes::success);
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }

    // test the same for get_monitor_list_values
    monitoring_list_id_t mlid;
    {
        auto r = service->create_monitoring_list_with_paths({ parameter_instance_path("AsyncParam") }, 0).get();
        EXPECT_EQ(r.status, status_codes::success);
        mlid = r.monitoring_list.id;
    }
    {
        auto f = service->get_values_for_monitoring_list(mlid);
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r.status, status_codes::success);
            EXPECT_EQ(r.parameter_values[0].value->get_string(), "EsGeht");
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }

    // test the same for invoke_method
    {
        auto f = service->invoke_method_by_path(parameter_instance_path("AsyncMethod"), {{"In", parameter_value::create_uint8(24)}});
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r.status, status_codes::success);
            EXPECT_EQ(r.out_args["Out"]->get_uint16(), 48);
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }

    // not treating warnings as errors for the following part, because we expect there to be warnings
    current_log_mode = lax;

    // test the same for get_all_parameters
    {
        auto f = service->get_all_parameters(parameter_filter::any);
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r.status, status_codes::success);
            EXPECT_GT(r.param_responses.size(), 5);
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }

    {
        auto f = service->invoke_method_by_path(parameter_instance_path("AsyncMethodWithException"), {});
        f.set_notifier([&](auto r) {
            EXPECT_EQ(r.status, status_codes::internal_error);
            EXPECT_EQ(passed, true); // makes sure the notifier is really called after the service caller returns
            notifications++;
        });
    }

    passed = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // give the service time to collect results without being destroyed
    EXPECT_EQ(notifications, 7);
    EXPECT_EQ(service->job_count, 0);
}

TEST_F(parameter_service_test_fixture, serial_pp) {
    auto sensitive_pp = make_shared<serial_sensitive_parameter_provider>();
    {
        auto r = service->register_model_providers({ sensitive_pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_device_description_providers({ sensitive_pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_parameter_provider(sensitive_pp.get(), parameter_provider_call_mode::serialized).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    auto sensitive_fp = make_shared<serial_sensitive_file_provider>();
    file_id fid;
    {
        auto r = service->register_file_provider(sensitive_fp.get(), 1, provider_call_mode::serialized).get();
        EXPECT_EQ(r.status, status_codes::success);
        fid = r.registered_file_id;
    }

    auto th1 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(101));
        auto f = service->get_parameters_by_path({ parameter_instance_path("SerialSensitiveParameter") });
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r[0].status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });
    auto th2 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f = service->set_parameter_values_by_path({{ parameter_instance_path("SerialSensitiveParameter"), parameter_value::create_string("Neuer Wert")}});
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r[0].status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });
    auto th3 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f = service->invoke_method_by_path(parameter_instance_path("SerialSensitiveMethod"), {});
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r.status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });
    auto th4 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f = service->file_read(fid, 0, 3);
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r.status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });
    auto th5 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f = service->file_write(fid, 0, {2, 3});
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r.status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });
    auto th6 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f = service->file_get_info(fid);
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r.status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });
    auto th7 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto f = service->file_create(fid, 5);
        f.set_notifier([&](auto r) {
            ASSERT_EQ(r.status, status_codes::success);
        });
        EXPECT_FALSE(f.ready());
        f.wait();
    });

    th1.join();
    th2.join();
    th3.join();
    th4.join();
    th5.join();
    th6.join();
    th7.join();
    EXPECT_EQ(service->job_count, 0);

    {
        service->unregister_parameter_provider(sensitive_pp.get());
        service->unregister_file_provider(sensitive_fp.get());
    }
}

TEST_F(parameter_service_test_fixture, nonserial_pp) {
    auto sensitive_pp = make_shared<serial_sensitive_parameter_provider>();

    {
        auto r = service->register_model_providers({ sensitive_pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_device_description_providers({ sensitive_pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_parameter_provider(sensitive_pp.get()).get();
        EXPECT_EQ(r.status, status_codes::success);
    }

    current_log_mode = lax;

    auto exception_occurred = make_shared<std::atomic<int>>(0);

    auto th1 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(101));
        printf("Thread 1 started.\n");
        auto f = service->get_parameters_by_path({ parameter_instance_path("SerialSensitiveParameter") });
        f.set_notifier([&](auto r) {
            printf("Got result 1.\n");
            if(r[0].status == status_codes::internal_error)
                exception_occurred->fetch_add(1);
        });
        f.wait();
        printf("Thread 1 done.\n");
    });
    auto th2 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        printf("Thread 2 started.\n");
        auto f = service->set_parameter_values_by_path({{ parameter_instance_path("SerialSensitiveParameter"), parameter_value::create_string("Neuer Wert")}});
        f.set_notifier([&](auto r) {
            printf("Got result 2.\n");
            if(r[0].status == status_codes::internal_error)
                exception_occurred->fetch_add(1);
        });
        f.wait();
        printf("Thread 2 done.\n");
    });
    auto th3 = std::thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(90));
        printf("Thread 3 started.\n");
        auto f = service->invoke_method_by_path(parameter_instance_path("SerialSensitiveMethod"), {});
        f.set_notifier([&](auto r) {
            printf("Got result 3.\n");
            if(r.status == status_codes::internal_error)
                exception_occurred->fetch_add(1);
        });
        f.wait();
        printf("Thread 3 done.\n");
    });

    th1.join();
    th2.join();
    th3.join();

    EXPECT_EQ(*exception_occurred, 2);
    EXPECT_EQ(service->job_count, 0);
}

TEST_F(fragments_test_fixture, method_instances) {
    auto pp = make_shared<method_parameter_provider>();
    {
        auto r = service->register_model_providers({ pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(pp.get()).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_parameter_providers({ pp.get() }).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->register_devices({register_device_request{device_id::headstation, "0763-1508", "01.00.00"}}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
    }
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("ClassesWithMethod/3/DoSomething"), {}).get();
        EXPECT_EQ(r.status, status_codes::unknown_parameter_path);
    }
    {
        auto r = service->invoke_method_by_path(parameter_instance_path("ClassesWithMethod/1/DoSomething"), {}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
}

TEST_F(fragments_test_fixture, unavailable_via_wdd) {
    auto pp = dynamic_provider();
    {
        auto r = service->register_model_provider(&pp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device_description_provider(&pp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->register_device(register_device_request{device_id::headstation, "0763-1508", "01.00.00"}).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Dynamics"), parameter_instance_path("Dynamics/1/ClassParam1"), parameter_instance_path("FeatureParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::status_value_unavailable);
        EXPECT_EQ(r[1].status, status_codes::status_value_unavailable);
        EXPECT_EQ(r[2].status, status_codes::status_value_unavailable);
        EXPECT_FALSE(r[0].has_error());
    }
    {
        auto r = service->register_parameter_provider(&pp).get();
        EXPECT_EQ(r.status, status_codes::success);
    }
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Dynamics"), parameter_instance_path("Dynamics/1/ClassParam1"), parameter_instance_path("FeatureParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::success);
        EXPECT_EQ(r[1].status, status_codes::success);
        EXPECT_EQ(r[2].status, status_codes::success);
    }
    service->unregister_parameter_provider(&pp);
    {
        auto r = service->get_parameters_by_path({parameter_instance_path("Dynamics"), parameter_instance_path("Dynamics/1/ClassParam1"), parameter_instance_path("FeatureParam")}).get();
        EXPECT_EQ(r[0].status, status_codes::status_value_unavailable);
        EXPECT_EQ(r[1].status, status_codes::status_value_unavailable);
        EXPECT_EQ(r[2].status, status_codes::status_value_unavailable);
    }

}

// This test should be last because it makes your other tests waiting


TEST_F(parameter_service_test_fixture, monitoring_lists)
{
    monitoring_list_id_t monitor_id;
    {
        auto result = service->create_monitoring_list_with_paths({
            parameter_instance_path("Identity/OrderNumber"),
            parameter_instance_path("Channels/2/Value/IoValue", "2-1"),
            parameter_instance_path("Channels/3/Value/IoValue", "2-1"),
            parameter_instance_path("Channels/3/Value/IoValue", "2-2")}, 5).get();

        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_list.id, 1); //this actually tests for a certain way of assigning the monitoring_list_id
        monitor_id = result.monitoring_list.id;
    }

    {
        auto result = service->get_monitoring_list(monitor_id).get();
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_list.id, monitor_id);
        EXPECT_EQ(result.monitoring_list.timeout_seconds, 5);
    }

    {
        auto result = service->get_monitoring_list(815).get(); // should not exist
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

    {
        auto result = service->get_values_for_monitoring_list(815).get(); // should not exist
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

    {
        auto result = service->delete_monitoring_list(715).get(); // should not exist
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

    {
        auto result = service->get_values_for_monitoring_list(monitor_id).get();
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.parameter_values.size(), 4);
        auto& values = result.parameter_values;
        EXPECT_EQ(values[0].value->get_string(), "0768-3301");
        EXPECT_EQ(values[1].status, status_codes::parameter_not_provided);
        EXPECT_TRUE(values[1].definition);
        EXPECT_EQ(values[1].id.id, 1110);
        EXPECT_EQ(values[1].id.instance_id, 2);
        EXPECT_EQ(values[2].status, status_codes::parameter_not_provided);
        EXPECT_EQ(values[3].status, status_codes::unknown_device);
        EXPECT_EQ(values[3].path, parameter_instance_path("Channels/3/Value/IoValue", "2-2"));
    }

    {
        auto result = service->get_values_for_monitoring_list(monitor_id).get(); // should be same result
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.parameter_values.size(), 4);
    }

    {
        auto result = service->create_monitoring_list_with_paths({
            parameter_instance_path("Identity/OrderNumber"),
        }, 5).get();

        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_list.id, 2); //this actually tests for a certain way of assigning the monitoring_list_id
    }

    {
        auto result = service->create_monitoring_list_with_paths({
            parameter_instance_path("Identity/OrderNumber"),
        }, 5).get();

        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_list.id, 3); //this actually tests for a certain way of assigning the monitoring_list_id
    }

    {
        auto result = service->get_all_monitoring_lists().get();
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_lists.size(), 3);
    }

    {
        auto result = service->delete_monitoring_list(monitor_id).get();
        EXPECT_EQ(result.status, status_codes::success);
    }

    {
        auto result = service->get_all_monitoring_lists().get();
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_lists.size(), 2);
    }

    {
        auto result = service->delete_monitoring_list(monitor_id).get();
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

    {
        auto result = service->get_values_for_monitoring_list(monitor_id).get(); // should not exist anymore
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

    {
        // one-off monitoring_list
        auto result = service->create_monitoring_list_with_paths({
            parameter_instance_path("Identity/OrderNumber"),
            parameter_instance_path("Channels/2/Value/IoValue", "2-1"),
            parameter_instance_path("Channels/3/Value/IoValue", "2-1"),
            parameter_instance_path("Channels/3/Value/IoValue", "2-2")}, 0).get();

        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_list.id, 4); //should keep generating different ids than previously deleted monitoring_lists. This actually tests for a certain way of assigning the monitoring_list_id
        monitor_id = result.monitoring_list.id;
    }

    {
        auto result = service->get_values_for_monitoring_list(monitor_id).get();
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.parameter_values.size(), 4);
        auto& values = result.parameter_values;
        EXPECT_EQ(values[0].value->get_string(), "0768-3301");
        EXPECT_EQ(values[1].status, status_codes::parameter_not_provided);
        EXPECT_EQ(values[2].status, status_codes::parameter_not_provided);
        EXPECT_EQ(values[3].status, status_codes::unknown_device);
    }

    {
        auto result = service->get_values_for_monitoring_list(monitor_id).get(); // should not exist anymore, because it was a one-off
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

    {
        auto result = service->get_all_monitoring_lists().get();
        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_lists.size(), 2);
    }

    {
        auto result = service->delete_monitoring_list(2).get();
        EXPECT_EQ(result.status, status_codes::success);
    }

    {
        auto result = service->delete_monitoring_list(3).get();
        EXPECT_EQ(result.status, status_codes::success);
    }

    // testing timeout
    {
        auto result = service->create_monitoring_list_with_paths({
            parameter_instance_path("Identity/OrderNumber"),
        }, 2).get();

        EXPECT_EQ(result.status, status_codes::success);
        EXPECT_EQ(result.monitoring_list.id, 5); //this actually tests for a certain way of assigning the monitoring_list_id
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_values_for_monitoring_list(5).get();
        EXPECT_EQ(result.status, status_codes::success);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_monitoring_list(5).get();
        EXPECT_EQ(result.status, status_codes::success); // refreshes the timeout
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_monitoring_list(5).get();
        EXPECT_EQ(result.status, status_codes::success); // refreshes the timeout
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_monitoring_list(5).get();
        EXPECT_EQ(result.status, status_codes::success); // refreshes the timeout
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_all_monitoring_lists().get();
        EXPECT_EQ(result.status, status_codes::success); // does not refresh the timeout
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_all_monitoring_lists().get();
        EXPECT_EQ(result.status, status_codes::success); // does not refresh the timeout
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {
        auto result = service->get_values_for_monitoring_list(5).get(); // should not exist anymore, because it has timed out
        EXPECT_EQ(result.status, status_codes::unknown_monitoring_list);
    }

}
