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
//------------------------------------------------------------------------------
#ifndef TEST_SRC_CORE_TEST_PROVIDERS_HPP_
#define TEST_SRC_CORE_TEST_PROVIDERS_HPP_

#include <vector>
#include <memory>
#include <exception>
#include <chrono>
#include <thread>
#include "wago/wdx/responses.hpp"
#include "wago/wdx/base_parameter_provider.hpp"
#include "model_provider_i.hpp"
#include "device_description_provider_i.hpp"
#include "device_extension_provider_i.hpp"
#include "wago/wdx/file_transfer/base_file_provider.hpp"
#include "wago/wdx/parameter_service_backend_i.hpp"

using namespace wago::wdx;
using namespace std;

class rocket_headstation_provider : public base_parameter_provider
{
    parameter_selector_response get_provided_parameters() override;

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;
    
    wago::future<method_invocation_response> invoke_method(parameter_instance_id methodID, std::vector<std::shared_ptr<parameter_value>> inArgs) override;

    private:
    std::vector<std::string> internal_instances = {"Jep"};
    std::vector<std::vector<std::string>> internal_instance_types = {{"DynamicSpecialClass"}};
    std::vector<std::string> internal_infos = {"Blabla"};

};

class description_provider1 : public device_description_provider_i
{
    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::specific({4, device_collections::rlb})});
    }

    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": ["TestFeature"],
            "Instantiations": [
                {
                    "Class": "DynamicSpecialClassB",
                    "Instances": [
                        {
                            "ID": 1,
                            "ParameterValues": [
                                {
                                    "ID": 20005,
                                    "Value": "Vorgabewert1"
                                },
                                {
                                    "ID": 20003,
                                    "Value": "Vorgabewert0"
                                }
                            ]
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }
};

class ThatDeviceProvider : public base_parameter_provider
{
    parameter_selector_response get_provided_parameters() override;
};

class test_provider : public base_parameter_provider
{
    parameter_selector_response get_provided_parameters() override;
    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs) override;
};

class model_provider1 : public model_provider_i, public device_description_provider_i
{
    wago::future<wdm_response> get_model_information() override;
    device_selector_response get_provided_devices() override;
    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override;
};

class model_provider2 : public model_provider_i, public device_description_provider_i
{
    wago::future<wdm_response> get_model_information() override;
    device_selector_response get_provided_devices() override;
    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override;
};

class clueless_model_provider : public model_provider_i, public device_description_provider_i
{
    wago::future<wdm_response> get_model_information() override;
    device_selector_response get_provided_devices() override;
    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override;
};

class parameter_provider3 : public base_parameter_provider, public model_provider_i, public device_extension_provider_i
{
    wago::future<wdm_response> get_model_information() override;
    device_extension_response get_device_extensions() override;

    parameter_selector_response get_provided_parameters() override;
    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override;
};

class file_provider : public base_file_provider
{
    wago::future<file_read_response> read(uint64_t offset, size_t length) override {
        return wago::resolved_future(file_read_response({(uint8_t)offset, (uint8_t)length}));
    }
};

class file_parameter_provider : public base_parameter_provider
{
    parameter_service_backend_i* backend = nullptr;
    std::vector<std::shared_ptr<file_provider>> fps;
    
    public:
    file_parameter_provider(parameter_service_backend_i* backend_): backend(backend_) {}
    
    parameter_selector_response get_provided_parameters() override
    {
        return parameter_selector_response({parameter_selector::all_of_feature("FileFeature")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs) override
    {
        std::vector<value_response> result(parameterIDs.size());

        for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx)
        {
            auto& id = parameterIDs[idx];
            if (id.id == 10010)
            {
                auto fp = std::make_shared<file_provider>();
                fps.push_back(fp);
                auto r = backend->register_file_provider(fp.get(), id.id).get();
                if(r.has_error())
                {
                    result[idx].set_error(r.status);
                }
                else
                {
                    result[idx].set_value(parameter_value::create_file_id(r.registered_file_id));
                }
            }
        }

        // TODO: when to unregister and cleanup? 

        return wago::resolved_future(std::move(result));
    }
};

class async_pp : public base_parameter_provider, public model_provider_i, public device_extension_provider_i
{
    public:
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature5",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 13014,
                            "Path": "AsyncParam",
                            "Type": "String",
                            "Writeable": true
                        },
                        {
                            "ID": 13015,
                            "Path": "AsyncErrorParam",
                            "Type": "String"
                        },
                        {
                            "ID": 13016,
                            "Path": "AsyncTimeoutParam",
                            "Type": "String"
                        },
                        {
                            "ID": 13017,
                            "Path": "AsyncMethod",
                            "Type": "Method",
                            "InArgs": [
                                {
                                    "Name": "In",
                                    "Type": "UInt8"
                                }
                            ],
                            "OutArgs": [
                                {
                                    "Name": "Out",
                                    "Type": "UInt16"
                                }
                            ]
                        },
                        {
                            "ID": 13018,
                            "Path": "AsyncMethodWithException",
                            "Type": "Method"
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));
    }

    device_extension_response get_device_extensions() override
    {
        return device_extension_response(device_selector::any, {"Feature5"});
    }

    parameter_selector_response get_provided_parameters() override
    {
        return parameter_selector_response({parameter_selector::all_of_feature("Feature5")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override
    {
        auto promise = std::make_shared<wago::promise<std::vector<value_response>>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            std::cout << std::this_thread::get_id() << " WAKEUP" << std::endl;
            try
            {
                std::vector<value_response> result(parameter_ids.size());
                for (size_t idx = 0, e = parameter_ids.size(); idx < e; ++idx)
                {
                    auto& id = parameter_ids[idx];

                    if (id.id == 13014)
                    {
                        result[idx].set_value(parameter_value::create("EsGeht"));
                    }
                    else if (id.id == 13015)
                    {
                        throw std::runtime_error("ParameterProviderException"); // this will terminate the program if not handled    
                    }
                    else if (id.id == 13016)
                    {
                        result[idx].set_value(parameter_value::create("Aha"));
                        // std::this_thread::sleep_for(20s); // should lead to timeout
                    }
                }
                promise->set_value(std::move(result));
            }
            catch(...)
            {
                std::cout << std::this_thread::get_id() << " EXCEPTION" << std::endl;
                promise->set_exception(std::make_exception_ptr(std::runtime_error("ParameterProviderException")));
            }
        });

        th.detach();
        return promise->get_future();
    }

    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override
    {
        auto promise = std::make_shared<wago::promise<std::vector<set_parameter_response>>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            std::cout << std::this_thread::get_id() << " WAKEUP" << std::endl;
            try
            {
                std::vector<set_parameter_response> result(value_requests.size());
                for (size_t idx = 0, e = value_requests.size(); idx < e; ++idx)
                {
                    auto& id = value_requests[idx].param_id;

                    if (id.id == 13014)
                    {
                        result[idx].set_success();
                    }
                }
                promise->set_value(std::move(result));
            }
            catch(...)
            {
                std::cout << std::this_thread::get_id() << " EXCEPTION" << std::endl;
                promise->set_value(std::move(std::vector<set_parameter_response>(value_requests.size(), set_parameter_response(status_codes::internal_error))));
            }
        });

        th.detach();
        return promise->get_future();    
    }

    wago::future<method_invocation_response> invoke_method(parameter_instance_id method_id, std::vector<std::shared_ptr<parameter_value>> in_args) override
    {
        auto promise = std::make_shared<wago::promise<method_invocation_response>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            std::cout << std::this_thread::get_id() << " WAKEUP" << std::endl;
            try
            {
                method_invocation_response result;
                if (method_id.id == 13017)
                {
                    auto n = in_args[0]->get_uint8();
                    result.set_out_args({parameter_value::create_uint16(2*n)});
                }
                else if (method_id.id == 13018)
                {
                    promise->set_exception(make_exception_ptr(std::runtime_error("ParameterProviderException")));
                    return;
                }
                promise->set_value(std::move(result));
            }
            catch(...)
            {
                std::cout << std::this_thread::get_id() << " EXCEPTION" << std::endl;
                promise->set_value(method_invocation_response(status_codes::internal_error));
            }
        });

        th.detach();
        return promise->get_future();
    }
};


