//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXCORE_WDMM_PARAMETER_DEFINITION_OVERRIDE_HPP_
#define SRC_LIBWDXCORE_WDMM_PARAMETER_DEFINITION_OVERRIDE_HPP_

#include <string>
#include <vector>
#include "wago/wdx/wdmm/parameter_definition.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct parameter_definition_override
{
    // Class or feature name. 
    // Empty if WDD override
    std::string owner_name; 
    parameter_id_t definition_id;
    overrideable_attributes attributes;
};

}
}
}
#endif // SRC_LIBWDXCORE_WDMM_PARAMETER_DEFINITION_OVERRIDE_HPP_
