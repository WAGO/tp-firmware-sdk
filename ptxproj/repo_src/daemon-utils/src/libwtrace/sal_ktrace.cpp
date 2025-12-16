//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of kernel tracer system abstraction.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "sal_ktrace.hpp"

#include <memory>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wtrace {
namespace sal {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

namespace {

class ktrace_impl : public ktrace_i
{
public:
    std::string find_debug_fs() const override
    {
        return wtrace::find_debug_fs();
    }

    int open(char const * path, int flags) const override
    {
        return ::open(path, flags); // parasoft-suppress CERT_C-FIO32-a "Only redirect a syscall"
    }

    int close(int fd) const noexcept override
    {
        return ::close(fd);
    }

    int write(int fd, char const *bytes, size_t nbytes) const override
    {
        return ::write(fd, bytes, nbytes);
    }
};

}

static ktrace_impl       g_ktrace_impl;
static sysfunc<ktrace_i> g_ktrace(&g_ktrace_impl);
template<>
sysfunc<ktrace_i> &ktrace::get_instance() noexcept
{
    return g_ktrace;
}

} // Namespace sal
} // Namespace wtrace
} // Namespace wago


//---- End of source file ------------------------------------------------------
