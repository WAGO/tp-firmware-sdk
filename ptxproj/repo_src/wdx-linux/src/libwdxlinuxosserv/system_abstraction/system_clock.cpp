//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for system_clock.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_serv.hpp"

#include <ctime>
#include <cerrno>
#include <thread>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace sal {

class clock_impl : public system_clock
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(clock_impl)

public:
    clock_impl() noexcept = default;
    ~clock_impl() noexcept override = default;

    void gettime(clockid_t  clk_id,
                timespec  *res) const noexcept override;
    void sleep_for(time_t milliseconds) const noexcept override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static clock_impl default_clock;

system_clock *system_clock::instance = &default_clock;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

void clock_impl::gettime(clockid_t  clk_id,
                        timespec  *res) const noexcept
{
    // no error possible on our systems
    // https://linux.die.net/man/3/clock_gettime
    (void) ::clock_gettime(clk_id, res);
}

void clock_impl::sleep_for(time_t milliseconds) const noexcept
{
    // "Any exception thrown by clock, time_point, or duration during the execution 
    // (clocks, time points, and durations provided by the standard library never throw)."
    // (from https://en.cppreference.com/w/cpp/thread/sleep_for)
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

} // Namespace sal
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
