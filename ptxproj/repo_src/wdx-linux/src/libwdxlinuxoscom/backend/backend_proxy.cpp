//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of the backend proxy.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "backend_proxy.hpp"
#include "backend_methods.hpp"

#include <wc/log.h>

#include <memory>
#include <exception>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {
void debug_log_exception_ptr(std::exception_ptr const &e_ptr);
void check(future<std::vector<wdx::register_file_provider_response>>       &&future_responses,
           std::string                                               const  &message);
void check(future<std::vector<wdx::response>>       &&future_responses,
           std::string                        const  &message);
void check(wdx::response const &response,
           std::string   const &message);

template<class ResponseType>
std::vector<ResponseType> create_error_responses(size_t const count);

template<class RequestType,
         class StubType>
bool extract_provider_from_stub(std::vector<RequestType> &providers, 
                                managed_object const &obj, 
                                std::function<bool(StubType const &)> filter);

template<class RequestType>
bool extract_provider_from_stub(std::vector<RequestType> &providers, 
                                managed_object const &obj);

bool extract_provider_from_stub(std::vector<wdx::reregister_file_provider_request> &requests, managed_object const &obj, std::function<bool(file_provider_stub const &)> filter);

}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

backend_proxy::backend_proxy(managed_object_id                id,
                             std::string               const &connection_name,
                             sender_i                        &sender,
                             driver_i                        &driver,
                             managed_object_store_i          &object_store,
                             registered_device_store_i       &device_store,
                             wc_trace_channels          const  channel)
: abstract_proxy(id, connection_name, sender, driver)
, object_store_m(object_store)
, device_store_m(device_store)
, channel_m(channel)
{}

void backend_proxy::reregister_devices()
{
    if(device_store_m.has_devices())
    {
        std::vector<wdx::register_device_request> requests;
        device_store_m.remove_devices([&](registered_device_store_i::device_meta const &meta){
            wdx::register_device_request request = { meta.id, meta.order_number,  meta.firmware_version };
            requests.push_back(std::move(request));
            return true;
        });
        wc_log(log_level_t::info, ("Try to re-register " + std::to_string(requests.size()) + " devices").c_str());
        check(register_devices(requests), "Failed to register device");
    }
}

