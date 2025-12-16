//------------------------------------------------------------------------------
// Copyright (c) 2017-2025 WAGO GmbH & Co. KG
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
///  \brief    Test for assertion helper macros.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include "wc/preprocessing.h"
#include "test_Assertion.h"
#include "c-modules/test_AssertionMod.h"
#include "cpp-modules/test_AssertionMod.hpp"

#ifdef __cplusplus // C++
#include <cassert>
#else // C
#include <assert.h>
#endif

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void wc_Fail(char const * const szReason,
             char const * const szFile,
             char const * const szFunction,
             int const line)
{
  EXPECT_STREQ(WC_ASSERT_PREFIX WC_STR(ONE) "==" WC_STR(ZERO), szReason);
#ifndef DISABLE_WC_FAIL_SOURCE
  EXPECT_NE(nullptr, szFile);
  EXPECT_NE('\0', szFile[0]);
  EXPECT_NE(nullptr, szFunction);
  EXPECT_NE('\0', szFunction[0]);
  EXPECT_LT(0U, line);
#endif

  // Trigger standard assert
  ASSERT_DEATH_IF_SUPPORTED(assert(false), "");
}


TEST(Assert, SucceedC)
{
  TriggerSuccessC();
}


TEST(Assert, FailC_DeathTest)
{
#ifndef NDEBUG
  ASSERT_NO_FATAL_FAILURE(TriggerFailC());
#endif
}


TEST(AssertReturn, SucceedC)
{
  int result = 0;
  result = TriggerSuccessReturnC(15);
  EXPECT_EQ(-1, result);
}


TEST(AssertReturn, FailC_DeathTest)
{
#ifndef NDEBUG
  int result = 0;
  ASSERT_NO_FATAL_FAILURE(result = TriggerFailReturnC(15));
  EXPECT_EQ(15, result);
#endif
}


TEST(AssertReturnVoid, SucceedC)
{
  TriggerSuccessReturnVoidC();
}


TEST(AssertReturnVoid, FailC_DeathTest)
{
#ifndef NDEBUG
  ASSERT_NO_FATAL_FAILURE(TriggerFailReturnVoidC());
#endif
}


TEST(Assert, SucceedCPP)
{
  TriggerSuccessCPP();
}


TEST(Assert, FailCPP_DeathTest)
{
#ifndef NDEBUG
  ASSERT_NO_FATAL_FAILURE(TriggerFailCPP());
#endif
}


TEST(AssertReturn, SucceedCPP)
{
  int result = 0;
  result = TriggerSuccessReturnCPP(15);
  EXPECT_EQ(-1, result);
}


TEST(AssertReturn, FailCPP_DeathTest)
{
#ifndef NDEBUG
  int result = 0;
  ASSERT_NO_FATAL_FAILURE(result = TriggerFailReturnCPP(15));
  EXPECT_EQ(15, result);
#endif
}


TEST(AssertReturnVoid, SucceedCPP)
{
  TriggerSuccessReturnVoidCPP();
}


TEST(AssertReturnVoid, FailCPP_DeathTest)
{
#ifndef NDEBUG
  ASSERT_NO_FATAL_FAILURE(TriggerFailReturnVoidCPP());
#endif
}


//---- End of source file ------------------------------------------------------

