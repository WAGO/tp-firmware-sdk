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
///  \brief    Definition for linux kernel tracer module.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWTRACE_KERNEL_TRACER_HPP_
#define SRC_LIBWTRACE_KERNEL_TRACER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wtrace/tracer_i.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <atomic>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wtrace {

class kernel_tracer final : public tracer_i
{
public:
    kernel_tracer(bool passive);
    ~kernel_tracer() override;

    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(kernel_tracer);

public:
    void trace(wc_trace_channels   const channel,
               char const        * const trace_point_name,
               size_t              const name_length) override;
    void start() override;
    void stop() override;

private:
    using file_descriptor = int;

    bool              const passive_m;
    file_descriptor         trace_fd_m;
    file_descriptor         marker_fd_m;
    std::atomic<bool>       started_m;
};


} // Namespace wtrace
} // Namespace wago


#endif // SRC_LIBWTRACE_KERNEL_TRACER_HPP_
//---- End of source file ------------------------------------------------------
