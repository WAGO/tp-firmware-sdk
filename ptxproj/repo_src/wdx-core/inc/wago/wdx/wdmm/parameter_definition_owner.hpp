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
#ifndef INC_WAGO_WDX_WDMM_PARAMETER_DEFINITION_OWNER_HPP_
#define INC_WAGO_WDX_WDMM_PARAMETER_DEFINITION_OWNER_HPP_

#include <string>
#include <vector>
#include <memory>
#include "wago/wdx/wdmm/base_types.hpp"
#include "wago/wdx/wdmm/parameter_definition.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct parameter_definition_override;

struct parameter_definition_owner
{
    name_t name;
    std::vector<name_t> includes;
    std::vector<std::shared_ptr<parameter_definition>> parameter_definitions;
    std::vector<std::shared_ptr<parameter_definition_override const>> overrides;
    bool is_beta       = false; // for class_definition, this will be determined by feature_def->is_beta
    bool is_deprecated = false; // for class_definition, this could not be determined from feature_def

    /** Resolving means to recursively collect all `parameter_definitions` from `includes`.
     * The original owned parameter_definitions stay untouched.
     */
    bool is_resolved = false;
    std::vector<name_t> resolved_includes;
    std::vector<std::shared_ptr<parameter_definition>> resolved_parameter_definitions;
    std::vector<std::shared_ptr<parameter_definition_override const>> resolved_overrides;

    parameter_definition_owner()                                                    = default;
    parameter_definition_owner(           parameter_definition_owner const  &other) = default;
    parameter_definition_owner& operator=(parameter_definition_owner const  &other) = default;
    parameter_definition_owner(           parameter_definition_owner       &&other) = default;
    parameter_definition_owner& operator=(parameter_definition_owner       &&other) = default;
    virtual ~parameter_definition_owner() noexcept                                  = default;
};

}
}
}
#endif // INC_WAGO_WDX_WDMM_PARAMETER_DEFINITION_OWNER_HPP_