class partial_model_provider1 : public model_provider_i
{
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature1",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 15014,
                            "Path": "Param1",
                            "Type": "String"
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));

    }
};
class partial_model_provider2 : public model_provider_i
{
    wago::future<wdm_response> get_model_information() override
    {
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
                            "ID": 15015,
                            "Path": "Param2",
                            "Type": "String"
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));

    }
};
class partial_description_provider : public device_description_provider_i
{
    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }
    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "Feature1", "Feature2"
            ],
            "ParameterValues": [
                {
                    "ID": 15014,
                    "Value": "Hallo"
                },
                {
                    "ID": 15015,
                    "Value": "Du"
                }
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }
};

class delayed_provider : public model_provider_i, public device_description_provider_i, public base_parameter_provider
{
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature1",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 15,
                            "Path": "Param1",
                            "Type": "String"
                        }
                    ]
                }
            ]
        }
        )";
        auto p = std::make_shared<wago::promise<wdm_response>>();

        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            p->set_value(wdm_response(std::move(wdm)));
        });

        th.detach();
        return p->get_future();
    }

    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }
    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "Feature1"
            ]
        }
        )";

        auto p = std::make_shared<wago::promise<wdd_response>>();

        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            p->set_value(wdd_response::from_pure_wdd(std::move(wdd)));
        });

        th.detach();
        return p->get_future();
    }

    parameter_selector_response get_provided_parameters() override
    {
        return parameter_selector_response({parameter_selector::all_of_feature("Feature1")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override
    {
        std::vector<value_response> result(parameter_ids.size());
        for (size_t idx = 0, e = parameter_ids.size(); idx < e; ++idx)
        {
            auto& id = parameter_ids[idx];

            if (id.id == 15)
            {
                result[idx].set_value(parameter_value::create("Hallo"));
            }
        }
        return wago::resolved_future<std::vector<value_response>>(std::move(result));
    }
};

class dynamic_provider : public model_provider_i, public device_description_provider_i, public base_parameter_provider
{
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "DynamicFeature",
                    "Classes": ["DynamicClass", "DynamicSpecialClass"],
                    "Parameters": [
                        {
                            "ID": 15015,
                            "Path": "FeatureParam",
                            "Type": "String"
                        }
                    ]
                }
            ],
            "Classes": [
                {
                    "ID": "DynamicClass",
                    "BasePath": "Dynamics",
                    "BaseID": 15016,
                    "Dynamic": true,
                    "Parameters": [
                        {
                            "Path": "ClassParam0",
                            "ID": 20003,
                            "Type": "String"
                        }
                    ]
                },
                {
                    "ID": "DynamicSpecialClass",
                    "Includes": ["DynamicClass"],
                    "Parameters": [
                        {
                            "Path": "ClassParam1",
                            "ID": 20002,
                            "Type": "String"
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));

    }
    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }
    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "DynamicFeature"
            ],
            "ParameterValues": [
                {
                    "ID": 15015,
                    "StatusUnavailableIfNotProvided": true
                },
                {
                    "ID": 15016,
                    "StatusUnavailableIfNotProvided": true
                }
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }
    parameter_selector_response get_provided_parameters() override
    {
        return parameter_selector_response({parameter_selector::all_of_feature("DynamicFeature")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs) override
    {
        std::vector<value_response> result(parameterIDs.size());

        for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx)
        {
            auto& id = parameterIDs[idx];
            if (id.id == 15016)
            {
                result[idx].set_value(parameter_value::create_instantiations({class_instantiation(1, "DynamicClass"), class_instantiation(2, "DynamicSpecialClass")}));
            }
            else if (id.id == 15015)
            {
                result[idx].set_value(parameter_value::create_string("Huhu"));
            }
            else if (id.id == 20003)
            {
                result[idx].set_value(parameter_value::create_string("Haha" + std::to_string(id.instance_id)));
            }
            else if (id.id == 20002)
            {
                result[idx].set_value(parameter_value::create_string("Hoho" + std::to_string(id.instance_id)));
            }
        }
        return wago::resolved_future(std::move(result));
    }
};

