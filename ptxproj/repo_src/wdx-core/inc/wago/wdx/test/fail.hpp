//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Header for failure handling in test.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_FAIL_HPP_
#define INC_WAGO_WDX_TEST_FAIL_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

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

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_FAIL_HPP_
//---- End of source file ------------------------------------------------------
