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

#ifndef INC_WAGO_WDX_WDMM_PARAMETER_DEFINITION_HPP_
#define INC_WAGO_WDX_WDMM_PARAMETER_DEFINITION_HPP_

#include <cstdint>
#include <string>
#include <memory>
#include "wago/wdx/wdmm/base_types.hpp"
#include "wago/wdx/wdmm/enum_definition.hpp"

namespace wago {
namespace wdx {

class parameter_value;

inline namespace wdmm {

struct allowed_values_spec
{
    std::int64_t min = 0;
    bool min_set = false;
    std::int64_t max = 0;
    bool max_set = false;
    std::vector<std::int64_t> whitelist;
    std::vector<std::int64_t> blacklist;

    bool is_valid(int64_t const &number) const;
};

struct overrideable_attributes
{
    std::string pattern;
    bool pattern_set = false;
    bool inactive = false;
    bool inactive_set = false;
    std::shared_ptr<parameter_value> default_value;
    allowed_values_spec allowed_values;
    bool allowed_values_set = false;
    allowed_values_spec allowed_length;
    bool allowed_length_set = false;
    void override_with(overrideable_attributes const& other)
    {
        if(other.pattern_set)
        {
            pattern = other.pattern;
            pattern_set = true;
        }
        if(other.inactive_set)
        {
            inactive = other.inactive;
            inactive_set = true;
        }
        if(other.default_value)
        {
            default_value = other.default_value;
        }
        if(other.allowed_values_set)
        {
            allowed_values = other.allowed_values;
            allowed_values_set = true;
        }
        if(other.allowed_length_set)
        {
            allowed_length = other.allowed_length;
            allowed_length_set = true;
        }
    }
};

struct class_definition;
struct feature_definition;
struct method_definition;

struct parameter_definition
{
    parameter_id_t id;
    std::string path;
    parameter_value_types value_type;
    parameter_value_rank value_rank;
    bool only_online;
    bool user_setting;
    bool writeable;
    bool instance_key = false;
    name_t enum_name; // for value_type enum
    std::vector<name_t> ref_classes; // for value_type instance_identity_ref
    std::weak_ptr<feature_definition> feature_def; // will be resolved during model loading
    std::weak_ptr<enum_definition> enum_def; // will be resolved during model loading
    std::weak_ptr<class_definition> class_def; // will be resolved during model loading
    std::vector<std::weak_ptr<class_definition>> ref_classes_def; // will be resolved during model loading
    overrideable_attributes overrideables;
    bool is_beta       = false; // will be determined by feature_def->is_beta
    bool is_deprecated = false; // will be determined by feature_def->is_deprecated

    // TODO: other attributes.
    // When adding an non-overridable attribute, search the project for `only_online` to find places to augment
    // When adding an overridable attribute, search the project for `writeable` (case-insensitive) to find places to augment

    parameter_definition() = default;
    parameter_definition(parameter_definition const &def) = default;
    virtual ~parameter_definition() noexcept = default;
};

}
}
}

#endif // INC_WAGO_WDX_WDMM_PARAMETER_DEFINITION_HPP_
