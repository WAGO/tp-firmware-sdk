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
///  \brief    Implementation of a Boost ASIO communication context (real IPC)
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio_server.hpp"
#include "protected_adapter.hpp"
#include "system_abstraction_com.hpp"
#include "wago/wdx/linuxos/com/definitions.hpp"
#include "errno_utils.hpp"

#include <wc/log.h>

#include <memory>

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
protocol::acceptor open_new_socket(std::string const &socket_path,
                                   io_context        &context,
                                   uid_t              socket_owner,
                                   gid_t              socket_group);
#ifdef SYSTEMD_INTEGRATION
protocol::acceptor get_systemd_socket(std::string const &socket_name,
                                      io_context        &context);
#endif

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
//NOLINTNEXTLINE(modernize-pass-by-value)
asio_server::asio_server(std::string const &connection_name,
                         std::string const &socket,
                         uid_t              socket_owner,
                         gid_t              socket_group)
: asio_context(std::make_shared<io_context>())
, connection_name_m(connection_name)
, acceptor_m(*context_m)
{
    // In case of an absolute or relative (starting with dot) file system path we create the socket
    if(!socket.empty() && ((socket[0] == '/') || (socket[0] == '.')))
    {
        wc_log(log_level_t::info, (SERVICE_LOG_PREFIX "IPC socket path given, try to open new socket: " + socket).c_str());
        acceptor_m = open_new_socket(socket, *context_m, socket_owner, socket_group);
    }

    // In case of a name we expect to get a file descriptor from systemd
    else
    {
        wc_log(log_level_t::info, (SERVICE_LOG_PREFIX "IPC socket name given, try to find socket file descriptor: \"" + socket + "\"").c_str());
#ifdef SYSTEMD_INTEGRATION
        acceptor_m = get_systemd_socket(socket, *context_m);

#else // no SYSTEMD_INTEGRATION (legacy daemon)
        throw std::runtime_error("Integration for systemd provided sockets is missing");
                         
#endif // SYSTEMD_INTEGRATION
    }
}

// cannot unlink sockets because paramd is not root anymore
asio_server::~asio_server() noexcept = default;

bool asio_server::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return asio_context::run_once(timeout_ms);
}

bool asio_server::run()
{
    return asio_context::run();
}

void asio_server::stop()
{
    asio_context::stop();
}

void asio_server::start(connection_handler<receiver_i> handler)
{
    do_accept(handler);
}

std::vector<int> asio_server::get_native_handles()
{
    std::lock_guard<std::mutex> lock(connections_mutex_m);

    std::vector<int> handles;
    handles.push_back(acceptor_m.native_handle());
    for (auto &cnx : connections_m)
    {
        handles.push_back(cnx.first->get_socket().native_handle());
    }
    return handles;
}

void asio_server::do_accept(connection_handler<receiver_i> handler)
{
    auto new_connection = std::make_shared<asio_adapter>(*context_m);
    wc_log(log_level_t::info, (SERVICE_LOG_PREFIX "Try to accept connections for "
                               "socket file descriptor: \"" + std::to_string(acceptor_m.native_handle()) + "\"").c_str());
    acceptor_m.async_accept(new_connection->get_socket(), [this, new_connection, handler = std::move(handler)](error_code code){
        on_accept(code, new_connection, handler);
    });
}
void asio_server::on_accept(error_code                     const &code,
                            std::shared_ptr<asio_adapter>         new_connection,
                            connection_handler<receiver_i>        handler)
{
    if(!code)
    {
        auto mutex   = std::make_shared<std::mutex>();
        auto adapter = std::make_unique<protected_adapter>(new_connection, mutex);
        {
            std::lock_guard<std::mutex> lock(connections_mutex_m);
            connections_m.emplace(new_connection, handler(std::move(adapter)));
        }
        wc_log(info, (SERVICE_LOG_PREFIX "New connection established (" + connection_name_m + "), current connection count: " +
                      std::to_string(connections_m.size())).c_str());
        new_connection->receive([this, new_connection](message_data const &new_message, std::string new_error_message){
            on_receive(new_message, new_error_message, new_connection);
        });
    }
    else
    {
        wc_log(warning, (SERVICE_LOG_PREFIX "Failed to establish connection (" + connection_name_m + "): " + code.message()).c_str());
    }
    do_accept(std::move(handler));
}
std::shared_ptr<receiver_i> asio_server::get_receiver(std::shared_ptr<asio_adapter> adapter)
{
    std::shared_ptr<receiver_i> receiver;
    {
        std::lock_guard<std::mutex> lock(connections_mutex_m);
        receiver = connections_m.at(adapter);
    }
    return receiver;
}

