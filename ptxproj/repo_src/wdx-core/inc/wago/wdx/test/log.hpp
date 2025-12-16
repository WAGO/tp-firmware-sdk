//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Header for log handling in test.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_LOG_HPP_
#define INC_WAGO_WDX_TEST_LOG_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/log.h>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

/// Utility function to be used to send logs received via global wc_log_output 
/// to.
/// \param log_level
///    To be passed from global wc_log_output.
/// \param message
///    To be passed from global wc_log_output.
void wc_log_output(log_level_t  const log_level, 
                   char const * const message) noexcept;

/// Utility function to be used from global wc_get_log_level.
/// to.
/// \return 
///    A log level
log_level_t wc_get_log_level() noexcept;

} // Namespace test
} // Namespace wdx
} // Namespace wago

#endif // INC_WAGO_WDX_TEST_LOG_HPP_
//---- End of source file ------------------------------------------------------
