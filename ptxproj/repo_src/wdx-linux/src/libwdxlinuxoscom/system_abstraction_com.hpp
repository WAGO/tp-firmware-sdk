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
///  \brief    System abstraction for liblibwdxlinuxoscom to be independent
///            from real system functions (e. g. helpful for tests)
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_SYSTEM_ABSTRACTION_COM_HPP_
#define SRC_LIBWDXLINUXOSCOM_SYSTEM_ABSTRACTION_COM_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>
#include <sys/stat.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
struct stat;

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {
namespace sal {

using file_descriptor   = int;
using watch_handle      = int;

class inotify
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(inotify)

protected:
    static inotify *instance;

protected:
    inotify() = default;

public:
    static inotify &get_instance() noexcept { return *instance; }

    virtual ~inotify() noexcept = default;

    virtual file_descriptor init1(int flags) noexcept = 0;
    virtual int close(file_descriptor fd) = 0;
    virtual watch_handle add_watch(file_descriptor        fd,
                                   char            const *name,
                                   uint32_t               mask) noexcept = 0;
    virtual int rm_watch(file_descriptor fd,
                         watch_handle    wd) noexcept = 0;
};

class system_sockets
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(system_sockets)

protected:
    static system_sockets *instance;

protected:
    system_sockets() = default;

public:
    static system_sockets &get_instance() noexcept { return *instance; }

    virtual ~system_sockets() noexcept = default;

    virtual std::string get_socket_peer_user(int socket_fd) const = 0;
};


} // Namespace sal
} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_SYSTEM_ABSTRACTION_COM_HPP_
//---- End of source file ------------------------------------------------------
