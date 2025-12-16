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
#ifndef INC_WAGO_WDX_WDMM_FEATURE_DEFINITION_HPP_
#define INC_WAGO_WDX_WDMM_FEATURE_DEFINITION_HPP_

#include "wago/wdx/wdmm/parameter_definition_owner.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct feature_definition : public parameter_definition_owner
{
    // TODO: deviceHubs - are they actually needed?

    std::vector<name_t> classes;

    using parameter_definition_owner::parameter_definition_owner;
    feature_definition(           feature_definition const  &other) = default;
    feature_definition& operator=(feature_definition const  &other) = default;
    feature_definition(           feature_definition       &&other) = default;
    feature_definition& operator=(feature_definition       &&other) = default;
    ~feature_definition() noexcept override                         = default;

    bool is_part_of(feature_definition const &upper);
};

}
}
}
#endif // INC_WAGO_WDX_WDMM_FEATURE_DEFINITION_HPP_
