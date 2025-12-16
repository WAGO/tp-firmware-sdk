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
///  \brief    Remote backend implementation managing remote provider proxies for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "remote_backend.hpp"
#include "common/definitions.hpp"
#include "common/manager.hpp"
#include "backend/parameter_provider_proxy.hpp"
#include "backend/file_provider_proxy.hpp"

#include <wago/wdx/status_codes.hpp>
#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
template<class Response,
         class ProviderType>
static void post_provider_register_processing(std::vector<managed_object_id> const &object_ids,
                                              managed_object_store_i               &store,
                                              std::vector<Response>          const &responses,
                                              std::vector<ProviderType*>     const &proxies,
                                              std::set<ProviderType*>              &registered_providers);

template<class ProviderType>
static void post_provider_unregister_processing(std::vector<managed_object_id> const &object_ids,
                                                managed_object_store_i               &store,
                                                std::vector<ProviderType*>     const &proxies,
                                                std::set<ProviderType*>              &registered_providers);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

// TODO: Remove and refactor were used
remote_backend::remote_backend(wdx::parameter_service_backend_extended_i &original_backend,
                               std::unique_ptr<manager>                   manager)
: backend_m(original_backend)
, store_m(manager->get_object_store())
, receiver_m(std::move(manager))
{ }

remote_backend::remote_backend(wdx::parameter_service_backend_extended_i &original_backend,
                               std::unique_ptr<receiver_i>                receiver,
                               managed_object_store_i                    &store)
: backend_m(original_backend)
, store_m(store)
, receiver_m(std::move(receiver))
{ }

remote_backend::~remote_backend() noexcept
{
    wc_log(log_level_t::debug, ("[" + store_m.get_connection_name() + " Stub 0] " // parasoft-suppress CERT_CPP-DCL57-a CERT_CPP-ERR50-b "bad_alloc exception is not thrown on Linux"
                                "Remote backend closing, unregister providers").c_str());
    for(auto proxy : registered_parameter_providers_m)
    {
        try
        {
            backend_m.unregister_parameter_provider(proxy);
        }
        catch(...)
        {
            WC_FAIL("Unregister should not throw.");
        }
    }
    for(auto proxy : registered_file_providers_m)
    {
        try
        {
            backend_m.unregister_file_provider(proxy);
        }
        catch(...)
        {
            WC_FAIL("Unregister should not throw.");
        }
    }
}

future<std::vector<wdx::response>> remote_backend::remote_register_devices(std::vector<wdx::register_device_request> requests)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::response>>>();

    auto future_responses = backend_m.register_devices(requests);
    future_responses.set_notifier([response_promise, requests, store = &store_m](std::vector<wdx::response> responses){
        for(size_t i = 0; i < requests.size(); ++i)
        {
            log_level_t const level = responses.at(i).has_error() ? log_level_t::error : log_level_t::info;
            wc_log(level, ("[" + store->get_connection_name()
                               + " Stub 0] registered device ("
                               + wdx::to_description(requests.at(i).device_id) + ", order-no "
                               + requests.at(i).order_number + ") result: "
                               + wdx::to_string(responses.at(i).status)).c_str());
        }
        response_promise->set_value(std::move(responses));
    });

    return response_promise->get_future();
}

future<std::vector<wdx::response>> remote_backend::remote_unregister_devices(std::vector<wdx::device_id> device_ids)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::response>>>();

    auto future_responses = backend_m.unregister_devices(device_ids);
    future_responses.set_notifier([response_promise, device_ids, store = &store_m](std::vector<wdx::response> responses){
        for(size_t i = 0; i < device_ids.size(); ++i)
        {
            WC_DEBUG_LOG(("[" + store->get_connection_name()
                              + " Stub 0] unregistered device ("
                              + wdx::to_description(device_ids.at(i)) + ") result: "
                              + wdx::to_string(responses.at(i).status)).c_str());
        }
        response_promise->set_value(std::move(responses));
    });

    return response_promise->get_future();
}

future<wdx::response> remote_backend::remote_unregister_all_devices(wdx::device_collection_id_t device_collection)
{
    auto response_promise = std::make_shared<promise<wdx::response>>();

    auto future_responses = backend_m.unregister_all_devices(device_collection);
    future_responses.set_notifier([response_promise, device_collection, store = &store_m](wdx::response response){
        WC_DEBUG_LOG(("[" + store->get_connection_name()
                          + " Stub 0] unregistered all devices (device collection "
                          // FIXME: Use wdx::to-string when available
                          + std::to_string(static_cast<unsigned>(device_collection)) + "), result: "
                          + wdx::to_string(response.status)).c_str());
        response_promise->set_value(std::move(response));
    });

    return response_promise->get_future();
}

future<std::vector<managed_object_id>> remote_backend::remote_create_parameter_provider_proxies(uint32_t count)
{
    return resolved_future(create_provider_proxies<parameter_provider_proxy>(count));
}

future<std::vector<managed_object_id>> remote_backend::remote_create_file_provider_proxies(uint32_t count)
{
    return resolved_future(create_provider_proxies<file_provider_proxy>(count));
}

