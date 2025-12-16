//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of frontend and backend interface stub components.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/parameter_service_frontend_stub.hpp"
#include "wago/wdx/linuxos/com/parameter_service_file_api_stub.hpp"
#include "wago/wdx/linuxos/com/parameter_service_backend_stub.hpp"
#include "common/definitions.hpp"
#include "common/manager.hpp"
#include "frontend/frontend_stub.hpp"
#include "file_api/file_api_stub.hpp"
#include "backend/backend_stub.hpp"
#include "backend/remote_backend.hpp"
#include "common/server_i.hpp"
#include "asio/asio_server.hpp"
#include "system_abstraction_com.hpp"

#include <wc/trace.h>
#include <wc/log.h>

#include <atomic>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class adapter_i;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
template <class Interface>
class stub_impl
{
private:
    std::atomic<uint64_t> next_connection_index_m;

public:
    std::unique_ptr<server_i>  server_m;
    Interface                 &original_m;
    wc_trace_channels          channel_m;

    stub_impl(std::string       const &connection_name, 
              Interface               &original_interface, 
              std::string       const &socket,
              uid_t                    socket_owner,
              gid_t                    socket_group,
              wc_trace_channels const  channel)
    : next_connection_index_m(0)
    , server_m(nullptr)
    , original_m(original_interface)
    , channel_m(channel)
    {
        try
        {
            WC_DEBUG_LOG((SERVICE_LOG_PREFIX "Created " + connection_name + " stub with ASIO domain sockets as IPC").c_str());
            server_m = std::make_unique<asio_server>(connection_name, socket, socket_owner, socket_group);
        }
        catch(std::exception &e)
        {
            std::string error_message = "Failed to create asio server: ";
            size_t const path_end_pos = socket.find_last_of('/');
            std::string const socket_directory = (path_end_pos == std::string::npos) ? "." : socket.substr(0, path_end_pos);
            if(!linuxos::sal::filesystem::get_instance().is_directory_existing(socket_directory))
            {
                error_message += "Socket directory \"" + socket_directory + "\" missing";
            }
            else
            {
                error_message += e.what();
            }
            throw wdx::linuxos::com::exception(error_message);
        }
        try
        {
            server_m->start([this, connection_name](std::unique_ptr<adapter_i> adapter) {
                auto index = next_connection_index_m++;
                auto info  = adapter->get_connection_info();
                auto mngr  = std::make_unique<manager>(connection_name + "(" + std::to_string(index) + ")", std::move(adapter), *server_m);
                return on_server_start(std::move(mngr), info);
            });
        }
        catch(std::exception &e)
        {
            throw wdx::linuxos::com::exception(std::string("Failed to start asio server: ") + e.what());
        }
    }

    // to be specialized for backend, frontend and file_api
    std::unique_ptr<receiver_i> on_server_start(std::unique_ptr<manager> mngr, connection_info const & connection);
};


//------------------------------------------------------------------------------
// frontend stub class implementation
//------------------------------------------------------------------------------

class parameter_service_frontend_stub_impl : public stub_impl<unauthorized<wdx::parameter_service_frontend_i>>
{
public:
    using stub_impl::stub_impl;
};
template<>
std::unique_ptr<receiver_i> stub_impl<unauthorized<wdx::parameter_service_frontend_i>>::on_server_start(std::unique_ptr<manager> mngr, connection_info const & connection)
{
    auto &store  = mngr->get_object_store();
    store.create_stub<frontend_stub>(frontend_object_id, original_m.authorize(user_data(connection.remote_process_owner)), channel_m);
    return mngr;
}

parameter_service_frontend_stub::parameter_service_frontend_stub(std::string                                     const &connection_name,
                                                                 unauthorized<wdx::parameter_service_frontend_i>       &original_frontend,
                                                                 std::string                                     const &socket_path,
                                                                 uid_t                                                  socket_owner,
                                                                 gid_t                                                  socket_group,
                                                                 wc_trace_channels                               const  channel)
: impl(std::make_unique<parameter_service_frontend_stub_impl>(connection_name, original_frontend, socket_path, socket_owner, socket_group, channel))
{ }

parameter_service_frontend_stub::parameter_service_frontend_stub(parameter_service_frontend_stub&&) noexcept = default;

parameter_service_frontend_stub& parameter_service_frontend_stub::operator=(parameter_service_frontend_stub&&) noexcept = default;