void backend_proxy::reregister_providers()
{
    if(object_store_m.has_generated_objects())
    {
        // remove potentially old stubs from manager,
        // which were created for a recent connection
        std::vector<wdx::model_provider_i *>               model_providers;
        std::vector<wdx::device_description_provider_i *>  device_description_providers;
        std::vector<wdx::device_extension_provider_i *>    device_extension_providers;
        std::vector<wdx::parameter_provider_i *>           parameter_providers;
        std::vector<wdx::parameter_provider_i *>           serial_parameter_providers;
        std::vector<wdx::reregister_file_provider_request> file_providers;
        std::vector<wdx::reregister_file_provider_request> serial_file_providers;
        object_store_m.remove_objects([&](auto const &obj) {
            if(obj.get_id() != backend_object_id)
            {
                bool extracted_provider = extract_provider_from_stub(model_providers, obj)
                                       || extract_provider_from_stub(device_description_providers, obj)
                                       || extract_provider_from_stub(device_extension_providers, obj)
                                       || extract_provider_from_stub<wdx::parameter_provider_i*, parameter_provider_stub>(
                                            parameter_providers, obj,
                                            [](parameter_provider_stub const &stub) { return !stub.used_serial_mode_on_register; }
                                          )
                                       || extract_provider_from_stub<wdx::parameter_provider_i*, parameter_provider_stub>(
                                            serial_parameter_providers, obj,
                                            [](parameter_provider_stub const &stub) { return  stub.used_serial_mode_on_register; }
                                          )
                                       || extract_provider_from_stub(
                                            file_providers, obj,
                                            [](file_provider_stub const &stub) { return !stub.used_serial_mode_on_register; }
                                          )
                                       || extract_provider_from_stub(
                                            serial_file_providers, obj,
                                            [](file_provider_stub const &stub) { return  stub.used_serial_mode_on_register; }
                                          );
                WC_ASSERT(extracted_provider);
                return true;
            }
            return false;
        });
        wc_log(log_level_t::info, "Try to re-register providers:");
        wc_log(log_level_t::info, ("Register " + std::to_string(model_providers.size()) + " model providers").c_str());
        check(register_model_providers(model_providers), "Failed to register model provider");
        wc_log(log_level_t::info, ("Register " + std::to_string(device_description_providers.size()) + " device description providers").c_str());
        check(register_device_description_providers(device_description_providers), "Failed to register device description provider");
        wc_log(log_level_t::info, ("Register " + std::to_string(device_extension_providers.size()) + " device extension providers").c_str());
        check(register_device_extension_providers(device_extension_providers), "Failed to register device extension provider");
        wc_log(log_level_t::info, ("Register " + std::to_string(parameter_providers.size()) + " parameter providers").c_str());
        check(register_parameter_providers(parameter_providers, wdx::parameter_provider_call_mode::concurrent), "Failed to register parameter provider");
        wc_log(log_level_t::info, ("Register " + std::to_string(serial_parameter_providers.size()) + " parameter providers (serial mode)").c_str());
        check(register_parameter_providers(serial_parameter_providers, wdx::parameter_provider_call_mode::serialized), "Failed to register parameter provider (serial mode)");
        wc_log(log_level_t::info, ("Register " + std::to_string(file_providers.size()) + " file providers").c_str());
        check(reregister_file_providers(file_providers, wdx::provider_call_mode::concurrent), "Failed to register file provider");
        wc_log(log_level_t::info, ("Register " + std::to_string(serial_file_providers.size()) + " file providers (serial mode)").c_str());
        check(reregister_file_providers(serial_file_providers, wdx::provider_call_mode::serialized), "Failed to register file provider (serial mode)");
        wc_log(log_level_t::info, "Re-registration of providers done");
    }
}

future<std::vector<wdx::response>> backend_proxy::register_devices(std::vector<wdx::register_device_request> requests)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::response>>>();
    try
    {
        auto future_responses = call<future<std::vector<wdx::response>>>(backend_method_id::backend_register_devices, requests);
        future_responses.set_notifier([requests, response_promise, device_store = &device_store_m](std::vector<wdx::response> responses) {
            for(size_t i = 0; i < requests.size(); ++i)
            {
                auto const &current_response = responses.at(i);
                if(!current_response.has_error() || current_response.status == status_codes::device_already_exists)
                {
                    device_store->add_device(requests.at(i));
                }
            }
            response_promise->set_value(std::move(responses));
        });
        future_responses.set_exception_notifier([response_promise, request_count = requests.size()](std::exception_ptr const e_ptr){
            debug_log_exception_ptr(e_ptr);
            response_promise->set_value(create_error_responses<wdx::response>(request_count));
        });
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, (std::string("Failed to register devices: ") + e.what()).c_str());
        response_promise->set_value(create_error_responses<wdx::response>(requests.size()));
    }

    return response_promise->get_future();
}

future<wdx::response> backend_proxy::unregister_all_devices(wdx::device_collection_id_t device_collection)
{
    auto response_promise = std::make_shared<promise<wdx::response>>();
    try
    {
        auto future_response = call<future<wdx::response>>(backend_method_id::backend_unregister_all_devices, device_collection);
        future_response.set_notifier([device_collection, response_promise, device_store = &device_store_m](wdx::response response) {
            if(!response.has_error())
            {
                device_store->remove_devices([device_collection](registered_device_store_i::device_meta const &meta){
                    return (meta.id.device_collection_id == device_collection);
                });
            }
            response_promise->set_value(std::move(response));
        });
        future_response.set_exception_notifier([response_promise](std::exception_ptr const e_ptr){
            debug_log_exception_ptr(e_ptr);
            response_promise->set_value(wdx::response(wdx::status_codes::internal_error));
        });
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, (std::string("Failed to unregister all devices: ") + e.what()).c_str());
        response_promise->set_value(wdx::response(wdx::status_codes::internal_error));
    }

    return response_promise->get_future();
}

