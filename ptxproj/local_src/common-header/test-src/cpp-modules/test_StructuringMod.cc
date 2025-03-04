//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project common-header (PTXdist package libcommonheader).
//
// Copyright (c) 2017-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file     test_StructuringMod.cc
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
size_t BreakScopePlainCPP()
{
  size_t counter = 0U;
  WC_BREAKABLE_SCOPE
  {
    if(counter >= 2)
    {
      break;
    }
    counter++;
  }

  return counter;
}


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
