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
#include <sstream>
#include <exception>
#include "test_providers.hpp"

// ToDo: check why this include is necessary here. It should have been already included by the base class
#include "wago/wdx/parameter_service_backend_i.hpp"

parameter_selector_response rocket_headstation_provider::get_provided_parameters() {
    return parameter_selector_response({
        parameter_selector::all_of_class("EthernetPort", device_selector::headstation()),
        parameter_selector::all_of_class("IPService", device_selector::headstation()),
        parameter_selector::all_of_feature("GW750Tunnel", device_selector::headstation()),
        parameter_selector::all_of_feature("TestFeature", device_selector::headstation()),
        parameter_selector::all_of_feature("ExtendedFeature", device_selector::headstation()),
        parameter_selector::all_of_feature("TestFeature", device_selector::specific({4, device_collections::rlb}))
    });
}

wago::future<std::vector<value_response>> rocket_headstation_provider::get_parameter_values(const std::vector<parameter_instance_id> parameterIDs)
{
    std::vector<value_response> result(parameterIDs.size());

    for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx) {
        result[idx].status = status_codes::parameter_value_unavailable;
        auto& id = parameterIDs[idx];

        // only for Test, if fallback device description is used (e. g 82xx)
        if (id.id == 1)
        {
            result[idx].set_value(parameter_value::create("0768-3301"));
        }
        else if (id.id == 10000)
        {
            result[idx].set_value(parameter_value::create("Hallo")); // wrong data type
        }
        else if (id.id == 10001)
        {
            result[idx].set_value(parameter_value::create("Hallo Welt")); // wrong pattern
        }
        else if(id.id == 10008)
        {
            throw std::runtime_error("I exploded.");
        }
        else if(id.id == 10011)
        {
            result[idx].set_value(parameter_value::create_ipv4address("2.3.4.5"));
        }
        else if(id.id == 10033)
        {
            result[idx].set_value(parameter_value::create("Base-Param"));
        }
        else if(id.id == 10034)
        {
            result[idx].set_value(parameter_value::create("Extended-Param"));
        }
        else if(id.id == 10036)
        {
            result[idx].set_value(parameter_value::create("Class-Param"));
        }
        else if(id.id == 20001)
        {
            result[idx].set_value(parameter_value::create_uint16(4156));
        }
        else if(id.id == 10021)
        {
            result[idx].set_error(status_codes::success, "evil");
        }
        else if (id.id == 20003)
        {
            result[idx].set_value(parameter_value::create(internal_instances.at(id.instance_id-1)));
        }
        else if(id.id == 20002 || id.id == 20004)
        {
            result[idx].set_value(parameter_value::create(internal_infos.at(id.instance_id-1)));
        }
        else if(id.id == 100001)
        {
            std::vector<class_instantiation> instantiations;
            int i=internal_instances.size(); // reversing the order of instances to test sorting
            for(auto& s : internal_instances)
            {
                instantiations.push_back(class_instantiation(i, internal_instance_types.at(i-1)));
                i--;
            }
            result[idx].set_value(parameter_value::create_instantiations(std::move(instantiations)));
        }
        else if(id.id == 100002)
        {
            result[idx].set_value(parameter_value::create_instantiations({class_instantiation(1,"DynamicReadOnlyClass"),class_instantiation(1001,"DynamicReadOnlyClass"),class_instantiation(1002,"DynamicReadOnlyClass")}));
        }
        else if(id.id == 10015)
        {
            ::std::vector<instance_id_t> instances = {};
            result[idx].set_value(parameter_value::create_instance_ref_array(instances));
        }
        else if(id.id == 10020)
        {
            result[idx].set_domain_specific_error(42, "Ups");
        }
        else if(id.id == 10027) 
        {
            std::vector<std::pair<std::string,instance_id_t>> instances = {}; 
            result[idx].set_value(parameter_value::create_instance_identity_ref_array(instances));
            auto values = result[idx].value->get_items();
        } 

        // add more test parameters here
    }

    return wago::resolved_future(std::move(result));
}

wago::future<std::vector<set_parameter_response>> rocket_headstation_provider::set_parameter_values(std::vector<value_request> value_requests) {
    std::vector<set_parameter_response> result(value_requests.size());

    for (size_t idx = 0, e = value_requests.size(); idx < e; ++idx)
    {
        result[idx].status = status_codes::could_not_set_parameter;
        auto& id = value_requests[idx].param_id;
        if(id.id == 10008) {
            throw std::runtime_error("I exploded.");
        }
        else if(id.id == 10001) {
            result[idx].set_domain_specific_error(43, "Mein Fehlertext");
        }
        else if(id.id == 100001) {
            auto instantiations = value_requests[idx].value->get_instantiations();
            internal_instances.clear();
            internal_instance_types.clear();
            internal_infos.clear();
            for(auto& inst : instantiations) {
                // TODO: check inst id and classes
                internal_instance_types.push_back(inst.classes);
                internal_instances.push_back("");
                internal_infos.push_back("");
            }
            result[idx].set_success();
        }
        else if(id.id == 20003) {
            internal_instances[id.instance_id-1] = value_requests[idx].value->get_string();
            result[idx].set_success();
        }
        else if(id.id == 20002 || id.id == 20004) {
            internal_infos[id.instance_id-1] = value_requests[idx].value->get_string();
            result[idx].set_success();
        }
    }

    return wago::resolved_future(std::move(result)); 
}

