//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of kernel tracer.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "kernel_tracer.hpp"
#include "wago/wtrace/trace.hpp"
#include "sal_ktrace.hpp"
#include <wc/assertion.h>

#include <cstring>
#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wtrace {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

kernel_tracer::kernel_tracer(bool passive)
: passive_m(passive)
, trace_fd_m(-1)
, marker_fd_m(-1)
, started_m(false)
{
    std::string const debug_fs_path = sal::ktrace::get_instance()->find_debug_fs();

    // Open file descriptor to start and stop
    std::string const trace_on_off_path = debug_fs_path + "/tracing/tracing_on";
    trace_fd_m = sal::ktrace::get_instance()->open(trace_on_off_path.c_str(), O_WRONLY);
    if(trace_fd_m < 0)
    {
        throw std::runtime_error(std::string("Kernel Tracer: Unable to open trace start descriptor for ") + trace_on_off_path);
    }

    // Open file descriptor to write trace marker
    std::string const trace_marker_path = debug_fs_path + "/tracing/trace_marker";
    marker_fd_m = sal::ktrace::get_instance()->open(trace_marker_path.c_str(), O_WRONLY);
    if(marker_fd_m < 0)
    {
        sal::ktrace::get_instance()->close(trace_fd_m);
        throw std::runtime_error(std::string("Kernel Tracer: Unable to open trace marker descriptor for ") + trace_marker_path);
    }
}

kernel_tracer::~kernel_tracer()
{
    sal::ktrace::get_instance()->close(marker_fd_m);
    sal::ktrace::get_instance()->close(trace_fd_m);
}

void kernel_tracer::trace(wc_trace_channels   const channel,
                          char const        * const trace_point_name,
                          size_t              const name_length)
{
    if(started_m)
    {
        constexpr size_t const prefix_length = 8;
        constexpr size_t const write_buffer_size = g_trace_point_max_length + prefix_length;
        char                   write_buffer[write_buffer_size];

        // Build string to write
        unsigned const channel_num      = wc_trace_get_channel_number(channel);
        char     const channel_prefix_1 = static_cast<char>('0' + (channel_num / 10));
        char     const channel_prefix_2 = static_cast<char>('0' + (channel_num % 10));
        size_t   const write_length     = std::min(name_length + prefix_length, write_buffer_size);
        write_buffer[0] = 'C';
        write_buffer[1] = 'H';
        write_buffer[2] = 'N';
        write_buffer[3] = ' ';
        write_buffer[4] = channel_prefix_1;
        write_buffer[5] = channel_prefix_2;
        write_buffer[6] = ':';
        write_buffer[7] = ' ';
        memcpy(write_buffer + prefix_length, trace_point_name, write_length - prefix_length);

        // Write trace marker
        if(sal::ktrace::get_instance()->write(marker_fd_m, write_buffer, write_length) < 0)
        {
            if(!passive_m)
            {
                throw std::runtime_error("Kernel Tracer: Failed to write trace marker");
            }
        }
    }
}

void kernel_tracer::start()
{
    // Start event tracing if not in passive mode
    if(!passive_m)
    {
        if(sal::ktrace::get_instance()->write(trace_fd_m, "1", 1) < 0)
        {
            throw std::runtime_error("Kernel Tracer: Failed to start trace");
        }
    }

    started_m = true;
}

void kernel_tracer::stop()
{
    started_m = false;

    // Stop event tracing if not in passive mode
    if(!passive_m)
    {
        if(sal::ktrace::get_instance()->write(trace_fd_m, "0", 1) < 0)
        {
            throw std::runtime_error("Kernel Tracer: Failed to stop trace");
        }
    }
}


} // Namespace wtrace
} // Namespace wago


//---- End of source file ------------------------------------------------------
