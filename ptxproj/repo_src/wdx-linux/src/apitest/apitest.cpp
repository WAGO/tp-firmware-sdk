//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Implementation of a the APITest
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "apitest.hpp"
#include "defines.hpp"

#include "generic_parameter_provider.hpp"
#include "file_parameter_provider.hpp"
#include "generic_error_parameter_provider.hpp"
#include "generic_file_provider.hpp"
#include "frontend_callback_provider.hpp"
#include "generic_method_provider.hpp"
#include "deferred_parameter_provider.hpp"
#include "generic_response_provider.hpp"
#include "exception_parameter_provider.hpp"
#include "exit_parameter_provider.hpp"
#include "file_api_callback_provider.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

namespace
{
    template<class T> 
    T* to_raw(std::unique_ptr<T> &unique)
    {
        return unique.get();
    }

    template<class T>
    std::vector<T*> to_raws(std::vector<std::unique_ptr<T>> &uniques)
    {
        std::vector<T*> raws;
        for (auto &unique : uniques)
        {
            raws.push_back(to_raw(unique));
        }
        return raws;
    }
}

apitest::apitest(wdx::parameter_service_backend_i &backend,
                 bool                              support_error_parameters)
{
    //Register RLB devices
    std::vector<wago::wdx::register_device_request> device_requests=
    {
        {{1,2}, "0763-1108",           "03.02.00"},
        {{2,2}, "0763-1116",           "03.02.00"},
        {{3,2}, "0763-5101",           "03.02.01"},
        {{4,2}, "0763-1508",           "03.02.00"},
        {{5,2}, "0763-1516",           "03.02.00"},
        {{6,2}, "0763-4750/0010-0000", "03.02.00"}
    };
    backend.register_devices(device_requests);

    // register the file providers for the static files
    std::string file_content = "I'm a file!";
    file_providers.push_back(std::make_unique<generic_file_provider>(std::vector<uint8_t>(file_content.data(), file_content.data() + file_content.size())));
    auto readonly_file_provider_registration_future = backend.register_file_provider(to_raw(file_providers.at(0)), g_api_test_id_files_static_readonly);
    readonly_file_provider_registration_future.set_exception_notifier([](std::exception_ptr e_ptr){
        try
        {
            if(e_ptr)
            {
                std::rethrow_exception(e_ptr);
            }
        }
        catch(std::exception const &e)
        {
            // We do not expect any exception
            WC_FAIL(e.what());
        }
    });
    readonly_file_provider_registration_future.set_notifier([this, &backend, support_error_parameters](wdx::register_file_provider_response readonly_file_provider_registration){
        WC_ASSERT(readonly_file_provider_registration.status == wdx::status_codes::success);
        concurrent_mode_parameter_providers.push_back(std::make_unique<file_parameter_provider>(
            device_selector::headstation(),
            g_api_test_id_files_static_writeable,
            false,
            backend
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<file_parameter_provider>(
            device_selector::headstation(),
            g_api_test_id_files_static_failed_validation,
            true,
            backend
        ));

        // register test parameters
        std::map<parameter_id_t, std::shared_ptr<parameter_value>> headstation_initial_parameter_values = {

            // "Basic" feature
            { g_api_test_id_basic_string,   parameter_value::create_string("test123") },
            { g_api_test_id_basic_boolean,  parameter_value::create_boolean(true) },
                                                                            // M_PI32 (math.h) does not work with clang-tidy
            { g_api_test_id_basic_float32,  parameter_value::create_float32(static_cast<float>(M_PI)) },
                                                                            // M_PI64 (math.h) does not work with clang-tidy
            { g_api_test_id_basic_float64,  parameter_value::create_float64(M_PI) },
            { g_api_test_id_basic_bytes,    parameter_value::create_bytes({ 0x01, 0x02, 0x03, 0x04 }) },
            { g_api_test_id_basic_uint8,    parameter_value::create_uint8(UINT8_MAX) },
            { g_api_test_id_basic_uint16,   parameter_value::create_uint16(UINT16_MAX) },
            { g_api_test_id_basic_uint32,   parameter_value::create_uint32(UINT32_MAX) },
            { g_api_test_id_basic_uint64,   parameter_value::create_uint64(UINT64_MAX) },
            { g_api_test_id_basic_int8,     parameter_value::create_int8(INT8_MAX) },
            { g_api_test_id_basic_int16,    parameter_value::create_int16(INT16_MAX) },
            { g_api_test_id_basic_int32,    parameter_value::create_int32(INT32_MAX) },
            { g_api_test_id_basic_int64,    parameter_value::create_int64(INT64_MAX) },
            { g_api_test_id_basic_enum,     parameter_value::create_enum_value(1) },

            // "Files" feature
            { g_api_test_id_files_static_readonly,          parameter_value::create_file_id(readonly_file_provider_registration.registered_file_id) },
          //{ g_api_test_id_files_static_writeable,         /* created internally by file parameter provider */ },
          //{ g_api_test_id_files_static_failed_validation, /* created internally by file parameter provider */ },

            // "CoreStatusCode" feature
            { g_api_test_id_core_status_code_not_writable,      parameter_value::create_boolean(true) },
            { g_api_test_id_core_status_code_unknown_parameter, parameter_value::create_boolean(false) },
          //{ g_api_test_id_core_status_code_not_provided,      parameter_value::create_boolean(false) }, // Explicitly not provided

            // "References" feature
            { g_api_test_id_references_refparam,         parameter_value::create_instance_identity_ref("APITest/References/Class", 1) },
            { g_api_test_id_references_danglingrefparam, parameter_value::create_instance_identity_ref("APITest/References/Class", 42) },
            { g_api_test_id_references_refparamarray,    parameter_value::create_instance_identity_ref_array({
                "APITest/References/Class/1",
                "APITest/References/Class/42",
                "APITest/References/Class/2",
                "APITest/References/OtherClass/123"
            }) }
        };

        // Classes feature
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_parameter_provider>(
            device_selector::headstation(),
            std::map<parameter_id_t, std::shared_ptr<parameter_value>> {
                { g_api_test_id_classes_dynamic, parameter_value::create_instantiations({ wdx::class_instantiation(1, "APITestDynamicClass"),
                                                                                          wdx::class_instantiation(2, "APITestDynamicClass")}) },
                { g_api_test_id_classes_dynamic_parameter, parameter_value::create_boolean(true) },
                { g_api_test_id_classes_dynamic_string,    parameter_value::create_string("★ Greek Capital Letter Omega: Ω") },
                { g_api_test_id_classes_static_no_fourty_two_on_instance_number_two, parameter_value::create_uint8(0) }
            }
        ));
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_classes_dynamic_method
        ));
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_classes_static_method
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_parameter_provider>(
            device_selector::headstation(),
            headstation_initial_parameter_values
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<frontend_callback_provider>(
            device_selector::headstation(),
            g_api_test_id_frontend_callback_read,
            g_api_test_id_basic_int16,
            g_api_test_id_basic_int16,
            g_api_test_id_basic_int16
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<frontend_callback_provider>(
            device_selector::headstation(),
            g_api_test_id_frontend_callback_write,
            g_api_test_id_basic_int16,
            g_api_test_id_basic_int16,
            g_api_test_id_basic_int16
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<frontend_callback_provider>(
            device_selector::headstation(),
            g_api_test_id_frontend_callback_method,
            g_api_test_id_methods_simple,
            g_api_test_id_methods_simple,
            g_api_test_id_methods_simple
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<file_api_callback_provider>(
            device_selector::headstation(),
            g_api_test_id_file_read_callback,
            g_api_test_id_file_write_callback,
            g_api_test_id_file_get_info_callback,
            g_api_test_id_file_create_callback
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_methods_simple
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_methods_multiply,
            [] (std::vector<std::shared_ptr<wdx::parameter_value>> in_args) {
                return std::vector<std::shared_ptr<wdx::parameter_value>> {
                    // simple multiplication; no error checks
                    wdx::parameter_value::create_float32(in_args.at(0)->get_float32() * in_args.at(1)->get_float32())
                };
            },
            true
        ));

        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_methods_wait,
            [] (std::vector<std::shared_ptr<wdx::parameter_value>> in_args) {
                std::this_thread::sleep_for(std::chrono::milliseconds( in_args.at(0)->get_uint16() * 1000 ));
                return std::vector<std::shared_ptr<wdx::parameter_value>> {};
            }
        ));

        // register error test parameters
        std::map<wdx::parameter_id_t, generic_error_parameter_provider::error_state> headstation_initial_error_parameters = {

            // "DomainStatusCode" feature
            { g_api_test_id_domain_status_code_read,   { wdx::status_codes::no_error_yet, "unavailable read test",   1 } },
            { g_api_test_id_domain_status_code_write,  { wdx::status_codes::no_error_yet, "unavailable write test",  2 } },
            { g_api_test_id_domain_status_code_method, { wdx::status_codes::no_error_yet, "unavailable invoke test", 3 } },
        };
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_error_parameter_provider>(
            device_selector::headstation(),
            headstation_initial_error_parameters
        ));

        // register deferred parameter provider
        concurrent_mode_parameter_providers.push_back(std::make_unique<deferred_parameter_provider>(
            device_selector::headstation(),
            std::map<parameter_id_t, std::shared_ptr<parameter_value>> {
                { g_api_test_id_deferred_parameter, parameter_value::create_string("Test") }
            }
        ));

        // status value unavailable
        value_response unavailable_status_value;
        unavailable_status_value.set_status_unavailable("Test");
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_response_provider>(
            device_selector::headstation(),
            std::map<parameter_id_t, value_response> {
                { g_api_test_id_unavailable_status_value, unavailable_status_value }
            }
        ));

        // Beta parameter and method
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_parameter_provider>(
            device_selector::headstation(),
            std::map<parameter_id_t, std::shared_ptr<parameter_value>>({ 
                { g_api_test_id_beta_parameter, parameter_value::create_string("I'm BETA!") }
            })
        ));
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_beta_method
        ));

        // Deprecated parameter and method
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_parameter_provider>(
            device_selector::headstation(),
            std::map<parameter_id_t, std::shared_ptr<parameter_value>>({
                { g_api_test_id_deprecated_parameter, parameter_value::create_string("I'm deprecated!") }
            })
        ));
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_method_provider>(
            device_selector::headstation(),
            g_api_test_id_deprecated_method
        ));

        // success but value adjusted
        value_response         value_adjusted;
        set_parameter_response success_but_value_adjusted;
        value_adjusted.set_value(parameter_value::create_string("Always respond this value"));
        success_but_value_adjusted.set_success_but_value_adjusted(wdx::parameter_value::create_string("I'm never happy with your value ;)"));
        concurrent_mode_parameter_providers.push_back(std::make_unique<generic_response_provider>(
            device_selector::headstation(),
            std::map<parameter_id_t, value_response> {
                { g_api_test_id_adjusted_parameter_value, value_adjusted }
            },
            std::map<parameter_id_t, set_parameter_response> {
                { g_api_test_id_adjusted_parameter_value, success_but_value_adjusted }
            }
        ));

        if(support_error_parameters)
        {
            // register exception throwing parameter provider
            concurrent_mode_parameter_providers.push_back(std::make_unique<exception_parameter_provider>(
                device_selector::headstation(),
                std::map<parameter_id_t, std::exception_ptr> {
                    { g_api_test_id_exception_throw_read,   std::make_exception_ptr(std::runtime_error("throw_read")) },
                    { g_api_test_id_exception_throw_write,  std::make_exception_ptr(std::runtime_error("throw_write")) },
                    { g_api_test_id_exception_throw_method, std::make_exception_ptr(std::runtime_error("throw_method")) }
                },
            exception_parameter_provider::exception_mode::throwing
            ));
            concurrent_mode_parameter_providers.push_back(std::make_unique<exception_parameter_provider>(
                device_selector::headstation(),
                std::map<parameter_id_t, std::exception_ptr> {
                    { g_api_test_id_exception_notify_read,   std::make_exception_ptr(std::runtime_error("notify_read")) },
                    { g_api_test_id_exception_notify_write,  std::make_exception_ptr(std::runtime_error("notify_write")) },
                    { g_api_test_id_exception_notify_method, std::make_exception_ptr(std::runtime_error("notify_method")) }
                },
                exception_parameter_provider::exception_mode::future_notifier
            ));

            // Parameter providers who exit() on calls (concurrent mode)
            concurrent_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_concurrent_parameter_sync,
                exit_parameter_provider::exit_mode::sync
            ));
            concurrent_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_concurrent_parameter_async,
                exit_parameter_provider::exit_mode::async
            ));
            concurrent_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_concurrent_method_sync,
                exit_parameter_provider::exit_mode::sync
            ));
            concurrent_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_concurrent_method_async,
                exit_parameter_provider::exit_mode::async
            ));


            // Parameter providers who exit() on calls (serialized mode)
            serialized_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_serialized_parameter_sync,
                exit_parameter_provider::exit_mode::sync
            ));
            serialized_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_serialized_parameter_async,
                exit_parameter_provider::exit_mode::async
            ));
            serialized_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_serialized_method_sync,
                exit_parameter_provider::exit_mode::sync
            ));
            serialized_mode_parameter_providers.push_back(std::make_unique<exit_parameter_provider>(
                device_selector::headstation(),
                g_api_test_id_terminate_serialized_method_async,
                exit_parameter_provider::exit_mode::async
            ));
        }

        auto register_result = backend.register_parameter_providers(to_raws(concurrent_mode_parameter_providers), wdx::parameter_provider_call_mode::concurrent);
        register_result.set_notifier([this, &backend](std::vector<wdx::response> &&) {
            auto register_serialized_result = backend.register_parameter_providers(to_raws(serialized_mode_parameter_providers), wdx::parameter_provider_call_mode::serialized);
            register_serialized_result.set_notifier([this, &backend](std::vector<wdx::response> &&){
                // !!! Keep this as last registered parameter !!!
                std::map<parameter_id_t, std::shared_ptr<parameter_value>> ready_parameter_values = {
                    { g_api_test_id_ready, parameter_value::create_boolean(true) }
                };
                auto ready_provider = std::make_unique<generic_parameter_provider>(device_selector::headstation(),
                                                                                   ready_parameter_values);
                backend.register_parameter_provider(ready_provider.get());
                concurrent_mode_parameter_providers.push_back(std::move(ready_provider));
            });
        });
    });
}

apitest::~apitest() noexcept = default;

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
