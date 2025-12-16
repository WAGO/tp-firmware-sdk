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
///  \brief    Test module for structuring helper macros (C).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_SRC_WC_TEST_C_MODULES_TEST_STRUCTURINGMOD_H_
#define TEST_SRC_WC_TEST_C_MODULES_TEST_STRUCTURINGMOD_H_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wc/structuring.h"
#include "wc/std_includes.h"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
struct TestStructC
{
  double doubleMember;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  size_t BreakLoopOnValueMatchPlainC(size_t const value);
  size_t GetSizeOfMemberPlainC(void);
  size_t GetSizeOfMemberMacroC(void);
  size_t GetSizeOfElementPlainC(void);
  size_t GetSizeOfElementMacroC(void);
  size_t GetArrayLengthPlainC(void);
  size_t GetArrayLengthMacroC(void);
  int  * GetArrayPointerPlainC(int array[]);
  int  * GetArrayPointerMacroC(int array[]);
  void * GetContainerAddressPlainC(void);
  void * GetContainerAddressMacroC(void);
  void const * GetContainerAddressReadonlyPlainC(void);
  void const * GetContainerAddressReadonlyMacroC(void);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------


#endif // TEST_SRC_WC_TEST_C_MODULES_TEST_STRUCTURINGMOD_H_
//---- End of source file ------------------------------------------------------

