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
///  \brief    Definition for linux kernel tracer module.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WTRACE_TRACER_I_HPP_
#define INC_WAGO_WTRACE_TRACER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"
#include <wc/trace.h>

#include <wc/std_includes.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wtrace {

class WTRACE_API tracer_i
{
public:
    tracer_i()          = default;
    virtual ~tracer_i() = default;

public:
    virtual void trace(wc_trace_channels   const channel,
                       char const        * const trace_point_name,
                       size_t              const name_length) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

} // Namespace wtrace
} // Namespace wago


#endif // INC_WAGO_WTRACE_TRACER_I_HPP_
//---- End of source file ------------------------------------------------------
