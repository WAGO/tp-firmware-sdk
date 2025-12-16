//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    wc/log function definitions for unit tests to make linker happy.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/trace.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

GNUC_ATTRIBUTE(weak) void wc_trace_activate_channel(wc_trace_channels const WC_UNUSED_PARAM(channel))
{ }
GNUC_ATTRIBUTE(weak) void wc_trace_deactivate_channel(wc_trace_channels const WC_UNUSED_PARAM(channel))
{ }
GNUC_ATTRIBUTE(weak) bool wc_trace_is_active_channel(wc_trace_channels const WC_UNUSED_PARAM(channel))
{ 
    return false;
}
GNUC_ATTRIBUTE(weak) void wc_trace_start_channel(wc_trace_channels const WC_UNUSED_PARAM(channel))
{ }
GNUC_ATTRIBUTE(weak) void wc_trace_stop_channel(wc_trace_channels const WC_UNUSED_PARAM(channel))
{ }
GNUC_ATTRIBUTE(weak) bool wc_trace_is_started_channel(wc_trace_channels const WC_UNUSED_PARAM(channel))
{
    return false;
}
GNUC_ATTRIBUTE(weak) void wc_trace_set_marker(wc_trace_channels const WC_UNUSED_PARAM(channel),
                                              char const *      const WC_UNUSED_PARAM(trace_point_name),
                                              size_t            const WC_UNUSED_PARAM(name_length))
{ }

//---- End of source file ------------------------------------------------------
