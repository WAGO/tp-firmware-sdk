//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for system sockets.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_com.hpp"
#include "errno_utils.hpp"

#include <wago/privileges/user.hpp>

#include <sys/socket.h>

#include <cerrno>
#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {
namespace sal {

class system_sockets_impl : public system_sockets
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(system_sockets_impl)

public:
    system_sockets_impl() noexcept = default;
    ~system_sockets_impl() noexcept override = default;

    std::string get_socket_peer_user(int socket_fd) const override;
    ucred       get_socket_peer_credentials(int socket_fd) const;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static system_sockets_impl default_system_sockets;

system_sockets *system_sockets::instance = &default_system_sockets;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
std::string system_sockets_impl::get_socket_peer_user(int socket_fd) const
{
    auto const cr = get_socket_peer_credentials(socket_fd);
    return privileges::get_user_name(cr.uid);
}

ucred system_sockets_impl::get_socket_peer_credentials(int socket_fd) const
{
    ucred    cr;
    unsigned cr_len = sizeof(cr);
    errno = 0;
    if(getsockopt(socket_fd, SOL_SOCKET, SO_PEERCRED, &cr, &cr_len) < 0)
    {
        throw std::runtime_error("Failed to get socket options: " + errno_to_string(errno));
    }
    // Additional check like done in DBus:
    // https://cgit.freedesktop.org/dbus/dbus/tree/dbus/dbus-sysdeps-unix.c?id=edaa6fe253782dda959d78396b43e9fd71ea77e3
    if(cr_len != sizeof(cr))
    {
        throw std::runtime_error("Failed to get socket options: Unexpected structure length");
    }

    return cr;
}


} // Namespace sal
} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
