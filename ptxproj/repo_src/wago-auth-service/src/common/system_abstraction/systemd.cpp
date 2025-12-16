//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for systemd integration.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#if SYSTEMD_INTEGRATION
#include <wc/log.h>

#include <stdexcept>
#include <systemd/sd-daemon.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace sal {

class systemd_impl : public systemd
{
public:
    systemd_impl() noexcept = default;
    ~systemd_impl() noexcept override = default;

    int get_listen_fds_start() const override;
    int get_listen_fds_with_names(char ***fd_names) const override;
    int check_for_socket_unix(int fd) const override;
    int find_systemd_socket(std::string const &socket_name) const override;

    void notify_ready() const override;
    void notify_stopping() const override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static systemd_impl default_systemd;

systemd *systemd::instance = &default_systemd;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
int systemd_impl::get_listen_fds_start() const
{
    return SD_LISTEN_FDS_START;
}

int systemd_impl::get_listen_fds_with_names(char ***fd_names) const
{
    int const unset_environment = 0;
    return ::sd_listen_fds_with_names(unset_environment, fd_names);
}

int systemd_impl::check_for_socket_unix(int fd) const
{
    int    const  family    = AF_UNIX;
    int    const  type      = SOCK_STREAM;
    int    const  listening = 1;
    return sd_is_socket(fd, family, type, listening);
}

int systemd_impl::find_systemd_socket(std::string const &socket_name) const
{
    char      **fd_names;
    int const   fd_count = get_listen_fds_with_names(&fd_names);
    if(fd_count < 0)
    {
        throw std::runtime_error("Failed to receive any socket file descriptor");
    }
    if(fd_count < 1)
    {
        throw std::runtime_error("Did not receive any socket file descriptor");
    }
    wc_log(log_level_t::info, "Got " + std::to_string(fd_count) + " file descriptor(s) from systemd.");

    int fd = 0;
    for(size_t i = 0; (i < fd_count) && (fd_names[i] != nullptr); ++i)
    {
        if(socket_name == fd_names[i])
        {
            fd = get_listen_fds_start() + i;
            wc_log(log_level_t::info, "Found socket file descriptor for \"" + socket_name + "\" as FD" + std::to_string(fd) + ".");
        }
        free(fd_names[i]); // Clean every allocated entry
    }
    free(fd_names); // Clean names array
    fd_names = nullptr;
    if(fd < 1)
    {
        throw std::runtime_error("Unable to find socket file descriptor \"" + socket_name + "\"");
    }

    int const socket_check_result = check_for_socket_unix(fd);
    if(socket_check_result < 0)
    {
        throw std::runtime_error("Failed to check socket file descriptor \"" + socket_name + "\"");
    }
    if(socket_check_result == 0)
    {
        throw std::runtime_error("Socket file descriptor \"" + socket_name + "\" (" + std::to_string(fd) + ") has wrong configuration");
    }

    return fd;
}

void systemd_impl::notify_ready() const
{
    int const unset_environment = 0;
    int const send_result       = sd_notify(unset_environment, "READY=1");
    WC_DEBUG_LOG("Result of systemd notify for READY: " + std::to_string(send_result));
    // It is recommended to further ignore the return value of notify
    // See: https://www.freedesktop.org/software/systemd/man/latest/sd_notify.html#
}

void systemd_impl::notify_stopping() const
{
    int const unset_environment = 0;
    int const send_result       = sd_notify(unset_environment, "STOPPING=1");
    WC_DEBUG_LOG("Result of systemd notify for STOPPING: " + std::to_string(send_result));
    // It is recommended to further ignore the return value of notify
    // See: https://www.freedesktop.org/software/systemd/man/latest/sd_notify.html#
}


} // Namespace sal
} // Namespace authserv
} // Namespace wago

#endif // SYSTEMD_INTEGRATION


//---- End of source file ------------------------------------------------------
