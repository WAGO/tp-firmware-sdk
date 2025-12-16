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
///  \brief    Test for structuring helper macros.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include "c-modules/test_StructuringMod.h"
#include "cpp-modules/test_StructuringMod.hpp"

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
TEST(Structuring, InfiniteLoopC)
{
  size_t const value = 42U;
  ASSERT_EQ(value, BreakLoopOnValueMatchPlainC(value));
}


TEST(Structuring, MemberSizeC)
{
  ASSERT_EQ(GetSizeOfMemberPlainC(), GetSizeOfMemberMacroC());
}


TEST(Structuring, ElementSizeC)
{
  ASSERT_EQ(GetSizeOfElementPlainC(), GetSizeOfElementMacroC());
}


TEST(Structuring, ArrayLengthC)
{
  ASSERT_EQ(GetArrayLengthPlainC(), GetArrayLengthMacroC());
}


TEST(Structuring, ArrayToPointerC)
{
  int test_array[8];
  ASSERT_EQ(GetArrayPointerPlainC(test_array), GetArrayPointerMacroC(test_array));
}


TEST(Structuring, ContainerFromMemberC)
{
  ASSERT_EQ(GetContainerAddressPlainC(), GetContainerAddressMacroC());
}


TEST(Structuring, ConstContainerFromMemberC)
{
  ASSERT_EQ(GetContainerAddressReadonlyPlainC(), GetContainerAddressReadonlyMacroC());
}

TEST(Structuring, InfiniteLoopCPP)
{
  size_t const value = 53U;
  ASSERT_EQ(value, BreakLoopOnValueMatchPlainCPP(value));
}


TEST(Structuring, MemberSizeCPP)
{
  ASSERT_EQ(GetSizeOfMemberPlainCPP(), GetSizeOfMemberMacroCPP());
}


TEST(Structuring, ElementSizeCPP)
{
  ASSERT_EQ(GetSizeOfElementPlainCPP(), GetSizeOfElementMacroCPP());
}


TEST(Structuring, ArrayLengthCPP)
{
  ASSERT_EQ(GetArrayLengthPlainCPP(), GetArrayLengthMacroCPP());
}


TEST(Structuring, ArrayToPointerCPP)
{
  int test_array[8];
  ASSERT_EQ(GetArrayPointerPlainCPP(test_array), GetArrayPointerMacroCPP(test_array));
}


TEST(Structuring, ContainerFromMemberCPP)
{
  ASSERT_EQ(GetContainerAddressPlainCPP(), GetContainerAddressMacroCPP());
}


TEST(Structuring, ConstContainerFromMemberCPP)
{
  ASSERT_EQ(GetContainerAddressReadonlyPlainCPP(), GetContainerAddressReadonlyMacroCPP());
}


//---- End of source file ------------------------------------------------------

