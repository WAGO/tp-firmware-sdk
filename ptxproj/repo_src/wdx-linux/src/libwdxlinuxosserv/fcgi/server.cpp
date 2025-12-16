//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    FCGI server for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "server.hpp"
#include "service_request.hpp"
#include "system_abstraction_serv.hpp"
#include "errno_utils.hpp"

#include <wago/wdx/wda/http/request_handler_i.hpp>
#include <wago/wdx/wda/trace_routes.hpp>
#include <wc/log.h>

#include <stdexcept>
#include <memory>
#include <cstring>

#include <fcntl.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace fcgi {

using wdx::wda::http::request_handler_i;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
int open_new_socket(std::string const &fcgi_socket_path, int count_of_waiting_sockets);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
server::server(std::string       const &fcgi_socket,
               trace_route       const  route,
               request_handler_i       &handler)
: route_m(route)
, handler_m(handler)
{
    if(sal::fcgi::get_instance().Init() != 0)
    {
        throw std::runtime_error("Failed to initialize FCGI");
    }

    // In case of an absolute or relative (starting with dot) file system path we create the socket
    if(!fcgi_socket.empty() && ((fcgi_socket[0] == '/') || (fcgi_socket[0] == '.')))
    {
        wc_log(log_level_t::info, ("FCGI socket path given, try to open new socket: " + fcgi_socket).c_str());
        fcgi_socket_m = open_new_socket(fcgi_socket, count_of_waiting_sockets);
    }

    // In case of a name we expect to get a file descriptor from systemd
    else
    {
        wc_log(log_level_t::info, ("FCGI socket name given, try to find socket file descriptor: \"" + fcgi_socket + "\"").c_str());
#ifdef SYSTEMD_INTEGRATION
        fcgi_socket_m = linuxos::sal::systemd::get_instance().find_systemd_socket(fcgi_socket);

#else // no SYSTEMD_INTEGRATION (legacy daemon)
        throw std::runtime_error("Integration for systemd provided sockets is missing");
                         
#endif // SYSTEMD_INTEGRATION
    }
}

server::~server() noexcept
{
    sal::fcgi::get_instance().ShutdownPending();
    linuxos::sal::filesystem::get_instance().close(fcgi_socket_m);
}

int server::get_native_handle() const
{ 
    return fcgi_socket_m;
}

// FIXME: Move macro to common header
#define WC_PTR_FROM_ARRAY(array)\
    (&(array)[0])

bool server::receive_next(uint32_t timeout)
{
    bool result = false;

    // only continue, if there is anything to receive
    // on the FCGI socket
    timespec start_time;
    linuxos::serv::sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &start_time);
    pollfd fds[] = {{fcgi_socket_m, POLLIN, 0}};
    WC_ASSERT(timeout <= INT_MAX);
    linuxos::sal::filesystem::get_instance().poll(WC_PTR_FROM_ARRAY(fds), 1, static_cast<int>(timeout));
    if ((fds[0].revents & POLLIN) != 0)
    {
        wc_trace_start_channel(for_route(route_m));
        WC_TRACE_SET_MARKER(for_route(route_m), "FCGI Server: receive request");
        auto request = std::make_unique<service_request>(fcgi_socket_m, route_m);
        if(request->accept())
        {
            handler_m.handle(std::move(request));
        }
        result = true;
    }
    else
    {
        timespec end_time;
        linuxos::serv::sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &end_time);
        auto elapsed_time_ms = static_cast<uint32_t>(end_time.tv_sec - start_time.tv_sec) * 1'000u +
                               static_cast<uint32_t>(end_time.tv_nsec - start_time.tv_nsec) / 1'000'000u;

        if (elapsed_time_ms < timeout)
        {
            WC_DEBUG_LOG("fcgi::server::receive_next: timeout not reached in "
                         "error case in poll, sleeping for the rest of the time");
            linuxos::serv::sal::system_clock::get_instance().sleep_for(timeout - elapsed_time_ms);
        }
    }

    return result;
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
int open_new_socket(std::string const &fcgi_socket_path, int count_of_waiting_sockets)
{
    int fd = sal::fcgi::get_instance().OpenSocket(fcgi_socket_path.c_str(), count_of_waiting_sockets);
    if(fd < 0)
    {
        throw std::runtime_error("Failed to open FCGI socket");
    }
    if(linuxos::sal::filesystem::get_instance().fcntl(fd, F_SETFD, O_CLOEXEC) < 0)
    {
        throw std::runtime_error("Failed to set \"socket close on execution\" flag (O_CLOEXEC) for security reasons");
    }
    if(linuxos::sal::filesystem::get_instance().chown(fcgi_socket_path.c_str(), "www", "www") < 0)
    {
        throw std::runtime_error(std::string("Failed to change socket owner/group to www: ") + errno_to_string(errno));
    }

    return fd;
}


} // Namespace fcgi
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
