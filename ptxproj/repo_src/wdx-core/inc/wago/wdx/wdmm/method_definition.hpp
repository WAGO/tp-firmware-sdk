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
#ifndef INC_WAGO_WDX_WDMM_METHOD_DEFINITION_HPP_
#define INC_WAGO_WDX_WDMM_METHOD_DEFINITION_HPP_

#include <vector>
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/wdmm/method_argument_definition.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct method_definition : public parameter_definition
{
    std::vector<method_argument_definition> in_args;
    std::vector<method_argument_definition> out_args;
};

}
}
}
#endif // INC_WAGO_WDX_WDMM_METHOD_DEFINITION_HPP_
