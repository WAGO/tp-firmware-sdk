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
///  \brief    WAGO Parameter Service implementation of common trace function.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wtrace/trace.hpp"
#include "wago/wtrace/tracer_i.hpp"
#include "kernel_tracer.hpp"
#include "stream_tracer.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <atomic>
#include <mutex>
#include <iostream>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using std::atomic;
using std::mutex;
using wago::wtrace::tracer_i;
using wago::wtrace::kernel_tracer;
using wago::wtrace::stream_tracer;

struct wtrace_tracer
{
    tracer_i *impl;
};


//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {
atomic<tracer_i *> g_current_tracer(nullptr);
atomic<uint32_t>   g_active_channels(0);
mutex              g_start_stop_mutex;
uint32_t           g_started_channels(0);

std::unique_ptr<kernel_tracer> g_kernel_tracer(nullptr);
wtrace_tracer                  g_kernel_tracer_wrapper         = { g_kernel_tracer.get() };

std::unique_ptr<kernel_tracer> g_passive_kernel_tracer(nullptr);
wtrace_tracer                  g_passive_kernel_tracer_wrapper = { g_passive_kernel_tracer.get() };

std::unique_ptr<stream_tracer> g_stdout_tracer(nullptr);
wtrace_tracer                  g_stdout_tracer_wrapper         = { g_stdout_tracer.get() };

std::unique_ptr<stream_tracer> g_stderr_tracer(nullptr);
wtrace_tracer                  g_stderr_tracer_wrapper         = { g_stderr_tracer.get() };

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
inline bool is_channel_active(wc_trace_channels const channel)
{
    return (g_active_channels & channel);
}

inline bool is_channel_started(wc_trace_channels const channel)
{
    return (g_started_channels & channel);
}

inline bool is_some_channel_active()
{
    return (g_active_channels > 0);
}

inline bool is_some_channel_started()
{
    return (g_started_channels > 0);
}

}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
GNUC_ATTRIBUTE(weak) void wc_trace_activate_channel(wc_trace_channels const channel)
{
    WC_ASSERT(channel != wc_trace_channels::invalid);
    g_active_channels.fetch_or(channel);
    return;
}

GNUC_ATTRIBUTE(weak) void wc_trace_deactivate_channel(wc_trace_channels const channel)
{
    WC_ASSERT(channel != wc_trace_channels::invalid);
    g_active_channels.fetch_and(~channel);
    return;
}

GNUC_ATTRIBUTE(weak) bool wc_trace_is_active_channel(wc_trace_channels const channel)
{
    return is_channel_active(channel);
}

GNUC_ATTRIBUTE(weak) void wc_trace_start_channel(wc_trace_channels const channel)
{
    WC_ASSERT(channel != wc_trace_channels::invalid);
    // Check if channel is active, only active channels are able to start tracer
    if(is_channel_active(channel))
    {
        std::lock_guard<std::mutex> lock(g_start_stop_mutex);
        g_started_channels |= channel;

        tracer_i * const tracer = g_current_tracer;
        if(tracer != nullptr)
        {
            tracer->start();
        }
    }
    return;
}

GNUC_ATTRIBUTE(weak) void wc_trace_stop_channel(wc_trace_channels const channel)
{
    WC_ASSERT(channel != wc_trace_channels::invalid);
    // Check if channel is active, only active channels are able to stop tracer
    if(is_channel_active(channel))
    {
        std::lock_guard<std::mutex> lock(g_start_stop_mutex);
        g_started_channels &= (~channel);

        // Check if all channels are stopped -> stop tracer
        tracer_i * const tracer = g_current_tracer;
        if(    (tracer != nullptr)
            && (g_started_channels == 0))
        {
            tracer->stop();
        }
    }
    return;
}

GNUC_ATTRIBUTE(weak) bool wc_trace_is_started_channel(wc_trace_channels const channel)
{
    return is_channel_started(channel);
}

GNUC_ATTRIBUTE(weak) void wc_trace_set_marker(wc_trace_channels const channel,
                                              char const *    const trace_point_name,
                                              size_t          const name_length)
{
    WC_ASSERT(channel != wc_trace_channels::invalid);
    WC_ASSERT(name_length <= WC_TRACE_POINT_MAX_LENGTH);
    tracer_i * const tracer = g_current_tracer;
    if(tracer != nullptr)
    {
        if(is_channel_active(channel) && is_channel_started(channel))
        {
            tracer->trace(channel, trace_point_name, name_length);
        }
    }
}

namespace wago {
namespace wtrace {

void set_tracer(tracer_i * const tracer)
{
    // Start new tracer if some channel is started
    if(tracer != nullptr)
    {
        std::lock_guard<std::mutex> lock(g_start_stop_mutex);
        if(is_some_channel_started())
        {
            tracer->start();
        }
    }

    // Replace tracer
    tracer_i * const old_tracer = g_current_tracer.exchange(tracer);
    if(old_tracer != nullptr)
    {
        old_tracer->stop();
    }
}

tracer_i * get_kernel_tracer()
{
    if(::g_kernel_tracer == nullptr)
    {
        ::g_kernel_tracer = std::make_unique<kernel_tracer>(false);
    }
    return ::g_kernel_tracer.get();
}

tracer_i * get_passive_kernel_tracer()
{
    if(::g_passive_kernel_tracer == nullptr)
    {
        ::g_passive_kernel_tracer = std::make_unique<kernel_tracer>(true);
    }
    return ::g_passive_kernel_tracer.get();
}

tracer_i * get_stdout_tracer()
{
    if(::g_stdout_tracer == nullptr)
    {
        ::g_stdout_tracer = std::make_unique<stream_tracer>(std::cout);
    }
    return ::g_stdout_tracer.get();
}

tracer_i * get_stderr_tracer()
{
    if(::g_stderr_tracer == nullptr)
    {
        ::g_stderr_tracer = std::make_unique<stream_tracer>(std::cerr);
    }
    return ::g_stderr_tracer.get();
}

} // Namespace wtrace
} // Namespace wago


//---- End of source file ------------------------------------------------------