future<std::vector<wdx::response>> backend_proxy::unregister_devices(std::vector<wdx::device_id> device_ids)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::response>>>();
    try
    {
        auto future_responses = call<future<std::vector<wdx::response>>>(backend_method_id::backend_unregister_devices, device_ids);
        future_responses.set_notifier([device_ids, response_promise, device_store = &device_store_m](std::vector<wdx::response> responses) {
            for(size_t i = 0; i < device_ids.size(); ++i)
            {
                if(!responses.at(i).has_error())
                {
                    device_store->remove_device(device_ids.at(i));
                }
            }
            response_promise->set_value(std::move(responses));
        });
        future_responses.set_exception_notifier([response_promise, request_count = device_ids.size()](std::exception_ptr const e_ptr){
            debug_log_exception_ptr(e_ptr);
            response_promise->set_value(create_error_responses<wdx::response>(request_count));
        });
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, (std::string("Failed to unregister devices: ") + e.what()).c_str());
        response_promise->set_value(create_error_responses<wdx::response>(device_ids.size()));
    }

    return response_promise->get_future();
}

future<std::vector<wdx::response>> backend_proxy::register_parameter_providers(std::vector<wdx::parameter_provider_i*> providers,
                                                                               wdx::parameter_provider_call_mode       mode)
{
    return register_providers<wdx::response,
                              parameter_provider_stub>(providers,
                                                       backend_method_id::backend_create_parameter_provider_proxies,
                                                       backend_method_id::backend_register_parameter_providers,
                                                       mode,
                                                       channel_m);
}

void backend_proxy::unregister_parameter_providers(std::vector<wdx::parameter_provider_i*> providers)
{
    unregister_providers(providers, backend_method_id::backend_unregister_parameter_providers);
}

future<std::vector<wdx::response>> backend_proxy::register_model_providers(std::vector<wdx::model_provider_i*> providers)
{
    std::vector<wdx::response> responses(providers.size());
    for(auto& response : responses)
    {
        response.set_error(wdx::status_codes::not_implemented);
    }
    return resolved_future(std::move(responses));
}

void backend_proxy::unregister_model_providers(std::vector<wdx::model_provider_i*> providers)
{ }

future<std::vector<wdx::response>> backend_proxy::register_device_description_providers(std::vector<wdx::device_description_provider_i*> providers)
{
    std::vector<wdx::response> responses(providers.size());
    for(auto& response : responses)
    {
        response.set_error(wdx::status_codes::not_implemented);
    }
    return resolved_future(std::move(responses));
}

void backend_proxy::unregister_device_description_providers(std::vector<wdx::device_description_provider_i*> providers)
{ }

future<std::vector<wdx::response>> backend_proxy::register_device_extension_providers(std::vector<wdx::device_extension_provider_i*> providers)
{
    std::vector<wdx::response> responses(providers.size());
    for(auto& response : responses)
    {
        response.set_error(wdx::status_codes::not_implemented);
    }
    return resolved_future(std::move(responses));
}

void backend_proxy::unregister_device_extension_providers(std::vector<wdx::device_extension_provider_i*> providers)
{ }

