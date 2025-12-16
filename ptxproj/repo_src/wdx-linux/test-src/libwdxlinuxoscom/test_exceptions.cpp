//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
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
///  \brief    Test exception objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/exception.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::linuxos::com::exception;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const default_exception_text[] = "Test exception text";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(base_exception, construct_delete_with_message)
{
    exception e(default_exception_text);
    EXPECT_STREQ(default_exception_text, e.what());
}

TEST(base_exception, construct_delete_with_runtime_error)
{
    std::runtime_error base_e(default_exception_text);
    exception e(base_e);
    EXPECT_STREQ(default_exception_text, e.what());
}

TEST(base_exception, construct_delete_with_std_exception)
{
    std::exception base_e;
    exception e(base_e);
    EXPECT_STRNE(nullptr, e.what());
    EXPECT_LT(0, strlen(e.what()));
}

TEST(base_exception, derived_from_std_exception)
{
    try
    {
        throw exception(default_exception_text);
    }
    catch(std::exception const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than an exception";
    }
}

TEST(base_exception, derived_from_runtime_error)
{
    try
    {
        throw exception(default_exception_text);
    }
    catch(std::runtime_error const &e)
    {
        EXPECT_STREQ(default_exception_text, e.what());
    }
    catch(...)
    {
        ADD_FAILURE() << "Not expected any other than an runtime_error exception";
    }
}

TEST(base_exception, basic_values)
{
    char const test_message[] = "Base exception test message";

    exception testexception(test_message);
    EXPECT_STREQ(test_message, testexception.what());
}


//---- End of source file ------------------------------------------------------
