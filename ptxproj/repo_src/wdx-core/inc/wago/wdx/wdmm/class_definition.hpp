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
#ifndef INC_WAGO_WDX_WDMM_CLASS_DEFINITION_HPP_
#define INC_WAGO_WDX_WDMM_CLASS_DEFINITION_HPP_

#include "wago/wdx/wdmm/parameter_definition_owner.hpp"
#include "wago/wdx/wdmm/feature_definition.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct class_definition : public parameter_definition_owner
{
    std::string base_path;
    parameter_id_t base_id = 0;
    bool is_dynamic = false;
    bool is_writable = false;
    bool is_user_setting = false;
    bool is_instance_key = false;

    std::weak_ptr<feature_definition> feature_def; // will be resolved during model loading
    
    std::shared_ptr<parameter_definition> instantiations_parameter; // pseudo-parameter-definition for representing the instantiations of this class

    using parameter_definition_owner::parameter_definition_owner;
    class_definition(           class_definition const  &other) = default;
    class_definition& operator=(class_definition const  &other) = default;
    class_definition(           class_definition       &&other) = default;
    class_definition& operator=(class_definition       &&other) = default;
    ~class_definition() noexcept override                       = default;

    bool is_part_of(class_definition const &upper);

    static void build_instantiations_parameter(std::shared_ptr<class_definition> class_def)
    {
        class_def->instantiations_parameter               = std::make_shared<parameter_definition>();
        class_def->instantiations_parameter->id           = class_def->base_id;
        class_def->instantiations_parameter->path         = class_def->base_path;
        class_def->instantiations_parameter->value_type   = parameter_value_types::instantiations;
        class_def->instantiations_parameter->value_rank   = parameter_value_rank::scalar;
        class_def->instantiations_parameter->writeable    = class_def->is_writable;
        class_def->instantiations_parameter->user_setting = class_def->is_user_setting;
        class_def->instantiations_parameter->only_online  = class_def->is_dynamic && !class_def->is_user_setting;
    }
};

}
}
}
#endif // INC_WAGO_WDX_WDMM_CLASS_DEFINITION_HPP_
