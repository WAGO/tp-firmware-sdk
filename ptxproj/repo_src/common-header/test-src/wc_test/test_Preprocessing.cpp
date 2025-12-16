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
///  \brief    Test for preprocessing helper macros.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include "test_Preprocessing.h"
#include "c-modules/test_PreprocessingMod.h"
#include "cpp-modules/test_PreprocessingMod.hpp"

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
// test implementation
//------------------------------------------------------------------------------
TEST(Preprocessing, StringFromExpressionC)
{
  ASSERT_EQ(8, GetC_3_5());
  ASSERT_STREQ("3+5", GetCStr_3_5());
}


TEST(Preprocessing, StringFromSubstitutionC)
{
  ASSERT_STREQ("3+FIVE", GetCStr_3_FIVE());
  ASSERT_STREQ("3+5", GetCSubstStr_3_FIVE());
}

TEST(Preprocessing, StringFromExpressionCPP)
{
  ASSERT_EQ(8, GetCPP_3_5());
  ASSERT_STREQ("3+5", GetCPPStr_3_5());
}


TEST(Preprocessing, StringFromSubstitutionCPP)
{
  ASSERT_STREQ("3+FIVE", GetCPPStr_3_FIVE());
  ASSERT_STREQ("3+5", GetCPPSubstStr_3_FIVE());
}

//---- End of source file ------------------------------------------------------

