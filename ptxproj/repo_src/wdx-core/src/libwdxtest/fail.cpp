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
///  \brief    Failure management for unit tests.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/test/fail.hpp"

#include <wc/assertion.h>
#include <gtest/gtest.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
unsigned g_expected_fail = 0;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

void add_expected_fail()
{
    ++g_expected_fail;
}

void check_fail_count(char const * const error_source)
{
#ifndef NDEBUG
    EXPECT_EQ(0, g_expected_fail) << "Error source: " << error_source;
#else 
    std::cout << "check_fail_count: Assertions are only checked in debug builds: " << error_source << std::endl;;
#endif
    g_expected_fail = 0;
}

void wc_fail(char const * const reason,
             char const * const file,
             char const * const function,
             int  const         line)
{
    if(g_expected_fail > 0)
    {
        --g_expected_fail;
        EXPECT_STRNE("", reason);
    }
    else
    {
        std::string problem = reason;
        problem += " [";
        problem += "from ";
        problem += file;
        problem += " in function ";
        problem += function;
        problem += ", line ";
        problem += std::to_string(line);
        problem += "]";

        // Trigger test fail
        ASSERT_STREQ("", problem.c_str());
    }
}

} // Namespace test
} // Namespace wdx
} // Namespace wago


GNUC_ATTRIBUTE(weak) void wc_Fail(char const * const reason,
                                  char const * const file,
                                  char const * const function,
                                  int  const         line)
{
    wago::wdx::test::wc_fail(reason, file, function, line);
}

//---- End of source file ------------------------------------------------------