future<no_return> remote_backend::remote_update_parameter_provider_proxies(std::vector<managed_object_id>                object_ids,
                                                                           std::vector<std::string>                      names,
                                                                           std::vector<wdx::parameter_selector_response> selected_parameters)
{
    auto proxies = get_provider_proxies<parameter_provider_proxy>(object_ids);
    WC_ASSERT(proxies.size() == object_ids.size());
    for(size_t i = 0; i < proxies.size(); ++i)
    {
        WC_DEBUG_LOG(("Update invars for proxy " + names[i]).c_str());
        proxies[i]->set_name(               std::move(names[i]));
        proxies[i]->set_provided_parameters(std::move(selected_parameters[i]));
    }

    return resolved_future(no_return());
}

future<std::vector<wdx::response>> remote_backend::remote_register_parameter_providers(std::vector<managed_object_id>    object_ids,
                                                                                       wdx::parameter_provider_call_mode call_mode)
{
    auto response_promise = std::make_shared<promise<std::vector<wdx::response>>>();

    auto proxies = get_provider_proxy_interfaces<parameter_provider_proxy>(object_ids);
    WC_ASSERT(proxies.size() == object_ids.size());
    auto future_responses = backend_m.register_parameter_providers(proxies, call_mode);
    future_responses.set_notifier([this, response_promise, object_ids, proxies](std::vector<wdx::response> responses){
        WC_ASSERT(responses.size() == proxies.size());
        post_provider_register_processing(object_ids, store_m, responses, proxies, registered_parameter_providers_m);
        response_promise->set_value(std::move(responses));
    });

    return response_promise->get_future();
}

future<no_return> remote_backend::remote_unregister_parameter_providers(std::vector<managed_object_id> object_ids)
{
    auto proxies = get_provider_proxy_interfaces<parameter_provider_proxy>(object_ids);
    WC_ASSERT(proxies.size() == object_ids.size());
    backend_m.unregister_parameter_providers(proxies);
    post_provider_unregister_processing(object_ids, store_m, proxies, registered_parameter_providers_m);

    return resolved_future(no_return());
}

future<std::vector<wdx::register_file_provider_response>> remote_backend::remote_register_file_providers(std::vector<managed_object_id>   object_ids,
                                                                                                         wdx::provider_call_mode          mode,
                                                                                                         std::vector<wdx::parameter_id_t> contexts)
{
    WC_ASSERT(object_ids.size() == contexts.size());
    auto response_promise = std::make_shared<promise<std::vector<wdx::register_file_provider_response>>>();

    auto proxies = get_provider_proxy_interfaces<file_provider_proxy>(object_ids);
    WC_ASSERT(proxies.size() == object_ids.size());
    std::vector<wdx::register_file_provider_request> requests(proxies.size(), {nullptr, 0});
    for(size_t i = 0; i < requests.size(); ++i)
    {
        requests[i] = wdx::register_file_provider_request(proxies.at(i), contexts.at(i));
    }

    auto future_responses = backend_m.register_file_providers(requests, mode);
    future_responses.set_notifier([this, response_promise, object_ids, proxies](std::vector<wdx::register_file_provider_response> responses){
        WC_ASSERT(responses.size() == proxies.size());
        post_provider_register_processing(object_ids, store_m, responses, proxies, registered_file_providers_m);
        response_promise->set_value(std::move(responses));
    });

    return response_promise->get_future();
}

future<no_return> remote_backend::remote_unregister_file_providers(std::vector<managed_object_id> object_ids)
{
    auto proxies = get_provider_proxy_interfaces<file_provider_proxy>(object_ids);
    WC_ASSERT(proxies.size() == object_ids.size());
    backend_m.unregister_file_providers(proxies);
    post_provider_unregister_processing(object_ids, store_m, proxies, registered_file_providers_m);

    return resolved_future(no_return());
}

future<std::vector<wdx::register_file_provider_response>> remote_backend::remote_reregister_file_providers(std::vector<managed_object_id>   object_ids,
                                                                                                           wdx::provider_call_mode          mode,
                                                                                                           std::vector<wdx::parameter_id_t> contexts,
                                                                                                           std::vector<wdx::file_id>        file_ids)
{
    WC_ASSERT(object_ids.size() == contexts.size());
    WC_ASSERT(object_ids.size() == file_ids.size());
    auto response_promise = std::make_shared<promise<std::vector<wdx::register_file_provider_response>>>();

    auto proxies = get_provider_proxy_interfaces<file_provider_proxy>(object_ids);
    WC_ASSERT(proxies.size() == object_ids.size());
    std::vector<wdx::reregister_file_provider_request> requests(proxies.size(), {nullptr, "", 0});
    for(size_t i = 0; i < requests.size(); ++i)
    {
        requests[i] = wdx::reregister_file_provider_request(proxies.at(i), file_ids.at(i), contexts.at(i));
    }

    auto future_responses = backend_m.reregister_file_providers(requests, mode);
    future_responses.set_notifier([this, response_promise, object_ids, proxies](std::vector<wdx::register_file_provider_response> responses){
        WC_ASSERT(responses.size() == proxies.size());
        post_provider_register_processing(object_ids, store_m, responses, proxies, registered_file_providers_m);
        response_promise->set_value(std::move(responses));
    });

    return response_promise->get_future();
}

