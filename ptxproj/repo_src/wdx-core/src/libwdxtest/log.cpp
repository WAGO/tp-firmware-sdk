//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    wc/log function definitions for unit tests to make linker happy.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/test/log.hpp"

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

void wc_log_output(log_level_t  const WC_UNUSED_PARAM(log_level),
                   char const * const WC_UNUSED_PARAM(message)) noexcept
{
    // just defined to keep linker happy
}

log_level_t wc_get_log_level() noexcept
{
    // just defined to keep linker happy
    return log_level_t::info;
}

} // Namespace test
} // Namespace wdx
} // Namespace wago

GNUC_ATTRIBUTE(weak) void wc_log_output(log_level_t  const log_level,
                                        char const * const message) noexcept
{
    wago::wdx::test::wc_log_output(log_level, message);
}

GNUC_ATTRIBUTE(weak) log_level_t wc_get_log_level() noexcept
{
    return wago::wdx::test::wc_get_log_level();
}

//---- End of source file ------------------------------------------------------
