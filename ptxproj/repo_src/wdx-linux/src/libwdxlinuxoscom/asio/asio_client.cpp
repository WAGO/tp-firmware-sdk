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
///  \brief    Implementation of a Boost ASIO client
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio_client.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "system_abstraction_com.hpp"
#include "file_notifier.hpp"

#include <wc/log.h>

#include <memory>
#include <unistd.h>

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
constexpr size_t const g_prevent_busy_waiting_timeout_ms =  100000;
constexpr size_t const await_cleanup_of_dying_server_ms  = 1000000;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
asio_client::asio_client(std::string const &connection_name,
                         std::string const &socket_path)
: asio_client(connection_name, socket_path, std::make_shared<io_context>())
{ }

// NOLINTNEXTLINE(modernize-pass-by-value)
asio_client::asio_client(std::string                 const &connection_name,
                         std::string                 const &socket_path,
                         std::shared_ptr<io_context>        context)
: asio_context(std::move(context))
, connection_name_m(connection_name)
, endpoint_m(socket_path)
, connection_m(nullptr)
, is_connected_m(false)
, mutex_m(std::make_shared<std::mutex>())
, on_connect_m(nullptr)
{ }

asio_client::~asio_client() noexcept = default;

std::shared_ptr<io_context> &asio_client::get_context()
{
    return context_m;
}

bool asio_client::is_connected() const
{
    return is_connected_m;
}

bool asio_client::run_once(uint32_t timeout_ms) // NOLINT(google-default-arguments)
{
    return asio_context::run_once(timeout_ms);
}

bool asio_client::run()
{
    bool run_result = asio_context::run();
    if(!is_connected() && !run_result)
    {
        if(::usleep(g_prevent_busy_waiting_timeout_ms) != 0)
        {
            WC_DEBUG_LOG("usleep was interrupted.");
        }
    }
    return run_result;
}

void asio_client::stop()
{
    asio_context::stop();
}

void asio_client::wait_for_socket()
{
    {
        std::lock_guard<std::mutex> lock_guard(*mutex_m);
        socket_notifier_m = std::make_unique<file_notifier>(*context_m, file_notifier::type::socket, endpoint_m.path());
    }
    wc_log(info, SERVICE_LOG_PREFIX "Created notifier to wait for socket");
    socket_notifier_m->async_wait_for_file([this] (bool success) mutable {
        {
            std::lock_guard<std::mutex> lock_guard(*mutex_m);
            socket_notifier_m.reset();
        }
        if(success)
        {
             do_connect_socket();
        }
        else
        {
            wc_log(log_level_t::warning, (SERVICE_LOG_PREFIX "Socket notifier for \"" + endpoint_m.path() + "\" failed, try again after short time").c_str());
            if(::usleep(g_prevent_busy_waiting_timeout_ms) != 0)
            {
                WC_DEBUG_LOG("usleep was interrupted.");
            }
 
            // Try a new notifier
            wait_for_socket();
        }
    });
}

void asio_client::do_connect_socket()
{
    connection_m->get_socket().async_connect(endpoint_m, [this](error_code code){
        if(!code)
        {
            wc_log(info, (SERVICE_LOG_PREFIX "Connection established (" + connection_name_m + ")").c_str());
            WC_ASSERT(receiver_m.get() != nullptr);
            is_connected_m = true;
            connection_m->receive([this](message_data const &message, std::string error_message) {
                on_receive(message, error_message, connection_m);
            });
            if(on_connect_m != nullptr)
            {
                on_connect_m();
            }
        }
        else if(code == boost::asio::error::connection_refused)
        {
            // case should only happen when paramd was killed or died
            wc_log(info, SERVICE_LOG_PREFIX "Connection refused: Try again in 1 second");
            if(::sleep(1) != 0)
            {
                WC_DEBUG_LOG("sleep was interrupted.");
            }
            do_connect_internal();
        }
        else
        {
            std::string const error_message = SERVICE_LOG_PREFIX "Failed to establish connection (" + connection_name_m + "): " + code.message();
            wc_log(error, error_message.c_str());
            throw wdx::linuxos::com::exception(error_message);
        }
    });
}

void asio_client::do_connect_internal()
{
    bool socket_exists = linuxos::sal::filesystem::get_instance().is_socket_existing(endpoint_m.path());
    if(socket_exists)
    {
        do_connect_socket();
    }
    else
    {
        wait_for_socket();
    }
}


std::unique_ptr<adapter_i> asio_client::do_connect(connect_notifier on_connect)
{
    {
        std::lock_guard<std::mutex> lock_guard(*mutex_m);
        connection_m = std::make_shared<asio_adapter>(*context_m);
        on_connect_m = on_connect;
    }
    do_connect_internal();
    return std::make_unique<protected_adapter>(connection_m, mutex_m);
}

void asio_client::set_receiver(std::unique_ptr<receiver_i> receiver)
{
    receiver_m = std::move(receiver);
}

std::shared_ptr<receiver_i> asio_client::get_receiver(std::shared_ptr<asio_adapter>)
{
    return receiver_m;
}

void asio_client::on_disconnect(std::shared_ptr<asio_adapter>)
{
    is_connected_m = false;
    connection_m->close([this](){
        try_reconnect();
    });
}

void asio_client::try_reconnect()
{
    connection_m->reinit();
    // Avoid fast re-connect to dying process
    if(::usleep(await_cleanup_of_dying_server_ms) != 0)
    {
        WC_DEBUG_LOG("usleep was interrupted.");
    }
    do_connect_internal();
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
