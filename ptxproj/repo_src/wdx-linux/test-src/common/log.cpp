//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
#include <wc/log.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
GNUC_ATTRIBUTE(weak) void wc_log_output(log_level_t  const WC_UNUSED_PARAM(log_level),
                                        char const * const WC_UNUSED_PARAM(message)) noexcept
{
    // just defined to keep linker happy
}

GNUC_ATTRIBUTE(weak) log_level_t wc_get_log_level() noexcept
{
    // just defined to keep linker happy
    return log_level_t::info;
}

//---- End of source file ------------------------------------------------------