parameter_service_frontend_stub::~parameter_service_frontend_stub() noexcept = default;

bool parameter_service_frontend_stub::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return impl->server_m->run_once(timeout_ms);
}

bool parameter_service_frontend_stub::run()
{
    return impl->server_m->run();
}

void parameter_service_frontend_stub::stop()
{
    impl->server_m->stop();
}

std::vector<int> parameter_service_frontend_stub::get_native_handles()
{
    return impl->server_m->get_native_handles();
}


//------------------------------------------------------------------------------
// file api stub class implementation
//------------------------------------------------------------------------------

class parameter_service_file_api_stub_impl : public stub_impl<unauthorized<wdx::parameter_service_file_api_i>>
{
public:
    using stub_impl::stub_impl;
};
template<>
std::unique_ptr<receiver_i> stub_impl<unauthorized<wdx::parameter_service_file_api_i>>::on_server_start(std::unique_ptr<manager> mngr, connection_info const & connection)
{
    auto &store  = mngr->get_object_store();
    store.create_stub<file_api_stub>(frontend_object_id, original_m.authorize(user_data(connection.remote_process_owner)), channel_m);
    return mngr;
}

parameter_service_file_api_stub::parameter_service_file_api_stub(std::string                                   const &connection_name,
                                                               unauthorized<wdx::parameter_service_file_api_i>       &original_file_api,
                                                               std::string                                     const &socket_path,
                                                               uid_t                                                  socket_owner,
                                                               gid_t                                                  socket_group,
                                                               wc_trace_channels                               const  channel)
: impl(std::make_unique<parameter_service_file_api_stub_impl>(connection_name, original_file_api, socket_path, socket_owner, socket_group, channel))
{ }

parameter_service_file_api_stub::parameter_service_file_api_stub(parameter_service_file_api_stub&&) noexcept = default;

parameter_service_file_api_stub& parameter_service_file_api_stub::operator=(parameter_service_file_api_stub&&) noexcept = default;

parameter_service_file_api_stub::~parameter_service_file_api_stub() noexcept = default;

bool parameter_service_file_api_stub::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return impl->server_m->run_once(timeout_ms);
}

bool parameter_service_file_api_stub::run()
{
    return impl->server_m->run();
}

void parameter_service_file_api_stub::stop()
{
    impl->server_m->stop();
}

std::vector<int> parameter_service_file_api_stub::get_native_handles()
{
    return impl->server_m->get_native_handles();
}


//------------------------------------------------------------------------------
// backend stub class implementation
//------------------------------------------------------------------------------

class parameter_service_backend_stub_impl : public stub_impl<wdx::parameter_service_backend_extended_i>
{
public:
    using stub_impl::stub_impl;
};
template<>
std::unique_ptr<receiver_i> stub_impl<wdx::parameter_service_backend_extended_i>::on_server_start(std::unique_ptr<manager> mngr, connection_info const &)
{
    auto &store  = mngr->get_object_store();
    auto backend = std::make_unique<remote_backend>(original_m, std::move(mngr));
    store.create_stub<backend_stub>(backend_object_id, static_cast<remote_backend_i&>(*backend), channel_m);
    return backend;
}

parameter_service_backend_stub::parameter_service_backend_stub(std::string                               const &connection_name,
                                                               wdx::parameter_service_backend_extended_i       &original_backend,
                                                               std::string                               const &socket_path,
                                                               uid_t                                            socket_owner,
                                                               gid_t                                            socket_group,
                                                               wc_trace_channels                          const  channel)
: impl(std::make_unique<parameter_service_backend_stub_impl>(connection_name, original_backend, socket_path, socket_owner, socket_group, channel))
{ }

parameter_service_backend_stub::parameter_service_backend_stub(parameter_service_backend_stub&&) noexcept = default;

parameter_service_backend_stub& parameter_service_backend_stub::operator=(parameter_service_backend_stub&&) noexcept = default;

parameter_service_backend_stub::~parameter_service_backend_stub() noexcept = default;

bool parameter_service_backend_stub::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return impl->server_m->run_once(timeout_ms);
}

bool parameter_service_backend_stub::run()
{
    return impl->server_m->run();
}

void parameter_service_backend_stub::stop()
{
    impl->server_m->stop();
}

std::vector<int> parameter_service_backend_stub::get_native_handles()
{
    return impl->server_m->get_native_handles();
}

} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
