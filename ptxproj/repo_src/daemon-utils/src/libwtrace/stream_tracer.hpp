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
///  \brief    Definition for stream based tracer module.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWTRACE_STREAM_TRACER_HPP_
#define SRC_LIBWTRACE_STREAM_TRACER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wtrace/tracer_i.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <ostream>
#include <atomic>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wtrace {

class stream_tracer final : public tracer_i
{
public:
    stream_tracer(std::ostream &out_stream);
    ~stream_tracer() override;

    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(stream_tracer);

public:
    void trace(wc_trace_channels   const channel,
               char const        * const trace_point_name,
               size_t              const name_length) override;
    void start() override;
    void stop() override;

private:
    std::ostream      &out_stream_m;
    std::atomic<bool>  started_m;
    std::mutex         stream_mutex_m;
};


} // Namespace wtrace
} // Namespace wago


#endif // SRC_LIBWTRACE_STREAM_TRACER_HPP_
//---- End of source file ------------------------------------------------------
