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
///  \brief    Test for standard includes.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include "c-modules/test_StdIncludesMod.h"
#include "cpp-modules/test_StdIncludesMod.hpp"

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
TEST(StdIncludes, stdintC)
{
  ASSERT_EQ(UINT8_MAX, GetUint8MaxC());
  ASSERT_EQ(INT_FAST64_MAX, GetFastInt64MaxC());
}


TEST(StdIncludes, stddefC)
{
  ASSERT_EQ(sizeof(ptrdiff_t), GetPtrDiffSizeC());
}


TEST(StdIncludes, stdboolC)
{
  ASSERT_TRUE(GetTrueC());
  ASSERT_FALSE(GetFalseC());
}


TEST(StdIncludes, stdintCPP)
{
  ASSERT_EQ(UINT8_MAX, GetUint8MaxCPP());
  ASSERT_EQ(INT_FAST64_MAX, GetFastInt64MaxCPP());
}


TEST(StdIncludes, stddefCPP)
{
  ASSERT_EQ(sizeof(ptrdiff_t), GetPtrDiffSizeCPP());
}


TEST(StdIncludes, stdboolCPP)
{
  ASSERT_TRUE(GetTrueCPP());
  ASSERT_FALSE(GetFalseCPP());
}


//---- End of source file ------------------------------------------------------

