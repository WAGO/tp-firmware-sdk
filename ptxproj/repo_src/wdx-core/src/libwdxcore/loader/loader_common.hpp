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
#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace wago {
namespace wdx {

inline allowed_values_spec parse_allowed_values_spec(nlohmann::json const &node)
{
    allowed_values_spec allowed_values;
    if(node.contains("Min"))
    {
        allowed_values.min = node.at("Min").get<int64_t>();
        allowed_values.min_set = true;
    }
    if(node.contains("Max"))
    {
        allowed_values.max = node.at("Max").get<int64_t>();
        allowed_values.max_set = true;
    }
    if(node.contains("List"))
    {
        allowed_values.whitelist = node.at("List").get<std::vector<int64_t>>();
    }
    if(node.contains("Not"))
    {
        allowed_values.blacklist = node.at("Not").get<std::vector<int64_t>>();
    }
    return allowed_values;
}

// TODO: deduplicate with same code from device_model_loader
inline overrideable_attributes parse_overrideable_attributes(nlohmann::json const &node)
{
    overrideable_attributes overrideables;
    if (node.contains("Pattern"))
    {
        overrideables.pattern = node.at("Pattern").get<std::string>();
        overrideables.pattern_set = true;
    }
    if (node.contains("Inactive"))
    {
        overrideables.inactive = node.at("Inactive").get<bool>();
        overrideables.inactive_set = true;
    }
    if(node.contains("DefaultValue"))
    {
        overrideables.default_value = parameter_value::create_with_unknown_type(node.at("DefaultValue").dump());
    }
    if (node.contains("AllowedValues"))
    {
        overrideables.allowed_values = parse_allowed_values_spec(node.at("AllowedValues"));
        overrideables.allowed_values_set = true;
    }
    if (node.contains("AllowedLength"))
    {
        overrideables.allowed_length = parse_allowed_values_spec(node.at("AllowedLength"));
        overrideables.allowed_length_set = true;
    }

    return overrideables;
}

}}