wago::future<method_invocation_response> rocket_headstation_provider::invoke_method(const parameter_instance_id methodID, const std::vector<std::shared_ptr<parameter_value>> inArgs)
{
    method_invocation_response response;
    if (methodID.id == 4012) {
        response.status = status_codes::success;
        response.domain_specific_status_code = 32945;
        auto b = inArgs[0]->get_bytes();
        b.push_back(255);
        response.out_args.push_back(parameter_value::create_bytes(b));
    }
    else if (methodID.id == 10002) {
        response.set_out_args({parameter_value::create(true)}); // wrong data type
    }
    else if (methodID.id == 10007) {
        response.set_out_args({parameter_value::create(inArgs[0]->get_string() + inArgs[1]->get_string())});
    }
    else if (methodID.id == 10022) {
        auto ping = inArgs[0]->get_enum_value();
        if(ping != 100)
            response.set_out_args({parameter_value::create_enum_value_array({3}), parameter_value::create_uint16(ping)});
        else
            response.set_out_args({parameter_value::create_enum_value_array({4}), parameter_value::create_uint16(ping)}); // this will be rejected by validation
    }
    else if (methodID.id == 10037) {
        auto ping = inArgs[0]->get_instance_ref();
        if(ping == 1)
            response.set_out_args({parameter_value::create_instance_ref_array({2}), parameter_value::create_uint16(ping)});
        else
            response.set_out_args({parameter_value::create_instance_ref_array({3}), parameter_value::create_uint16(ping)}); // this will be rejected by validation
    }
    else if (methodID.id == 10023) {
        throw std::runtime_error("I exploded.");
    }
    return wago::resolved_future(std::move(response));
}

//****************************************************************************************************


parameter_selector_response ThatDeviceProvider::get_provided_parameters()
{
    return parameter_selector_response({});
}


//****************************************************************************************************


parameter_selector_response test_provider::get_provided_parameters()
{
    return parameter_selector_response({
        parameter_selector::all_of_devices(device_selector::all_of(device_collections::kbus)),
        parameter_selector::all_with_definition(1100, device_selector::all_of(device_collections::rlb)),
        parameter_selector::all_of_feature("WAGODevice", device_selector::all_of(device_collections::rlb))
    });
}

wago::future<std::vector<value_response>> test_provider::get_parameter_values(const std::vector<parameter_instance_id> parameterIDs)
{
    std::vector<value_response> result(parameterIDs.size());

    for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx) {
        result[idx].status = status_codes::parameter_value_unavailable;
        auto& id = parameterIDs[idx];

        if (id.id == 1100 || id.id == 1110)
        {
            result[idx].set_value(parameter_value::create(true));
        }
        else if (id.id == 1006)
        {
            result[idx].set_value(parameter_value::create("1234"));
        }

        // add more test parameters here
    }

    return wago::resolved_future(std::move(result));
}

//****************************************************************************************************



wago::future<wdm_response> model_provider1::get_model_information() {
    auto wdm = R"(
        {
    "WDMMVersion": "1.0.0",
    "Name": "WAGO",
    "Features": [
        {
            "ID": "Feature1",
            "Classes": ["Class1"],
            "Parameters": [
                {
                    "ID": 8,
                    "Path": "Param1",
                    "Type": "String"
                }
            ]
        }
    ],
    "Classes": [
        {
            "ID": "Class1",
            "BasePath": "TestClasses",
            "BaseID": 11231,
            "Parameters": [
                {
                    "ID": 2,
                    "Path": "Param2",
                    "Type": "Enum",
                    "Enum": "Enum1"
                }
            ]
        }
    ],
    "Enums": [
        {
            "Name": "Enum1",
            "Members": [
                {
                    "ID": 1,
                    "Name": "EnumMember1"
                }
            ]
        }
    ]
}
    )";
    return wago::resolved_future(wdm_response(std::move(wdm)));
}

device_selector_response model_provider1::get_provided_devices() {
    return device_selector_response({device_selector::any});
}

wago::future<wdd_response> model_provider1::get_device_information(std::string order_number, std::string firmware_version) {
    auto wdd = R"(
{
    "WDMMVersion": "1.0.0",
    "ModelReference": "WAGO",
    "Features": [
        "Feature1"
    ],
    "ParameterValues": [
        {
            "ID": 8,
            "Value": "Hallo"
        }
    ],
    "Instantiations": [
        {
            "Class": "Class1",
            "Instances": [
                {
                    "ID": 1
                }
            ]
        }
    ]
}
    )";
    return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
}