class autarc_parameter_provider : public base_parameter_provider, public model_provider_i, public device_description_provider_i
{
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature9",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 15112,
                            "Path": "Param1",
                            "Type": "String"
                        },
                        {
                            "ID": 15113,
                            "Path": "WriteableParam1",
                            "Type": "String",
                            "Writeable": true
                        },
                        {
                            "ID": 15210,
                            "Path": "UnavailableUserSetting1",
                            "Type": "String",
                            "Writeable": true,
                            "UserSetting": true
                        },
                        {
                            "ID": 15211,
                            "Path": "UnavailableStatusParam1",
                            "Type": "String",
                            "OnlyOnline": true
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));
    }

    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }

    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "Feature9"
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }

    parameter_selector_response get_provided_parameters()
    {
        return parameter_selector_response({parameter_selector::all_of_feature("Feature9")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs)
    {
        std::vector<value_response> result(parameterIDs.size());

        for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx)
        {
            auto& id = parameterIDs[idx];
            if (id.id == 15112)
            {
                result[idx].set_value(parameter_value::create_string("MeinWert"));
            }
            else if(id.id == 15210)
            {
                result[idx].set_status_unavailable("SomeReason");
            }
            else if(id.id == 15211)
            {
                result[idx].set_status_unavailable("SomeReason");
            }
        }

        return wago::resolved_future(std::move(result));
    }
    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests)
    {
        std::vector<set_parameter_response> result(value_requests.size());

        for (size_t idx = 0, e = value_requests.size(); idx < e; ++idx)
        {
            auto& req = value_requests[idx];
            if (req.param_id.id == 15113)
            {
                result[idx].set_success();
            }
        }
        return wago::resolved_future(std::move(result));
    }
};