future<std::vector<wdx::register_file_provider_response>> backend_proxy::register_file_providers(std::vector<wdx::register_file_provider_request> requests,
                                                                                                 wdx::provider_call_mode                          mode)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::register_file_provider_response>>>();

    // Create remote proxies first, then create stubs on our side
    auto const providers_count = requests.size();
    create_providers_proxies(response_promise, providers_count, backend_method_id::backend_create_file_provider_proxies, [this, response_promise, requests, object_store = &object_store_m, mode, channel = channel_m](std::vector<managed_object_id> object_ids){
        try
        {
            auto contexts = std::vector<wdx::parameter_id_t>(requests.size());
            for(size_t i = 0; i < object_ids.size(); ++i)
            {
                auto &stub = object_store->create_stub<file_provider_stub>(object_ids.at(i), *requests.at(i).provider, channel);
                stub.used_serial_mode_on_register = (mode == wdx::provider_call_mode::serialized);
                contexts.at(i) = requests.at(i).context;
                stub.set_context(requests.at(i).context);
            }

            // Register providers, but use the generated IDs instead of the pointers
            remote_register_providers(response_promise, object_ids, backend_method_id::backend_register_file_providers, mode,
                                      [response_promise, object_ids, object_store](std::vector<wdx::register_file_provider_response> register_results){
                for(size_t i = 0; i < object_ids.size(); ++i)
                {
                    auto const &result = register_results.at(i);
                    if(!result.is_success())
                    {
                        object_store->remove_object(object_ids.at(i));
                    }
                    else
                    {
                        auto &stub = object_store->get_object<file_provider_stub>(object_ids.at(i));
                        stub.set_file_id(result.registered_file_id);
                    }
                }
                response_promise->set_value(std::move(register_results));
            }, contexts);
        }
        catch(std::exception const &e)
        {
            wc_log(log_level_t::error, (std::string("Failed to register providers: ") + e.what()).c_str());
            response_promise->set_value(create_error_responses<wdx::register_file_provider_response>(requests.size()));
        }
    });

    return response_promise->get_future();
}

void backend_proxy::unregister_file_providers(std::vector<wdx::file_provider_i*> providers)
{
    unregister_providers(providers, backend_method_id::backend_unregister_file_providers);
}

future<std::vector<wdx::register_file_provider_response>> backend_proxy::reregister_file_providers(std::vector<wdx::reregister_file_provider_request> requests, wdx::provider_call_mode mode)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::register_file_provider_response>>>();

        // Create remote proxies first, then create stubs on our side
        auto const providers_count = requests.size();
        create_providers_proxies(response_promise, providers_count, backend_method_id::backend_create_file_provider_proxies, [this, response_promise, requests, object_store = &object_store_m, mode, channel = channel_m](std::vector<managed_object_id> object_ids){
            try
            {
                auto contexts = std::vector<wdx::parameter_id_t>(requests.size());
                auto file_ids = std::vector<wdx::file_id>(       requests.size());
                for(size_t i = 0; i < object_ids.size(); ++i)
                {
                    auto &stub = object_store->create_stub<file_provider_stub>(object_ids.at(i), *requests.at(i).provider, channel);
                    stub.used_serial_mode_on_register = (mode == wdx::provider_call_mode::serialized);
                    contexts.at(i) = requests.at(i).context;
                    stub.set_context(requests.at(i).context);
                    file_ids.at(i) = requests.at(i).id;
                    stub.set_file_id(requests.at(i).id);
                }

                // Re-register providers, but use the generated IDs instead of the pointers
                auto reregister_future_results = call<future<std::vector<wdx::register_file_provider_response>>>(backend_method_id::backend_reregister_file_providers, object_ids, mode, contexts, file_ids);
                reregister_future_results.set_notifier([response_promise, object_store, object_ids](std::vector<wdx::register_file_provider_response> reregister_results){
                    for(size_t i = 0; i < object_ids.size(); ++i)
                    {
                        auto const &result = reregister_results.at(i);
                        if(result.is_success())
                        {
                            auto &stub = object_store->get_object<file_provider_stub>(object_ids.at(i));
                            stub.set_file_id(result.registered_file_id);
                        }
                    }
                    response_promise->set_value(std::move(reregister_results));
                });
            }
            catch(std::exception const &e)
            {
                wc_log(log_level_t::error, (std::string("Failed to register providers: ") + e.what()).c_str());
                response_promise->set_value(create_error_responses<wdx::register_file_provider_response>(requests.size()));
            }
        });

        return response_promise->get_future();
}

