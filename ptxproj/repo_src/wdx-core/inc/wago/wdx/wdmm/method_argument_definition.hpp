//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDMM_METHOD_ARGUMENT_DEFINITION_HPP_
#define INC_WAGO_WDX_WDMM_METHOD_ARGUMENT_DEFINITION_HPP_

#include <string>
#include <memory>
#include "wago/wdx/wdmm/base_types.hpp"
#include "wago/wdx/parameter_value.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct method_argument_definition
{
    std::string name;
    parameter_value_types value_type = parameter_value_types::unknown;
    parameter_value_rank value_rank = parameter_value_rank::scalar;
    std::shared_ptr<parameter_value> default_value;
    name_t enum_name; // for value_type enum
    std::vector<name_t> ref_classes; // for value_type instance_identity_ref
    std::weak_ptr<enum_definition> enum_def; // will be resolved during model loading
    std::vector<std::weak_ptr<class_definition>> ref_classes_def; // will be resolved during model loading
    // TODO: unify validation attributes for ParameterDefinition and ArgumentDefinition, as e.g. pattern makes sense for both
    virtual ~method_argument_definition() noexcept = default;
};

}
}
}
#endif // INC_WAGO_WDX_WDMM_METHOD_ARGUMENT_DEFINITION_HPP_