class connection_aware_parameter_provider : public base_parameter_provider, public model_provider_i, public device_description_provider_i
{
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature10",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 15114,
                            "Path": "ConnectionChangingParam",
                            "Type": "String",
                            "Writeable": true
                        },
                        {
                            "ID": 15115,
                            "Path": "NonConnectionChangingParam",
                            "Type": "String",
                            "Writeable": true
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));

    }

    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }

    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "Feature10"
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }

    parameter_selector_response get_provided_parameters()
    {
        return parameter_selector_response({parameter_selector::all_of_feature("Feature10")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs)
    {
        std::vector<value_response> result(parameterIDs.size());

        for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx)
        {
            auto& id = parameterIDs[idx];
            if (id.id == 15114)
            {
                result[idx].set_value(parameter_value::create_string(myvalue));
            }
            else if (id.id == 15115)
            {
                result[idx].set_value(parameter_value::create_string(myvalue+"Suffix"));
            }
        }

        return wago::resolved_future(std::move(result));    
    }

    wago::future<std::vector<set_parameter_response>> set_parameter_values_connection_aware(std::vector<value_request> value_requests, bool defer_wda_web_connection_changes)
    {
        std::vector<set_parameter_response> result(value_requests.size());

        for (size_t idx = 0, e = value_requests.size(); idx < e; ++idx)
        {
            auto& val = value_requests[idx];
            if (val.param_id.id == 15114)
            {
                if(defer_wda_web_connection_changes) {
                    result[idx].set_deferred();
                }
                else
                {
                    myvalue = val.value->get_string();
                    result[idx].set_success();
                }
            }
            else if (val.param_id.id == 15115)
            {
                result[idx].set_success();
            }
        }
        return wago::resolved_future(std::move(result));
    }
    private:
        std::string myvalue = "Startwert";
};

