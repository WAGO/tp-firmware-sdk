//------------------------------------------------------------------------------
// Copyright (c) 2020-2023 WAGO GmbH & Co. KG
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
///  \brief    Header for failure handling in test.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_FAIL_HPP_
#define TEST_INC_FAIL_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace authtest {

/// Call to push one call expectation to the global "wc_Fail" function.
//------------------------------------------------------------------------------
void add_expected_fail();

/// Call to verify that all expected calls to wc_Fail have been happening.
///
/// \param error_source
///   Error source information to be printed when check fails
///   (e.g. '__FILE__ ": " WC_SUBST_STR(__LINE__)')
//------------------------------------------------------------------------------
void check_fail_count(char const * const error_source);

/// Call to verify that all expected calls to wc_Fail have been happening.
//------------------------------------------------------------------------------
inline void check_fail_count() { check_fail_count("No source information given to check_fail_count()"); }

/// Utility function to be called from actual wc_Fail implementation
/// to enable add_expected_fail and check_fail_count helper functions
/// to work as intended.
///
/// \param reason
///     To be passed from wc_Fail
/// \param file
///     To be passed from wc_Fail
/// \param function
///     To be passed from wc_Fail
/// \param line
///     To be passed from wc_Fail
//------------------------------------------------------------------------------
void wc_fail(char const * const reason,
             char const * const file,
             char const * const function,
             int const          line);

} // Namespace authtest
} // Namespace wago

#endif // TEST_INC_FAIL_HPP_
//---- End of source file ------------------------------------------------------
