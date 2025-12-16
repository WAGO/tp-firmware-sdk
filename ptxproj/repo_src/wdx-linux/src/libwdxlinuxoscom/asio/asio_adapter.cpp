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
///  \brief    Implementation of Boost ASIO IPC adapter.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio_adapter.hpp"
#include "wago/wdx/linuxos/com/exception.hpp"
#include "system_abstraction_com.hpp"
#include "common/definitions.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <boost/version.hpp>
#include <algorithm>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

#define boost_version_min(major, minor) (\
    (BOOST_VERSION / 100000) > major || ((BOOST_VERSION / 100000) == major && (BOOST_VERSION / 100 % 1000) >= minor)\
)

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using boost::system::error_code;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

asio_adapter::asio_adapter(boost::asio::io_context &context)
: socket_m(context)
, current_in_message_size_m(0)
, in_count_m(0)
, close_handler_m(nullptr)
, exit_marker_m(std::make_shared<std::atomic<bool>>(false))
, exit_mutex_m(std::make_shared<std::mutex>())
{

}

asio_adapter::~asio_adapter() noexcept
{
    *exit_marker_m = true;

    try
    {
        std::lock_guard<std::mutex> exit_lock_guard(*exit_mutex_m);
        std::lock_guard<std::mutex> lock_guard(out_mutex_m);
        // parasoft-begin-suppress CERT_CPP-DCL57-a-2 CERT_CPP-ERR50-b-3 "empty() and pop() do not throw"
        while (!out_queue_m.empty())
        {
            out_queue_m.pop();
        }
        // parasoft-end-suppress CERT_CPP-DCL57-a-2 CERT_CPP-ERR50-b-3
    }
    catch(...)
    {
        // Catch all exceptions to avoid throwing from destructor.
        // - std::lock_guard throws when recursivly locking the same mutex, which
        //   is most certainly not the case.
        WC_FAIL("Unexpected exception caught in asio_adapter destructor");
    }
}

protocol::socket & asio_adapter::get_socket()
{
    return socket_m;
}

void asio_adapter::close(close_handler handler)
{
    bool queue_empty = false;
    {
        std::lock_guard<std::mutex> lock_guard(out_mutex_m);
        close_handler_m = std::move(handler);
        queue_empty = out_queue_m.empty();
    }
    if(queue_empty)
    {
        close_handler_m();
    }
}

void asio_adapter::reinit()
{
    close([this] {
        std::lock_guard<std::mutex> lock_guard(out_mutex_m);
// from boost 1.69 on, the socket's executor is to be used
#if boost_version_min(1, 69)
        auto &context = socket_m.get_executor();
#else
        auto &context = socket_m.get_io_context();
#endif  
        socket_m = protocol::socket(context);
        current_in_message_size_m = 0;
        current_in_message_m.clear();
        WC_ASSERT(out_queue_m.empty());
        WC_ASSERT(in_count_m == 0);
        close_handler_m = nullptr;
    });
}

bool asio_adapter::is_closed()
{
    std::lock_guard<std::mutex> lock_guard(out_mutex_m);
    return (close_handler_m != nullptr);
}

void asio_adapter::send(message_data &&message,
                        send_handler   handler)
{
    bool start_write_operation = false;
    {
        std::lock_guard<std::mutex> lock_guard(out_mutex_m);
        start_write_operation = out_queue_m.empty();
        auto message_size = message.size();
        out_queue_m.push(std::make_tuple(std::move(message), message_size, handler));
    }

    if(start_write_operation)
    {
        start_write();
    }
}