void asio_server::on_disconnect(std::shared_ptr<asio_adapter> adapter)
{
    // TODO: check if we need to protect the sender interface which
    //       may still be used from another thread.
    {
        std::lock_guard<std::mutex> lock(connections_mutex_m);
        connections_m.erase(adapter);
    }
    wc_log(info, (SERVICE_LOG_PREFIX "Current connection count (" + connection_name_m + "): " + std::to_string(connections_m.size())).c_str());
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
protocol::acceptor open_new_socket(std::string const &socket_path,
                                   io_context        &context,
                                   uid_t              socket_owner,
                                   gid_t              socket_group)
{
    auto tmp_socket_path = socket_path + ".tmp";

    auto &fs = linuxos::sal::filesystem::get_instance();

    // Cleanup if old socket is still existing
    fs.unlink(socket_path.c_str());
    fs.unlink(tmp_socket_path.c_str());

    auto acceptor = protocol::acceptor(context, protocol::endpoint(tmp_socket_path));

    if(wc_get_log_level() == log_level_t::debug)
    {
        if(fs.is_socket_existing(tmp_socket_path))
        {
            wc_log(log_level_t::debug, (SERVICE_LOG_PREFIX "Socket \"" + tmp_socket_path + "\" created.").c_str());
        }
        else
        {
            wc_log(log_level_t::error, (SERVICE_LOG_PREFIX "Failed to create socket \"" + tmp_socket_path + "\".").c_str());
            throw std::runtime_error("Socket not available 1");
        }
    }

    // Apply owner and group if provided
    if((socket_owner != asio_server::invalid_uid) || (socket_group != asio_server::invalid_gid))
    {
        if(fs.chown(tmp_socket_path.c_str(), socket_owner, socket_group) < 0)
        {
            throw std::runtime_error(SERVICE_LOG_PREFIX "Failed to change socket owner for "
                                     "\"" + tmp_socket_path + "\""
                                     ": " + errno_to_string(errno) + " "
                                     "("  + std::to_string( errno) + ")");
        }
        
    }

    constexpr auto mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP); // 660
    if(fs.chmod(tmp_socket_path.c_str(), mode) < 0)
    {
        throw std::runtime_error(SERVICE_LOG_PREFIX "Failed to change socket mode for "
                                 "\"" + tmp_socket_path + "\""
                                 ": " + errno_to_string(errno) + " "
                                 "("  + std::to_string( errno) + ")");
    }

    if(fs.rename(tmp_socket_path.c_str(), socket_path.c_str()) < 0)
    {
        throw std::runtime_error(SERVICE_LOG_PREFIX "Failed to rename socket for"
                                 "\"" + tmp_socket_path + "\""
                                 ": " + errno_to_string(errno) + " "
                                 "("  + std::to_string( errno) + ")");
    }

    return acceptor;
}

#ifdef SYSTEMD_INTEGRATION
protocol::acceptor get_systemd_socket(std::string const &socket_name,
                                      io_context        &context)
{
    int  socket_fd = linuxos::sal::systemd::get_instance().find_systemd_socket(socket_name);
    auto acceptor  = protocol::acceptor(context, protocol(), socket_fd);
    if((acceptor.native_handle() != socket_fd) || (acceptor.native_handle() < 1))
    {
        throw std::runtime_error(SERVICE_LOG_PREFIX "Failed to create IPC acceptor with "
                                 "file descriptor FD" + std::to_string(socket_fd) + " "
                                 "for socket: \""     + socket_name + "\"");
    }
    if(!acceptor.is_open())
    {
        throw std::runtime_error(SERVICE_LOG_PREFIX "Acceptor socket is not open: \"" + socket_name + "\"");
    }
    if(!acceptor.non_blocking())
    {
        wc_log(log_level_t::warning, SERVICE_LOG_PREFIX "Acceptor has a blocking socket, try to change.");

        auto &fs = linuxos::sal::filesystem::get_instance();
        int flags = fs.fcntl(socket_fd, F_GETFL, 0);
        if(flags < 0)
        {
            throw std::runtime_error(SERVICE_LOG_PREFIX "Error on getting flags for "
                                     "socket \"" + socket_name + "\""
                                     ": " + errno_to_string(errno) + " "
                                     "("  + std::to_string( errno) + ")");
        }
        if(fs.fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            throw std::runtime_error(SERVICE_LOG_PREFIX "Error changing flags for "
                                     "socket \"" + socket_name + "\""
                                     ": " + errno_to_string(errno) + " "
                                     "("  + std::to_string( errno) + ")");
        }
    }

    return acceptor;
}
#endif // SYSTEMD_INTEGRATION


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
