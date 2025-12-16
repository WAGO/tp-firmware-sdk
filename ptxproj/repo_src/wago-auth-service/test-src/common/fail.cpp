//------------------------------------------------------------------------------
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
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
///  \brief    Failure management for unit tests.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "fail.hpp"

#include <wc/assertion.h>
#include <gtest/gtest.h>

#include <string>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
unsigned g_expected_fail = 0;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace wago {
namespace authtest {

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

} // Namespace authtest
} // Namespace wago


void wc_Fail(char const * const reason,
             char const * const file,
             char const * const function,
             int          const line)
{
    wago::authtest::wc_fail(reason, file, function, line);
}


//---- End of source file ------------------------------------------------------
