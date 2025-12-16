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
///  \brief    Implementation of output stream tracer.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "stream_tracer.hpp"

#include <wc/assertion.h>

#include <string>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>

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
stream_tracer::stream_tracer(std::ostream &out_stream)
: out_stream_m(out_stream)
, started_m(false)
{ }

stream_tracer::~stream_tracer()
{ }

void stream_tracer::trace(wc_trace_channels   const channel,
                          char const        * const trace_point_name,
                          size_t              const)
{
    if(started_m)
    {
        // Prepare timestamp
        auto     const now      = std::chrono::high_resolution_clock::now();
        auto     const us_count = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        WC_ASSERT(us_count >= 0);
        uint64_t const us       = static_cast<uint64_t>(us_count);

        {
            std::lock_guard<std::mutex> lock(stream_mutex_m);

            // Save stream format
            std::ios_base::fmtflags const stream_flags(out_stream_m.flags());

            // Print output
            constexpr uint64_t us_factor = 1000000;
            out_stream_m.fill('0');
            out_stream_m << us/us_factor << "." << std::setw(6) << us%us_factor
                         << "   CHN " << std::setw(2) << wc_trace_get_channel_number(channel) << ": "
                         << trace_point_name << std::endl;

            // Restore stream format
            out_stream_m.flags(stream_flags);
        }
    }
}

void stream_tracer::start()
{
    started_m = true;
}

void stream_tracer::stop()
{
    started_m = false;
}


} // Namespace wtrace
} // Namespace wago


//---- End of source file ------------------------------------------------------