class serial_sensitive_parameter_provider : public base_parameter_provider, public model_provider_i, public device_description_provider_i
{
    public:
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature11",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 15116,
                            "Path": "SerialSensitiveParameter",
                            "Type": "String",
                            "Writeable": true
                        },
                        {
                            "ID": 15117,
                            "Path": "SerialSensitiveMethod",
                            "Type": "Method"
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));
    }

    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }

    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "Feature11"
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }

    parameter_selector_response get_provided_parameters()
    {
             return parameter_selector_response({parameter_selector::all_of_feature("Feature11")});
    }

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameterIDs)
    {
        printf("Request received: get_parameter_values\n");
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<std::vector<value_response>>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            std::vector<value_response> result(parameterIDs.size());
            for (size_t idx = 0, e = parameterIDs.size(); idx < e; ++idx)
            {
                auto& id = parameterIDs[idx];
                if (id.id == 15116)
                {
                    result[idx].set_value(parameter_value::create_string(myvalue));
                }
            }
            request_running = false;
            printf("Request done: get_parameter_values\n");
            promise->set_value(std::move(result));
        });
        th.detach();
        return promise->get_future();
    }
    wago::future<std::vector<set_parameter_response>> set_parameter_values_connection_aware(std::vector<value_request> value_requests, bool defer_wda_web_connection_changes)
    {
        printf("Request received: set_parameter_values_connection_aware\n");
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<std::vector<set_parameter_response>>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            std::vector<set_parameter_response> result(value_requests.size());
            for (size_t idx = 0, e = value_requests.size(); idx < e; ++idx)
            {
                auto& val = value_requests[idx];
                if (val.param_id.id == 15116)
                {
                    myvalue = val.value->get_string();
                    result[idx].set_success();
                }
                else if (val.param_id.id == 15115)
                {
                    result[idx].set_success();
                }
            }
            request_running = false;
            printf("Request done: set_parameter_values_connection_aware\n");
            promise->set_value(std::move(result));
        });
        th.detach();
        return promise->get_future();
    }

    wago::future<method_invocation_response> invoke_method(parameter_instance_id method_id, std::vector<std::shared_ptr<parameter_value>> in_args) override
    {
        printf("Request received: invoke_method\n");
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<method_invocation_response>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);
            method_invocation_response result;
            if (method_id.id == 15117)
            {
                result.set_out_args({});
            }
            request_running = false;
            printf("Request done: invoke_method\n");
            promise->set_value(std::move(result));
        });

        th.detach();
        return promise->get_future();
    }
    
    serial_sensitive_parameter_provider(): request_running(false) {}
    private:
    std::string myvalue = "Startwert";
    std::atomic<bool> request_running;
};
class serial_sensitive_file_provider : public base_file_provider
{
    public:
    wago::future<file_read_response> read(uint64_t offset, size_t length)
    {
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<file_read_response>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);         
            request_running = false;
            promise->set_value(file_read_response({1, 3, 4}));
        });
        th.detach();
        return promise->get_future();
    }

    wago::future<response> write(uint64_t offset, std::vector<uint8_t> data)
    {
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<response>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);         
            request_running = false;
            promise->set_value(response(status_codes::success));
        });
        th.detach();
        return promise->get_future();
    }

    wago::future<file_info_response> get_file_info()
    {
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<file_info_response>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);         
            request_running = false;
            promise->set_value(file_info_response(5));
        });
        th.detach();
        return promise->get_future();
    }

    wago::future<response> create(uint64_t capacity)
    {
        if(request_running)
        {
            throw runtime_error("Request already running.");
        }
        request_running = true;
        auto promise = std::make_shared<wago::promise<response>>();
        auto th = std::thread([=]() {
            std::this_thread::sleep_for(1s);         
            request_running = false;
            promise->set_value(response(status_codes::success));
        });
        th.detach();
        return promise->get_future();
    }
    
    serial_sensitive_file_provider(): request_running(false) {}
    private:
    std::atomic<bool> request_running;
};

