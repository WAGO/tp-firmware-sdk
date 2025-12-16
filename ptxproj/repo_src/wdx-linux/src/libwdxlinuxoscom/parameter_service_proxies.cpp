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
///  \brief    Implementation of parameter service proxy components.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_frontend_proxy.hpp"
#include "wago/wdx/linuxos/com/parameter_service_file_api_proxy.hpp"
#include "wago/wdx/linuxos/com/parameter_service_backend_proxy.hpp"
#include "wago/wdx/linuxos/com/parameter_service_proxy.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "common/definitions.hpp"
#include "common/manager.hpp"
#include "frontend/frontend_proxy.hpp"
#include "file_api/file_api_proxy.hpp"
#include "file_api/file_api_chunking_handler.hpp"
#include "backend/backend_proxy.hpp"
#include "backend/registered_device_store.hpp"
#include "common/client_i.hpp"
#include "asio/asio_client.hpp"

#include <wc/structuring.h>
#include <wc/trace.h>
#include <wc/log.h>

#include <atomic>
#include <mutex>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

constexpr size_t g_file_api_read_chunk_size  =  128 * 1024; // 128 kiB
constexpr size_t g_file_api_write_chunk_size = 2048 * 1024; //   2 MiB

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
template <class InterfaceType>
class proxy_impl
{
public:
    std::unique_ptr<client_i>     client_m;
    manager                      &manager_m;
    std::atomic<InterfaceType *>  proxy_m;

    proxy_impl(std::string const &connection_name,
               std::string const &socket_path,
               connect_notifier   on_connect   = nullptr,
               client_i          *other_client = nullptr)
    : client_m(other_client == nullptr ? std::make_unique<asio_client>(connection_name, socket_path)
                                       : std::make_unique<asio_client>(connection_name, socket_path, static_cast<asio_client*>(other_client)->get_context()))
    , manager_m(client_m->connect<manager>([this, connection_name](std::unique_ptr<adapter_i> adapter) {
        return std::make_unique<manager>(connection_name, std::move(adapter), *client_m);
    }, on_connect))
    { }

    InterfaceType &get_interface()
    {
        auto interface_ptr = proxy_m.load();
        if(interface_ptr == nullptr)
        {
            throw std::logic_error("Tried to load interface without setting a proxy");
        }
        return *(interface_ptr);
    }

    void set_proxy(InterfaceType *proxy)
    {
        proxy_m = proxy;
    }
};

//------------------------------------------------------------------------------
// frontend implementations
//------------------------------------------------------------------------------
class parameter_service_frontend_proxy_impl : public proxy_impl<frontend_proxy::interface>
{
public:
    parameter_service_frontend_proxy_impl(std::string const &connection_name,
                                          std::string const &socket_path,
                                          client_i          *other_client = nullptr)
    : proxy_impl(connection_name, socket_path, nullptr, other_client)
    {
        set_proxy(&(manager_m.get_object_store().create_proxy<frontend_proxy>(frontend_object_id)));
    }
};

class parameter_service_file_api_proxy_impl : public proxy_impl<file_api_proxy::interface>
{
private:
    file_api_chunking_handler chunking_handler_m;

public:
    parameter_service_file_api_proxy_impl(std::string const &connection_name,
                                          std::string const &socket_path,
                                          client_i          *other_client = nullptr)
    : proxy_impl(connection_name, socket_path, nullptr, other_client)
    , chunking_handler_m(
          // the chunking handler will get a reference to the original file api stub.
          // as the lifetime of the stub is bound to the manager/object_store, it will be
          // valid for the whole chunking handler's lifetime, as that member gets destructed
          // first. Any open async notifiers in the chunking handlers implementation will
          // be called latest while destructing the manager/object_store/stub.
          manager_m.get_object_store().create_proxy<file_api_proxy>(file_api_object_id),
          g_file_api_read_chunk_size, g_file_api_write_chunk_size
      )
    {
        WC_DEBUG_LOG(SERVICE_LOG_PREFIX "Created file api proxy with ASIO domain sockets as IPC");
        set_proxy(&chunking_handler_m);
    }
};

//------------------------------------------------------------------------------
// backend implementation
//------------------------------------------------------------------------------
class parameter_service_backend_proxy_impl : public proxy_impl<backend_proxy>
{
public:
    using connect_notifier = parameter_service_backend_proxy::connect_notifier;
    using notifier_handle  = backend_notification_manager_i::notifier_handle;

    std::vector<connect_notifier> notifier_store_m;
    std::mutex                    notifier_store_mutex_m;
    std::atomic<bool>             pending_on_connect_m;
    registered_device_store       device_store_m;
    wc_trace_channels             channel_m;

    parameter_service_backend_proxy_impl(std::string       const &connection_name,
                                         std::string       const &socket_path,
                                         wc_trace_channels const  channel)
    : parameter_service_backend_proxy_impl(connection_name, socket_path, nullptr, channel)
    { }

