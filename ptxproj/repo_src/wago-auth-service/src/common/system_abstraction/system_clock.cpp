//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for clock.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <ctime>
#include <wc/structuring.h>
#include <thread>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace sal {

class system_clock_impl : public system_clock
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(system_clock_impl)
public:
    system_clock_impl() noexcept = default;
    ~system_clock_impl() noexcept override = default;

    void gettime(clockid_t  clk_id,
                 timespec  *res) const noexcept override;
    virtual void sleep_for(time_t milliseconds) const noexcept override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static system_clock_impl default_clock;

system_clock *system_clock::instance = &default_clock;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

void system_clock_impl::gettime(clockid_t  clk_id,
                                timespec  *res) const noexcept
{
    // no error possible on our systems
    // https://linux.die.net/man/3/clock_gettime
    (void) ::clock_gettime(clk_id, res);
}

void system_clock_impl::sleep_for(time_t milliseconds) const noexcept
{
    // "Any exception thrown by clock, time_point, or duration during the execution 
    // (clocks, time points, and durations provided by the standard library never throw)."
    // (from https://en.cppreference.com/w/cpp/thread/sleep_for)
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

} // Namespace sal
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