class method_parameter_provider : public base_parameter_provider, public model_provider_i, public device_description_provider_i
{
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature14",
                    "Classes": ["ClassWithMethod"]
                }
            ],
            "Classes": [
                {
                    "ID": "ClassWithMethod",
                    "BasePath": "ClassesWithMethod",
                    "BaseID": 2044,
                    "Parameters": [
                        {
                            "ID": 2048,
                            "Path": "DoSomething",
                            "Type": "Method"
                        },
                        {
                            "ID": 2049,
                            "Path": "SomeParam",
                            "Type": "String"
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));
    }

    device_selector_response get_provided_devices() override
    {
        return device_selector_response({device_selector::any});
    }

    wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) override
    {
        auto wdd = R"(
        {
            "WDMMVersion": "1.0.0",
            "ModelReference": "WAGO",
            "Features": [
                "Feature14"
            ],
            "Instantiations": [
                {
                    "Class": "ClassWithMethod",
                    "Instances": [
                        {
                            "ID": 1
                        },
                        {
                            "ID": 2
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdd_response::from_pure_wdd(std::move(wdd)));
    }

    parameter_selector_response get_provided_parameters()
    {
        return parameter_selector_response({parameter_selector::all_of_feature("Feature14")});
    }

    wago::future<method_invocation_response> invoke_method(parameter_instance_id method_id, std::vector<std::shared_ptr<parameter_value>> in_args) override
    {
        method_invocation_response result;
        if (method_id.id == 2048)
        {
            result.set_out_args({});
        }
        return wago::resolved_future(std::move(result));
    }

};

class configfile_parameter_provider : public base_parameter_provider, public model_provider_i, public device_extension_provider_i
{
    public:
    wago::future<wdm_response> get_model_information() override
    {
        auto wdm = R"(
        {
            "WDMMVersion": "1.0.0",
            "Name": "WAGO",
            "Features": [
                {
                    "ID": "Feature15",
                    "Classes": [],
                    "Parameters": [
                        {
                            "ID": 15115,
                            "Path": "ConfigFileParam",
                            "Type": "FileID",
                            "Writeable": true,
                            "UserSetting": true
                        },
                        {
                            "ID": 15116,
                            "Path": "NormalParam",
                            "Type": "String",
                            "Writeable": true,
                            "UserSetting": true
                        }
                    ]
                }
            ]
        }
        )";
        return wago::resolved_future(wdm_response(std::move(wdm)));

    }

    device_extension_response get_device_extensions() override
    {
        return device_extension_response(device_selector::any, {"Feature15"});
    }

    parameter_selector_response get_provided_parameters()
    {
        return parameter_selector_response({parameter_selector::all_of_feature("Feature15")});
    }

    wago::future<file_id_response> create_parameter_upload_id(parameter_id_t context_) override
    {
        context = context_;
        auto r = service->register_file_provider(register_file_provider_request(fp.get(), context)).get();
        fid = r.registered_file_id;
        return wago::resolved_future(std::move(r));
    }

    wago::future<response> remove_parameter_upload_id(file_id id, parameter_id_t context_) override
    {
        cleanup_called = true;
        if(fid != id)
        {
            throw runtime_error("Wrong file_id.");
        }
        if(context != context_)
        {
            throw runtime_error("Wrong context.");
        }
        service->unregister_file_provider(fp.get());
        return wago::resolved_future(response(status_codes::success));
    }

    configfile_parameter_provider(parameter_service_backend_i* service_)
    : service(service_)
    {
        fp = std::make_shared<file_provider>();
        cleanup_called = false;
    }

    bool cleanup_called;

    private:
    std::shared_ptr<file_provider> fp;
    parameter_service_backend_i* service;
    parameter_id_t context;
    file_id fid;
};

#endif // TEST_SRC_CORE_TEST_PROVIDERS_HPP_