template <class Interface>
std::vector<managed_object_id> backend_proxy::get_provider_stub_ids(std::vector<Interface*> const &providers)
{
    std::vector<managed_object_id> ids;
    for(size_t i = 0; i < providers.size(); ++i)
    {
        auto *stub = object_store_m.find_stub(*providers.at(i));
        if(stub)
        {
            ids.push_back(stub->get_id());
        }
        else
        {
            ids.push_back(unknown_managed_object_id);
        }
    }
    return ids;
}

template<typename ResponseType>
void backend_proxy::create_providers_proxies(std::shared_ptr<promise<std::vector<ResponseType>>>       response_promise,
                                             size_t                                              const providers_count,
                                             method_id_type                                      const create_proxy_id,
                                             future_notifier<std::vector<managed_object_id>>&&         notifier)
{
    try
    {
        // Create remote proxies first, then create stubs on our side
        auto future_object_ids = call<future<std::vector<managed_object_id>>>(create_proxy_id, providers_count);
        future_object_ids.set_notifier(std::move(notifier));
        future_object_ids.set_exception_notifier([response_promise, providers_count](std::exception_ptr const e_ptr){
            debug_log_exception_ptr(e_ptr);
            response_promise->set_value(create_error_responses<ResponseType>(providers_count));
        });
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, (std::string("Failed to prepare provider proxies: ") + e.what()).c_str());
        response_promise->set_value(create_error_responses<ResponseType>(providers_count));
    }
}

template<typename ProviderType,
         typename ResponseType>
void backend_proxy::remote_update_providers(std::shared_ptr<promise<std::vector<ResponseType>>>,
                                            std::vector<managed_object_id>                      const &,
                                            std::vector<ProviderType*>                          const  ,
                                            future_notifier<no_return>&&                               notifier)
{
    // just jump to next step
    notifier(no_return());
}

template<>
void backend_proxy::remote_update_providers(std::shared_ptr<promise<std::vector<wdx::response>>>        response_promise,
                                            std::vector<managed_object_id>                       const &object_ids,
                                            std::vector<wdx::parameter_provider_i*>              const  providers,
                                            future_notifier<no_return>&&                                notifier)
{
    WC_ASSERT(object_ids.size() == providers.size());
    size_t const providers_count = providers.size();
    try
    {
        std::vector<std::string>                      names;
        std::vector<wdx::parameter_selector_response> selected_parameters;
        for(auto const provider : providers)
        {
            names.push_back(provider->display_name());
            selected_parameters.push_back(provider->get_provided_parameters());
        }
        auto future_responses = call<future<no_return>>(backend_method_id::backend_update_parameter_provider_proxies, object_ids, names, selected_parameters);
        future_responses.set_notifier(std::move(notifier));
        future_responses.set_exception_notifier([response_promise = std::move(response_promise), providers_count](std::exception_ptr const e_ptr){
            debug_log_exception_ptr(e_ptr);
            response_promise->set_value(create_error_responses<wdx::response>(providers_count));
        });
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, (std::string("Failed to update provider proxies: ") + e.what()).c_str());
        response_promise->set_value(create_error_responses<wdx::response>(providers_count));
    }
}

template<typename ResponseType,
         typename ... ArgumentTypes>
