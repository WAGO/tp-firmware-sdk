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
///  \brief    Test module for assertion helper macros (C).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "../test_Assertion.h"
#include "test_AssertionMod.h"

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
void TriggerSuccessC(void)
{
  WC_ASSERT(ONE==ONE); //lint -e 506 Constant value Boolean
}


void TriggerFailC(void)
{
  WC_ASSERT(ONE==ZERO); //lint -e 506 Constant value Boolean
}


int TriggerSuccessReturnC(int const value)
{
  WC_ASSERT_RETURN(ONE==ONE, value); //lint -e 506 Constant value Boolean
  return -1;
}


int TriggerFailReturnC(int const value)
{
  WC_ASSERT_RETURN(ONE==ZERO, value); //lint -e 506 Constant value Boolean
  return -1;
}


void TriggerSuccessReturnVoidC(void)
{
  WC_ASSERT_RETURN_VOID(ONE==ONE); //lint -e 506 Constant value Boolean
}


void TriggerFailReturnVoidC(void)
{
  WC_ASSERT_RETURN_VOID(ONE==ZERO); //lint -e 506 Constant value Boolean
}


//---- End of source file ------------------------------------------------------