//****************************************************************************************************


wago::future<wdm_response> model_provider2::get_model_information() {
    auto wdm = R"(
        {
    "WDMMVersion": "1.0.0",
    "Name": "WAGO",
    "Features": [
        {
            "ID": "Feature2",
            "Classes": [],
            "Parameters": [
                {
                    "ID": 3,
                    "Path": "Param3",
                    "Type": "String"
                }
            ]
        },
        {
            "ID": "Feature3",
            "Classes": [],
            "Parameters": [
                {
                    "ID": 5,
                    "Path": "Param5",
                    "Type": "String"
                }
            ]
        }
    ],
    "Classes": [
        {
            "ID": "Class2",
            "BasePath": "TestClasses",
            "Parameters": [
                {
                    "ID": 4,
                    "Path": "Param4",
                    "Type": "Enum",
                    "Enum": "Enum1"
                }
            ]
        }
    ],
    "Enums": [
        {
            "Name": "Enum2",
            "Members": [
                {
                    "ID": 1,
                    "Name": "EnumMember1"
                }
            ]
        }
    ]
}
    )";

    // TODO: remove Feature3 to test bundling
    return wago::resolved_future(wdm_response(std::move(wdm)));
}

device_selector_response model_provider2::get_provided_devices() {
    return device_selector_response({device_selector::any});
}

wago::future<wdd_response> model_provider2::get_device_information(std::string order_number, std::string firmware_version) {
    auto wdd = R"(
{
    "WDMMVersion": "1.0.0",
    "ModelReference": "WAGO",
    "Features": [
        "Feature2", "Feature3"
    ],
    "ParameterValues": [
        {
            "ID": 3,
            "Value": "Wurst"
        },
        {
            "ID": 5,
            "Value": "Klappt"
        }
    ],
    "Instantiations": [
        {
            "Class": "Class2",
            "Instances": [
                {
                    "ID": 1
                }
            ]
        }
    ]
}
    )";


    // TODO: use to test bundling
    auto wdm = R"(
        {
    "WDMMVersion": "1.0.0",
    "Name": "WAGO",
    "Features": [
        {
            "ID": "Feature3",
            "Classes": ["Class2"],
            "Parameters": [
                {
                    "ID": 5,
                    "Path": "Param5",
                    "Type": "String"
                }
            ]
        }
    ]
}
    )";

    return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
}

wago::future<wdm_response> clueless_model_provider::get_model_information() {
    return wago::resolved_future(wdm_response());
}

device_selector_response clueless_model_provider::get_provided_devices() {
    return device_selector_response({device_selector::any});
}

wago::future<wdd_response> clueless_model_provider::get_device_information(std::string order_number, std::string firmware_version) {
    return wago::resolved_future(wdd_response());
}

wago::future<wdm_response> parameter_provider3::get_model_information() {
    auto wdm = R"(
        {
    "WDMMVersion": "1.0.0",
    "Name": "WAGO",
    "Features": [
        {
            "ID": "Feature4",
            "Classes": ["DynClass4"],
            "Parameters": [
                {
                    "ID": 7,
                    "Path": "Param7",
                    "Type": "String"
                }
            ]
        }
    ],
    "Classes": [
        {
            "ID": "DynClass4",
            "BaseID": 10017,
            "BasePath": "DynClasses",
            "Dynamic": true,
            "Parameters": [
                {
                    "ID": 8,
                    "Path": "Param8",
                    "Type": "String"
                }
            ]
        }
    ]
}
    )";
    return wago::resolved_future(wdm_response(std::move(wdm)));
}

device_extension_response parameter_provider3::get_device_extensions() {
    return device_extension_response(device_selector::any, {"Feature4"});
}

parameter_selector_response parameter_provider3::get_provided_parameters() {
    return parameter_selector_response({parameter_selector::all_of_feature("Feature4")});
}

wago::future<std::vector<value_response>> parameter_provider3::get_parameter_values(std::vector<parameter_instance_id> parameter_ids) {
    std::vector<value_response> result(parameter_ids.size());
    
    for (size_t idx = 0, e = parameter_ids.size(); idx < e; ++idx) {
        auto& id = parameter_ids[idx];

        if (id.id == 7)
        {
            result[idx].set_value(parameter_value::create("EsGeht"));
        }
        else if (id.id == 10017)
        {
            result[idx].set_value(parameter_value::create_instantiations({class_instantiation(1, "DynClass4"), class_instantiation(2, "DynClass4")}));
        }
        else if (id.id == 8)
        {
            result[idx].set_value(parameter_value::create("DynParam"+std::to_string(id.instance_id)));
        }
    }

    return wago::resolved_future(std::move(result));   
}