void backend_proxy::remote_register_providers(std::shared_ptr<promise<std::vector<ResponseType>>>        response_promise,
                                              std::vector<managed_object_id>                      const &object_ids,
                                              method_id_type                                      const  register_id,
                                              wdx::provider_call_mode                             const  mode,
                                              future_notifier<std::vector<ResponseType>>&&               notifier,
                                              ArgumentTypes &...                                         args)
{
    auto const providers_count = object_ids.size();
    auto future_responses = call<future<std::vector<ResponseType>>>(register_id, object_ids, mode, args...);
    if(notifier)
    {
        future_responses.set_notifier(std::move(notifier));
    }
    else
    {
        future_responses.set_notifier([response_promise](std::vector<ResponseType> responses){
            response_promise->set_value(std::move(responses));
        });
    }
    future_responses.set_exception_notifier([response_promise, providers_count](std::exception_ptr const e_ptr){
        debug_log_exception_ptr(e_ptr);
        response_promise->set_value(create_error_responses<ResponseType>(providers_count));
    });
}

template<typename ResponseType,
         typename ProviderStubType,
         typename ProviderType>
future<std::vector<ResponseType>> backend_proxy::register_providers(std::vector<ProviderType*> const providers,
                                                                    method_id_type             const create_proxy_id,
                                                                    method_id_type             const register_id,
                                                                    wdx::provider_call_mode    const mode,
                                                                    wc_trace_channels          const channel)
{
    auto response_promise = std::make_shared<promise<std::vector<ResponseType>>>();

    // Create remote proxies first, then create stubs on our side
    auto const providers_count = providers.size();
    create_providers_proxies(response_promise, providers_count, create_proxy_id, [this, response_promise, providers, object_store = &object_store_m, register_id, mode, channel](std::vector<managed_object_id> object_ids){
        try
        {
            for(size_t i = 0; i < object_ids.size(); ++i)
            {
                auto &stub = object_store->create_stub<ProviderStubType>(object_ids.at(i), *providers.at(i), channel);
                stub.used_serial_mode_on_register = (mode == wdx::provider_call_mode::serialized);
            }

            // Set provider invars, if any
            remote_update_providers(response_promise, object_ids, providers, [this, response_promise, object_ids, register_id, mode, object_store = &object_store_m](no_return){
                // Register providers, but use the generated IDs instead of the pointers
                remote_register_providers(response_promise, object_ids, register_id, mode, [response_promise, object_ids, object_store](std::vector<ResponseType> responses){
                    // Remove stub from object store if registration failed
                    for(size_t i = 0; i < object_ids.size(); ++i)
                    {
                        auto const &result = responses.at(i);
                        if(!result.is_success())
                        {
                            object_store->remove_object(object_ids.at(i));
                        }
                    }
                    response_promise->set_value(std::move(responses));
                });
            });
        }
        catch(std::exception const &e)
        {
            wc_log(log_level_t::error, (std::string("Failed to register providers: ") + e.what()).c_str());
            response_promise->set_value(create_error_responses<ResponseType>(providers.size()));
        }
    });

    return response_promise->get_future();
}

