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
///  \brief    WAGO Parameter Service provider smoke test helper functions
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_TEST_PROVIDER_SMOKE_TEST_HPP_
#define INC_WAGO_WDX_TEST_PROVIDER_SMOKE_TEST_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_provider_i.hpp>
//#include <wago/wdx/model_provider_i.hpp>
//#include <wago/wdx/device_description_provider_i.hpp>
//#include <wago/wdx/device_extension_provider_i.hpp>
#include <wago/wdx/file_transfer/file_provider_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace test {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

/// Run smoke test against a parameter provider
///
/// \brief The tests executes calls to the provider methods and checks for:
///        - providing a valid string for `get_display_name()`
///        - the parameter selector response contains at least one selector
///        - returned futures are resolved (even in error cases)
///        - WDA status codes match specification in some common error cases
/// 
/// \param provider
///          The parameter provider instance to be smoke-tested
void parameter_provider_smoke_test(wago::wdx::parameter_provider_i &provider);

/// Run smoke test against a model provider
///
/// \brief The tests executes calls to the provider methods and performs
///        basic checks for the provided model
///
/// \param provider
///          The model provider instance to be smoke-tested
//void model_provider_smoke_test(wago::wdx::model_provider_i &provider);

/// Run smoke test against a device description provider
///
/// \brief The tests executes calls to the provider methods and performs
///        basic checks for the provided description
///
/// \param provider
///          The device description provider instance to be smoke-tested
//void device_description_provider_smoke_test(wago::wdx::device_description_provider_i &provider);

/// Run smoke test against a device extension provider
///
/// \brief The tests executes calls to the provider methods and performs
///        basic checks for the provided extension
///
/// \param provider
///          The device extension provider instance to be smoke-tested
//void device_extension_provider_smoke_test(wago::wdx::device_extension_provider_i &provider);

/// Run smoke test against a file provider
///
/// \brief The tests executes calls to the provider methods and checks for:
///        - provided file information
///
/// \param provider
///          The file provider instance to be smoke-tested
void file_provider_smoke_test(wago::wdx::file_provider_i &provider);


} // Namespace test
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_TEST_PROVIDER_SMOKE_TEST_HPP_
//---- End of source file ------------------------------------------------------