void asio_adapter::start_write()
{
    out_mutex_m.lock(); // parasoft-suppress CERT_CPP-CON51-a-2 "Avoid copy of element instead of reference when using lockguard in scope"
    WC_ASSERT(!out_queue_m.empty());
    send_element const &element      = out_queue_m.front();
    out_mutex_m.unlock();
    auto   const &message            = std::get<message_data>(element);
    auto   const &message_size       = std::get<uint32_t>(element);
    auto   const &handler            = std::get<send_handler>(element);
    size_t const message_size_length = sizeof(message_size);
    auto   const message_size_buffer = boost::asio::buffer(&message_size, message_size_length);
    auto   const message_buffer      = boost::asio::buffer(message);
    std::array<typeof(message_buffer), 2> buffers = {{ message_size_buffer, message_buffer }};
    // FIXME: Move message & message size to shared_ptr to avoid invalid access from async_write?
    boost::asio::async_write(socket_m, buffers, [this, exit_marker = exit_marker_m, exit_mutex = exit_mutex_m, total_size = (message_size + message_size_length), handler](error_code code, size_t length){
        // Check for parallel destruction of "this" adapter
        if(*exit_marker)
        {
            return;
        }
        std::lock_guard<std::mutex> exit_lock_guard(*exit_mutex);
        if(*exit_marker)
        {
            return;
        }

        // Adapter not destroyed, execute handler
        bool write_more = false;
        {
            std::lock_guard<std::mutex> lock_guard(out_mutex_m);
            out_queue_m.pop();
            write_more = !out_queue_m.empty();
        }
        if(write_more)
        {
            start_write();
        }
        else
        {
            if(is_closed())
            {
                close_handler_m();
            }
        }
        if(!code)
        {
            WC_ASSERT(length == total_size);
            WC_DEBUG_LOG((SERVICE_LOG_PREFIX "Async write handler: Written " + std::to_string(length) + " bytes for header and message").c_str());
            handler("");
        }
        else
        {
            handler("Failed to send message: " + code.message());
        }
    });
}

void asio_adapter::receive(receive_handler handler)
{
    // This implementation does not allow multiple active receive calls
    signed asio_adapter_receive_count = in_count_m.fetch_add(1);
    while(asio_adapter_receive_count > 0)
    {
        in_count_m--;
        if(::usleep(100) != 0)
        {
            WC_DEBUG_LOG("usleep was interrupted.");
        }
        asio_adapter_receive_count = in_count_m.fetch_add(1);
    }
    WC_ASSERT(asio_adapter_receive_count == 0);
    try
    {
        receive_header(handler);
    }
    catch(...)
    {
        in_count_m--;
        throw;
    }
}

void asio_adapter::receive_header(receive_handler handler)
{
    size_t const message_size_length = sizeof(uint32_t);
    auto message_size_buffer = boost::asio::buffer(&current_in_message_size_m, message_size_length);
    boost::asio::async_read(socket_m, message_size_buffer, [this, message_size_length, handler = std::move(handler)](error_code code, std::size_t const length)
    {
        if(!code)
        {
            WC_ASSERT(length == message_size_length);
            try
            {
                receive_body(handler);
            }
            catch(...)
            {
                in_count_m--;
                throw;
            }
        }
        else
        {
            in_count_m--;
            handler(message_data(), "Failed to read message header: " + code.message());
        }
    });
}

void asio_adapter::receive_body(receive_handler handler)
{
    WC_DEBUG_LOG((SERVICE_LOG_PREFIX "New message available with " + std::to_string(current_in_message_size_m) + " bytes").c_str());
    {
        std::lock_guard<std::mutex> lock(current_in_mutex_m);
        current_in_message_m = message_data(current_in_message_size_m);
    }
    auto read_buffer = boost::asio::buffer(current_in_message_m, current_in_message_size_m);
    boost::asio::async_read(socket_m, read_buffer, [this, handler = std::move(handler)](error_code code, std::size_t const length)
    {
        in_count_m--;
        if(!code)
        {
            WC_ASSERT(length == current_in_message_size_m);
            
            message_data complete_message;
            {
                std::lock_guard<std::mutex> lock(current_in_mutex_m);
                complete_message = std::move(current_in_message_m);
            }
            handler(complete_message, "");
        }
        else
        {
            handler(message_data(), "Failed to read message of " + std::to_string(current_in_message_size_m) + " bytes: " + code.message());
        }
    });
}

connection_info asio_adapter::get_connection_info() const
{
    try
    {
        connection_info ci;
        int const socket_handle = const_cast<protocol::socket*>(&socket_m)->native_handle(); // parasoft-suppress CERT_CPP-EXP55-a-2 "Get native handle should not change anything on the socket"
        ci.remote_process_owner = sal::system_sockets::get_instance().get_socket_peer_user(socket_handle);

        return ci;
    }
    catch(std::exception const &e)
    {
        throw com::exception("Failed to determine peer socket user: " + std::string(e.what()));
    }
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