template<typename ProviderType>
void backend_proxy::unregister_providers(std::vector<ProviderType*> const providers,
                                         method_id_type             const unregister_id)
{
    // Remove stubs, proxies will be removed on the remote side automatically on unregistration
    auto stub_ids = get_provider_stub_ids(providers);
    auto remote_responses = call<future<no_return>>(unregister_id, stub_ids);
    remote_responses.set_notifier([](no_return const &){
        std::string const message = std::string("Operation done: Unregistered providers of type ") + typeid(ProviderType).name();
        wc_log(log_level_t::debug, message.c_str());
    });

    // Set marker in separate loop to keep access window for others to get a lock as small as possible
    std::vector<std::shared_ptr<managed_object_store_i::object_meta>> stub_meta_vector(stub_ids.size());
    for(size_t i = 0; i < stub_ids.size(); ++i)
    {
        stub_meta_vector.at(i) = object_store_m.get_object_meta(stub_ids.at(i));
        WC_ASSERT(stub_meta_vector.at(i).get() != nullptr); // Stub should exist
        if(stub_meta_vector.at(i).get() != nullptr)
        {
            stub_meta_vector.at(i)->marked_to_destroy = true;
        }
    }
    for(size_t i = 0; i < stub_ids.size(); ++i)
    {
        auto const stub_id = stub_ids.at(i);
        if(stub_meta_vector.at(i).get() != nullptr)
        {
            try
            {
                if(stub_meta_vector.at(i)->call_counter.fetch_add(1) != 0)
                {
                    // This unique lock constructor locks the given mutex on construction
                    std::unique_lock<std::mutex> lock(stub_meta_vector.at(i)->object_mutex);
                    stub_meta_vector.at(i)->call_counter_condition.wait(lock, [&call_counter = stub_meta_vector.at(i)->call_counter]{
                        return (call_counter == 1);
                    });
                    lock.unlock();
                }
                object_store_m.remove_object(stub_id);
            }
            catch(...)
            {
                wc_log(log_level_t::fatal, ("Failed to remove stub (object_id="
                                           + std::to_string(stub_id) + ")").c_str());
                // Catch all exceptions to avoid throwing.
                // - std::lock_guard throws when recursivly locking the same mutex, which
                //   is most certainly not the case.
                WC_FAIL("Unexpected exception caught when unregister providers in backend_proxy");
            }
        }
    }
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace {
void debug_log_exception_ptr(std::exception_ptr const &e_ptr)
{
    try
    {
        if(e_ptr)
        {
            std::rethrow_exception(e_ptr);
        }
    }
    catch(std::exception const &e)
    {
        WC_DEBUG_LOG((std::string("Received unexpected exception from backend: ") + e.what()).c_str());
    }
}

void check(future<std::vector<wdx::register_file_provider_response>>       &&future_responses,
           std::string                                               const  &message)
{
    future_responses.set_notifier([message](std::vector<wdx::register_file_provider_response> const &responses){
        for(auto &response : responses)
        {
            check(response, message);
        }
    });
}

void check(future<std::vector<wdx::response>>       &&future_responses,
           std::string                        const  &message)
{
    future_responses.set_notifier([message](std::vector<wdx::response> const &responses){
        for(auto &response : responses)
        {
            check(response, message);
        }
    });
}

void check(wdx::response const &response,
           std::string   const &message)
{
    if(response.has_error())
    {
        wc_log(log_level_t::fatal, message.c_str());
        wc_Fail(message.c_str(), "<backend_proxy>", "<re-register>", 0);
    }
}


template<class ResponseType>
std::vector<ResponseType> create_error_responses(size_t const count)
{
    std::vector<ResponseType> error_responses(count);
    for(auto &error_response : error_responses)
    {
        error_response.status = wdx::status_codes::internal_error;
    }

    return error_responses;
}

template<class RequestType>
bool extract_provider_from_stub(std::vector<RequestType>       &providers,
                                managed_object           const &obj)
{
    return extract_provider_from_stub<RequestType, abstract_stub<std::remove_pointer_t<RequestType>>>(providers, obj, nullptr);
}

template<class RequestType,
         class StubType>
bool extract_provider_from_stub(std::vector<RequestType>       &providers,
                                managed_object                        const &obj,
                                std::function<bool(StubType const &)>        filter)
{
    auto *stub = dynamic_cast<StubType const *>(&obj);
    if(    (stub != nullptr)
        && ((filter == nullptr) || filter(*stub)))
    {
        providers.push_back(&stub->target());
        return true;
    }
    return false;
}

bool extract_provider_from_stub(std::vector<wdx::reregister_file_provider_request>       &requests,
                                managed_object                                     const &obj,
                                std::function<bool(file_provider_stub const &)>           filter)
{
    auto *stub = dynamic_cast<file_provider_stub const *>(&obj); // parasoft-suppress CERT_C-EXP39-b-3 "Types are not incompatible"
    if(    (stub != nullptr)
        && ((filter == nullptr) || filter(*stub)))
    {
        requests.emplace_back(&stub->target(), stub->get_file_id(), stub->get_context());
        return true;
    }
    return false;
}

}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
