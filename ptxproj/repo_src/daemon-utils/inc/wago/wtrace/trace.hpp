//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Linux trace framework to manage trace points.
///
///  \author   PEn : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WTRACE_TRACE_HPP_
#define INC_WAGO_WTRACE_TRACE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/trace.h>
#include <wc/std_includes.h>
#include <wc/assertion.h>

// export symbols for functions declared by wc/trace
extern "C"
{
WTRACE_API void wc_trace_activate_channel(wc_trace_channels const channel);
WTRACE_API void wc_trace_deactivate_channel(wc_trace_channels const channel);
WTRACE_API bool wc_trace_is_active_channel(wc_trace_channels const channel);
WTRACE_API void wc_trace_start_channel(wc_trace_channels const channel);
WTRACE_API void wc_trace_stop_channel(wc_trace_channels const channel);
WTRACE_API bool wc_trace_is_started_channel(wc_trace_channels const channel);
WTRACE_API void wc_trace_set_marker(wc_trace_channels const channel,
                                    char const *      const trace_point_name,
                                    size_t            const name_len);
}

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

WTRACE_API inline wc_trace_channels operator+(wc_trace_channels const &lhs, wc_trace_channels const &rhs)
{
    uint32_t const combined_channels = lhs | rhs;
    WC_STATIC_ASSERT(sizeof(combined_channels) <= sizeof(wc_trace_channels));
    return static_cast<wc_trace_channels>(combined_channels); // parasoft-suppress CERT_CPP-INT50-a "We want to combine channels with binary operation"
}

namespace wago {
namespace wtrace {

    using channels = wc_trace_channels;
    class tracer_i;

    constexpr size_t   const g_trace_point_max_length = WC_TRACE_POINT_MAX_LENGTH;
    constexpr unsigned const g_all_channel_number     = WC_TRACE_ALL_CHANNEL_NUMBER;

    struct WTRACE_API string
    {
        char   const * const name;
        size_t const         length;

        constexpr string(char const trace_point_name[])
        : name(trace_point_name)
        , length(cstring_len(trace_point_name))
        { }

        constexpr static size_t cstring_len(char const cstring[])
        {
            return (cstring[0] == '\0') ? 0 : (1 + cstring_len(cstring + 1));
        }
    };

    //------------------------------------------------------------------------------
    /// Setter function for a tracer.
    ///
    /// \note The underlying C-function may be implemented/provided application specific.
    ///
    /// \param tracer
    ///   Tracer instance to set.
    //------------------------------------------------------------------------------
    WTRACE_API void set_tracer(tracer_i *tracer);

    //------------------------------------------------------------------------------
    /// Get predefined kernel tracer instance.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \return
    ///   Kernel tracer instance pointer.
    //------------------------------------------------------------------------------
    WTRACE_API tracer_i* get_kernel_tracer();

    //------------------------------------------------------------------------------
    /// Get predefined passive kernel tracer instance.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \return
    ///   Passive kernel tracer instance pointer.
    //------------------------------------------------------------------------------
    WTRACE_API tracer_i* get_passive_kernel_tracer();

    //------------------------------------------------------------------------------
    /// Get predefined stdout tracer instance.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \return
    ///   Kernel stdout instance pointer.
    //------------------------------------------------------------------------------
    WTRACE_API tracer_i* get_stdout_tracer();

    //------------------------------------------------------------------------------
    /// Get predefined stderr tracer instance.
    ///
    /// \note The function may be implemented/provided application specific.
    ///
    /// \return
    ///   Kernel stderr instance pointer.
    //------------------------------------------------------------------------------
    WTRACE_API tracer_i* get_stderr_tracer();
} // Namespace wtrace
} // Namespace wago

#endif // INC_WAGO_WTRACE_TRACE_HPP_
//---- End of source file ------------------------------------------------------
