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
///  \brief    Utility to create prefixed Parameter-IDs.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDMM_PREFIX_HPP_
#define INC_WAGO_WDX_WDMM_PREFIX_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "base_types.hpp"
#include <wc/std_includes.h>
#include <wc/assertion.h>

//------------------------------------------------------------------------------
// const expression functions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wdmm {

enum class prefix : uint8_t {
    wago    = 0,
    sandbox = 1,
    apitest = 2
};

constexpr uint8_t        prefix_id_bit_size     =  4;
constexpr parameter_id_t prefix_id_max          = set_bits(prefix_id_bit_size);
WC_STATIC_ASSERT(prefix_id_bit_size     <  parameter_id_bit_size);
WC_STATIC_ASSERT(prefix_id_max          <  set_bits(parameter_id_bit_size));

constexpr parameter_id_t apply_prefix(prefix prefix_id, parameter_id_t definition_id)
{
    parameter_id_t const prefix_mask = (static_cast<parameter_id_t>(prefix_id) << parameter_id_bit_size - prefix_id_bit_size);
    return prefix_mask + definition_id;
}

} // Namespace wdmm
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDMM_PREFIX_HPP_
//---- End of source file ------------------------------------------------------