    parameter_service_backend_proxy_impl(std::string       const &connection_name,
                                         std::string       const &socket_path,
                                         client_i                *other_client,
                                         wc_trace_channels const  channel)
    : proxy_impl(connection_name, socket_path, [this]() { on_connect_notifier(); }, other_client)
    , pending_on_connect_m(false)
    , device_store_m(connection_name)
    , channel_m(channel)
    {
        WC_DEBUG_LOG(SERVICE_LOG_PREFIX "Created backend proxy with ASIO domain sockets as IPC");
        set_proxy(&(manager_m.get_object_store().create_proxy<backend_proxy>(backend_object_id,
                                                                             manager_m.get_object_store(),
                                                                             device_store_m,
                                                                             channel_m)));
    }

    notifier_handle add_notifier(connect_notifier notifier)
    {
        if(client_m->is_connected())
        {
            auto *backend = proxy_m.load();
            if(backend != nullptr)
            {
                notifier(*backend);
            }
        }
        notifier_handle element_index = 0;
        {
            std::lock_guard<std::mutex> lock_guard(notifier_store_mutex_m);
            notifier_store_m.push_back(std::move(notifier));
            element_index = notifier_store_m.size() - 1;
        }

        return element_index;
    }

    void remove_notifier(notifier_handle handle)
    {
        std::lock_guard<std::mutex> lock_guard(notifier_store_mutex_m);
        if(handle >= notifier_store_m.size())
        {
            throw wdx::linuxos::com::exception("Invalid notifier handle");
        }
        notifier_store_m[handle] = [](wdx::parameter_service_backend_i &){
            // NOOP
        };
    }

    void on_connect_notifier()
    {
        auto *backend = proxy_m.load();
        if(backend != nullptr)
        {
            pending_on_connect_m = false;

            backend->reregister_devices();
            backend->reregister_providers();

            std::lock_guard<std::mutex> lock_guard(notifier_store_mutex_m);
            for(auto const &notifier : notifier_store_m)
            {
                notifier(*backend);
            }
        }
        else
        {
            pending_on_connect_m = true;
        }
    }
};

//------------------------------------------------------------------------------
// frontend proxy class implementation
//------------------------------------------------------------------------------
parameter_service_frontend_proxy::parameter_service_frontend_proxy(std::string const &connection_name)
: parameter_service_frontend_proxy(connection_name, WC_ARRAY_TO_PTR(frontend_domain_socket))
{}

parameter_service_frontend_proxy::parameter_service_frontend_proxy(std::string const &connection_name,
                                                                   std::string const &socket_path)
: impl(std::make_unique<parameter_service_frontend_proxy_impl>(connection_name, socket_path))
{}

parameter_service_frontend_proxy::parameter_service_frontend_proxy(parameter_service_frontend_proxy&&) noexcept = default;

parameter_service_frontend_proxy& parameter_service_frontend_proxy::operator=(parameter_service_frontend_proxy&&) noexcept = default;

parameter_service_frontend_proxy::~parameter_service_frontend_proxy() noexcept = default;

bool parameter_service_frontend_proxy::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return impl->client_m->run_once(timeout_ms);
}

bool parameter_service_frontend_proxy::run()
{
    return impl->client_m->run();
}

void parameter_service_frontend_proxy::stop()
{
    impl->client_m->stop();
}

wdx::parameter_service_frontend_i &parameter_service_frontend_proxy::get_frontend()
{
    return impl->get_interface();
}

//------------------------------------------------------------------------------
// file api proxy class implementation
//------------------------------------------------------------------------------
parameter_service_file_api_proxy::parameter_service_file_api_proxy(std::string const &connection_name)
: parameter_service_file_api_proxy(connection_name, WC_ARRAY_TO_PTR(file_api_domain_socket))
{}

parameter_service_file_api_proxy::parameter_service_file_api_proxy(std::string const &connection_name,
                                                                   std::string const &socket_path)
: impl(std::make_unique<parameter_service_file_api_proxy_impl>(connection_name, socket_path))
{}

parameter_service_file_api_proxy::parameter_service_file_api_proxy(parameter_service_file_api_proxy&&) noexcept = default;

parameter_service_file_api_proxy& parameter_service_file_api_proxy::operator=(parameter_service_file_api_proxy&&) noexcept = default;

parameter_service_file_api_proxy::~parameter_service_file_api_proxy() noexcept = default;

bool parameter_service_file_api_proxy::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return impl->client_m->run_once(timeout_ms);
}

bool parameter_service_file_api_proxy::run()
{
    return impl->client_m->run();
}

void parameter_service_file_api_proxy::stop()
{
    impl->client_m->stop();
}

wdx::parameter_service_file_api_i &parameter_service_file_api_proxy::get_file_api()
{
    return impl->get_interface();
}

//------------------------------------------------------------------------------
// backend proxy class implementation
//------------------------------------------------------------------------------
parameter_service_backend_proxy::parameter_service_backend_proxy(std::string const &connection_name)
: parameter_service_backend_proxy(connection_name, WC_ARRAY_TO_PTR(backend_domain_socket))
{}

