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
///  \brief    Static check for trace route header.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/client/trace_routes.hpp"

#include <wc/std_includes.h>
#include <wc/assertion.h>

//------------------------------------------------------------------------------
// checks
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

namespace {
    constexpr size_t bit_count(size_t val)
    {
        return (val > 1) ? ((val & 1) + bit_count(val>>1)) : val;
    }
    constexpr size_t bit_count(trace_route route)
    {
        return bit_count(static_cast<size_t>(route));
    }
}
WC_STATIC_ASSERT(bit_count(trace_route::all) == (sizeof(all_trace_routes)/sizeof(trace_route)));


} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
