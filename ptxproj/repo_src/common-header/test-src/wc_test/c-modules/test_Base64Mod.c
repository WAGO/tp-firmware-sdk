//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
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
///  \brief    Test module for Base64 header (C).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "test_Base64Mod.h"
#include "wc/base64.h"

#include <malloc.h>

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
bool test_base64_endecode_c(uint8_t const *bytes,
                            size_t         bytes_length,
                            uint8_t       *decoded,
                            size_t        *decoded_length)
{
  size_t const  needed_bytes   = wc_base64_calc_encoded_length(bytes_length);
  if((bytes_length > 0) && (needed_bytes == 0))
  {
    return false;
  }

  char         *encoded_bytes  = (char *)malloc(needed_bytes);
  size_t        encoded_length = 0;

  bool success = needed_bytes < wc_get_max_base64_encoded_length();
  if(success)
  {
    success = wc_base64_encode(bytes, bytes_length, encoded_bytes, &encoded_length);
  }

  if(success)
  {
    success = bytes_length < wc_get_max_base64_unencoded_length();
  }

  if(success)
  {
    success = bytes_length <= wc_base64_calc_decoded_length(encoded_length);
  }

  if(success)
  {
    success = wc_base64_decode(encoded_bytes, encoded_length, decoded, decoded_length);
  }
  free(encoded_bytes);

  return success;
}

bool test_base64_endecode_url_c(uint8_t const *bytes,
                                size_t         bytes_length,
                                uint8_t       *decoded,
                                size_t        *decoded_length)
{
  size_t const  needed_bytes   = wc_base64_calc_encoded_length(bytes_length);
  if((bytes_length > 0) && (needed_bytes == 0))
  {
    return false;
  }

  char         *encoded_bytes  = (char *)malloc(needed_bytes);
  size_t        encoded_length = 0;

  bool success = needed_bytes < wc_get_max_base64_encoded_length();
  if(success)
  {
    success = wc_base64_encode_for_url(bytes, bytes_length, encoded_bytes, &encoded_length);
  }

  if(success)
  {
    success = bytes_length < wc_get_max_base64_unencoded_length();
  }

  if(success)
  {
    success = bytes_length <= wc_base64_calc_decoded_length(encoded_length);
  }

  if(success)
  {
    success = wc_base64_decode(encoded_bytes, encoded_length, decoded, decoded_length);
  }
  free(encoded_bytes);

  return success;
}


//---- End of source file ------------------------------------------------------

