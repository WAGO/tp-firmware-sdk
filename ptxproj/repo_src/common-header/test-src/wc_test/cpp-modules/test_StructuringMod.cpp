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
///  \brief    Test module for structuring helper macros (C++).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "test_StructuringMod.hpp"

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
static TestStructCPP stTestContainer = { 7.88 };
static TestStructCPP const stTestContainerR = { 3.22 };
static double const arDoubles[2] = { 1.23, 4.56 };

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
size_t BreakLoopOnValueMatchPlainCPP(size_t const value)
{
  size_t counter = 0U;
  WC_LOOP_UNITL_BREAK
  {
    if(counter == value)
    {
      break;
    }
    counter++;
  }

  return counter;
}


size_t GetSizeOfMemberPlainCPP()
{
  return sizeof(stTestContainer.doubleMember);
}


size_t GetSizeOfMemberMacroCPP()
{
  return WC_SIZEOF_MEMBER(TestStructCPP, doubleMember);
}


size_t GetSizeOfElementPlainCPP()
{
  return sizeof(double);
}


size_t GetSizeOfElementMacroCPP()
{
  return WC_SIZEOF_ELEMENT(arDoubles);
}


size_t GetArrayLengthPlainCPP()
{
  return (sizeof(arDoubles) / sizeof(double));
}


size_t GetArrayLengthMacroCPP()
{
  return WC_ARRAY_LENGTH(arDoubles);
}


int  * GetArrayPointerPlainCPP(int array[])
{
  return &(array[0]);
}


int  * GetArrayPointerMacroCPP(int array[])
{
  return WC_ARRAY_TO_PTR(array);
}


void * GetContainerAddressPlainCPP()
{
  return &stTestContainer;
}


void * GetContainerAddressMacroCPP()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
  return WC_CONTAINER_OF(TestStructCPP, doubleMember, &(stTestContainer.doubleMember));
#pragma GCC diagnostic pop
}


void const * GetContainerAddressReadonlyPlainCPP()
{
  return &stTestContainerR;
}


void const * GetContainerAddressReadonlyMacroCPP()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
  return WC_CONTAINER_OF_R(TestStructCPP, doubleMember, &(stTestContainerR.doubleMember));
#pragma GCC diagnostic pop
}


//---- End of source file ------------------------------------------------------