parameter_service_backend_proxy::parameter_service_backend_proxy(std::string       const &connection_name,
                                                                 std::string       const &socket_path,
                                                                 wc_trace_channels const  channel)
: impl(std::make_unique<parameter_service_backend_proxy_impl>(connection_name, socket_path, channel))
{}

parameter_service_backend_proxy::parameter_service_backend_proxy(parameter_service_backend_proxy&&) noexcept = default;

parameter_service_backend_proxy& parameter_service_backend_proxy::operator=(parameter_service_backend_proxy&&) noexcept = default;

parameter_service_backend_proxy::~parameter_service_backend_proxy() noexcept =  default;

bool parameter_service_backend_proxy::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    if(impl->pending_on_connect_m)
    {
        impl->on_connect_notifier();
        return true;
    }
    return impl->client_m->run_once(timeout_ms);
}

bool parameter_service_backend_proxy::run()
{
    if(impl->pending_on_connect_m)
    {
        impl->on_connect_notifier();
        return true;
    }
    return impl->client_m->run();
}

void parameter_service_backend_proxy::stop()
{
    impl->client_m->stop();
}

parameter_service_backend_proxy::notifier_handle parameter_service_backend_proxy::add_notifier(connect_notifier notifier)
{
    return impl->add_notifier(notifier);
}

void parameter_service_backend_proxy::remove_notifier(parameter_service_backend_proxy::notifier_handle handle)
{
    impl->remove_notifier(handle);
}

wdx::parameter_service_backend_i &parameter_service_backend_proxy::get_backend()
{
    return impl->get_interface();
}

backend_notification_manager_i &parameter_service_backend_proxy::get_notification_manager()
{
    return *this;
}

//------------------------------------------------------------------------------
// combined proxy class implementation
//------------------------------------------------------------------------------
parameter_service_proxy::parameter_service_proxy(std::string const &connection_name)
: parameter_service_proxy(connection_name, WC_ARRAY_TO_PTR(frontend_domain_socket),
                                           WC_ARRAY_TO_PTR(file_api_domain_socket),
                                           WC_ARRAY_TO_PTR(backend_domain_socket))
{}

// TODO: marked as deprecated. implementation may be removed soon(tm)
parameter_service_proxy::parameter_service_proxy(std::string const &connection_name,
                                                 std::string const &frontend_socket_path,
                                                 std::string const &backend_socket_path)
: parameter_service_proxy(connection_name, frontend_socket_path, "", backend_socket_path)
{ }

parameter_service_proxy::parameter_service_proxy(std::string const &connection_name,
                                                 std::string const &frontend_socket_path,
                                                 std::string const &file_api_socket_path,
                                                 std::string const &backend_socket_path)
: frontend_impl(std::make_unique<parameter_service_frontend_proxy_impl>(connection_name, frontend_socket_path))
, file_api_impl(file_api_socket_path.empty() ? nullptr : 
                std::make_unique<parameter_service_file_api_proxy_impl>(connection_name, file_api_socket_path, 
                                                                        frontend_impl->client_m.get()))
, backend_impl(std::make_unique<parameter_service_backend_proxy_impl>(connection_name, backend_socket_path,
                                                                      frontend_impl->client_m.get(),
                                                                      wc_trace_channels::invalid))
{ }

parameter_service_proxy::parameter_service_proxy(parameter_service_proxy&&) noexcept = default;

parameter_service_proxy& parameter_service_proxy::operator=(parameter_service_proxy&&) noexcept = default;

parameter_service_proxy::~parameter_service_proxy() noexcept = default;

bool parameter_service_proxy::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    if(backend_impl->pending_on_connect_m)
    {
        backend_impl->on_connect_notifier();
        return true;
    }
    return backend_impl->client_m->run_once(timeout_ms);
}

bool parameter_service_proxy::run()
{
    if(backend_impl->pending_on_connect_m)
    {
        backend_impl->on_connect_notifier();
        return true;
    }
    return backend_impl->client_m->run();
}

void parameter_service_proxy::stop()
{
    backend_impl->client_m->stop();
}

parameter_service_proxy::notifier_handle parameter_service_proxy::add_notifier(connect_notifier notifier)
{
    return backend_impl->add_notifier(notifier);
}

void parameter_service_proxy::remove_notifier(parameter_service_proxy::notifier_handle handle)
{
    backend_impl->remove_notifier(handle);
}

wdx::parameter_service_frontend_i &parameter_service_proxy::get_frontend()
{
    return frontend_impl->get_interface();
}

wdx::parameter_service_file_api_i &parameter_service_proxy::get_file_api()
{
    if (file_api_impl == nullptr)
    {
        throw std::runtime_error("you've used a deprecated constructor and didn't provide a socket path for the file api");
    }
    return file_api_impl->get_interface();
}

wdx::parameter_service_backend_i &parameter_service_proxy::get_backend()
{
    return backend_impl->get_interface();
}

backend_notification_manager_i &parameter_service_proxy::get_notification_manager()
{
    return *this;
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
