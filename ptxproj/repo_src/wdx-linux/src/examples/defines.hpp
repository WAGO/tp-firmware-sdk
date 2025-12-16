//------------------------------------------------------------------------------
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
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
///  \brief    Example definitions.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_EXAMPLES_DEFINES_HPP_
#define SRC_EXAMPLES_DEFINES_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/wdmm/prefix.hpp>

#include <wc/assertion.h>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace examples {
using wdx::wdmm::parameter_id_t;

/// Sandbox prefix path
constexpr char const *   const g_sandbox_id_prefix_path = "Sandbox";

// Example-IDs for use with REST-API
//
// Warning:
// Keep this Test-IDs in sync with integration test class
// com.wago.integration.utils.parameterservice.ParameterDefinition
/// Feature name for examples
constexpr char const *   const g_sandbox_feature_examples         = "Examples";
/// Sandbox ID for example readonly value
constexpr parameter_id_t const g_sandbox_id_example_readonly      = wdmm::apply_prefix(wdmm::prefix::sandbox, 1);
/// Sandbox ID for example method to create a file ID
constexpr parameter_id_t const g_sandbox_id_example_create_fileid = wdmm::apply_prefix(wdmm::prefix::sandbox, 2);
/// Sandbox ID for example method to delete a file ID
constexpr parameter_id_t const g_sandbox_id_example_delete_fileid = wdmm::apply_prefix(wdmm::prefix::sandbox, 3);
/// Sandbox ID for example writeflag value
constexpr parameter_id_t const g_sandbox_id_example_writeflag     = wdmm::apply_prefix(wdmm::prefix::sandbox, 4);
/// Sandbox ID for example method serving calculations
constexpr parameter_id_t const g_sandbox_id_example_calculation   = wdmm::apply_prefix(wdmm::prefix::sandbox, 5);
WC_STATIC_ASSERT(5 <= wdmm::definition_id_max);


} // Namespace examples
} // Namespace wdx
} // Namespace wago


#endif // SRC_EXAMPLES_DEFINES_HPP_
//---- End of source file ------------------------------------------------------