template<class Proxy>
std::vector<Proxy*> remote_backend::get_provider_proxies(std::vector<managed_object_id> object_ids)
{
    // get proxies for managed object ids
    std::vector<Proxy*> proxies;
    proxies.reserve(object_ids.size());
    for(auto obj_id : object_ids) {
        proxies.push_back(&dynamic_cast<Proxy &>(store_m.get_object(obj_id)));
    }
    return proxies;
}

template<class Proxy,
         class Interface>
std::vector<Interface*> remote_backend::get_provider_proxy_interfaces(std::vector<managed_object_id> object_ids)
{
    // get proxy interfaces for managed object ids
    std::vector<Interface*> providers;
    for(auto obj_id : object_ids) {
        Interface &proxy = dynamic_cast<Proxy &>(store_m.get_object(obj_id));
        providers.push_back(&proxy);
    }
    return providers;
}

template <class Proxy>
std::vector<managed_object_id> remote_backend::create_provider_proxies(uint32_t count) 
{
    // create stubs on the other side of IPC
    // before creating proxies on our side
    std::vector<managed_object_id> object_ids(count);
    for (auto &obj_id : object_ids)
    {
        obj_id = store_m.generate_object_id();
        store_m.create_proxy<Proxy>(obj_id);
    }
    WC_DEBUG_LOG(("Created " + std::to_string(count) + " provider proxies of type " + typeid(Proxy).name()).c_str());
    return object_ids;
}

void remote_backend::receive(message_data message)
{
    receiver_m->receive(std::move(message));
}


//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

template<class Response,
         class ProviderType>
static void post_provider_register_processing(std::vector<managed_object_id> const &object_ids,
                                              managed_object_store_i               &store,
                                              std::vector<Response>          const &responses,
                                              std::vector<ProviderType*>     const &proxies,
                                              std::set<ProviderType*>              &registered_providers)
{
    WC_ASSERT(responses.size() == object_ids.size());
    WC_ASSERT(proxies.size()   == object_ids.size());
    for(size_t i = 0; i < responses.size(); ++i)
    {
        log_level_t const level = responses.at(i).has_error() ? log_level_t::error : log_level_t::info;
        wc_log(level, ("[" + store.get_connection_name()
                           + " Stub 0] registered provider (object_id="
                           + std::to_string(object_ids.at(i)) + ") result: "
                           + wdx::to_string(responses.at(i).status)).c_str());
        if(responses.at(i).has_error())
        {
            store.remove_object(object_ids.at(i));
        }
        else
        {
            registered_providers.emplace(proxies.at(i));
        }
    }
}

template<class ProviderType>
static void post_provider_unregister_processing(std::vector<managed_object_id> const &object_ids,
                                                managed_object_store_i               &store,
                                                std::vector<ProviderType*>     const &proxies,
                                                std::set<ProviderType*>              &registered_providers)
{
    WC_ASSERT(proxies.size() == object_ids.size());
    // Set marker in separate loop to keep access window for others to get a lock as small as possible
    std::vector<std::shared_ptr<managed_object_store_i::object_meta>> proxy_meta_vector(object_ids.size());
    for(size_t i = 0; i < object_ids.size(); ++i)
    {
        auto const proxy_id = object_ids.at(i);
        proxy_meta_vector.at(i) = store.get_object_meta(proxy_id);
        WC_ASSERT(proxy_meta_vector.at(i).get() != nullptr); // Proxy should exist
        if(proxy_meta_vector.at(i).get() != nullptr)
        {
            proxy_meta_vector.at(i)->marked_to_destroy = true;
        }
    }
    for(size_t i = 0; i < object_ids.size(); ++i)
    {
        auto const proxy_id = object_ids.at(i);
        WC_DEBUG_LOG(("[" + store.get_connection_name()
                          + " Stub 0] unregistered provider (object_id="
                          + std::to_string(proxy_id) + ")").c_str());
        registered_providers.erase(proxies.at(i));
        if(proxy_meta_vector.at(i).get() != nullptr)
        {
            try
            {
                if(proxy_meta_vector.at(i)->call_counter.fetch_add(1) != 0)
                {
                    // This unique lock constructor locks the given mutex on construction
                    std::unique_lock<std::mutex> lock(proxy_meta_vector.at(i)->object_mutex);
                    proxy_meta_vector.at(i)->call_counter_condition.wait(lock, [&call_counter = proxy_meta_vector.at(i)->call_counter]{
                        return (call_counter == 1);
                    });
                    lock.unlock();
                }
                store.remove_object(proxy_id);
            }
            catch(...)
            {
                wc_log(log_level_t::fatal, ("Failed to remove proxy (object_id="
                                           + std::to_string(proxy_id) + ")").c_str());
                // Catch all exceptions to avoid throwing.
                // - std::lock_guard throws when recursivly locking the same mutex, which
                //   is most certainly not the case.
                WC_FAIL("Unexpected exception caught when unregister providers in remote_backend");
            }
        }
    }
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
