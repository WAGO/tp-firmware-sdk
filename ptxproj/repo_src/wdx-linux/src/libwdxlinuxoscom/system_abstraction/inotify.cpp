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
///  \brief    System abstraction implementation for file system notifications.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_com.hpp"

#include <sys/inotify.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {
namespace sal {

class inotify_impl : public inotify
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(inotify_impl)

public:
    inotify_impl() noexcept = default;
    ~inotify_impl() noexcept override = default;

    file_descriptor init1(int flags) noexcept override;
    int close(file_descriptor fd) override;
    watch_handle add_watch(file_descriptor        fd,
                           char            const *name,
                           uint32_t               mask) noexcept override;
    int rm_watch(file_descriptor fd,
                 watch_handle    wd) noexcept override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static inotify_impl default_inotify;

inotify *inotify::instance = &default_inotify;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
file_descriptor inotify_impl::init1(int const flags) noexcept
{
    return ::inotify_init1(flags);
}

int inotify_impl::close(file_descriptor fd)
{
    return ::close(fd);
}

watch_handle inotify_impl::add_watch(file_descriptor        fd,
                                     char            const *name,
                                     uint32_t               mask) noexcept
{
    return ::inotify_add_watch(fd, name, mask);
}

int inotify_impl::rm_watch(file_descriptor fd,
                           watch_handle    wd) noexcept
{
    return ::inotify_rm_watch(fd, wd);
}


} // Namespace sal
} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
