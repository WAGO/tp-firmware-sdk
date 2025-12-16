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
/*
Small implementations of miscellaneous declarations
*/

#include "wago/wdx/wdmm/class_instantiation.hpp"
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "utils/algo_helpers.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

class_instantiation::class_instantiation(instance_id_t const &id_, name_t const &cls, std::vector<name_t> const &additionalClasses)
: id(id_)
{
    classes.push_back(cls);
    append(classes, additionalClasses);
}

class_instantiation::class_instantiation(instance_id_t const &id_, std::vector<name_t> const &classes_)
: id(id_)
{
    append(classes, classes_);
}

bool class_instantiation::has_class(std::string const &class_name) const
{
    return contains<std::string>(classes, [=](const std::string& s) { return s == class_name; });
}

bool allowed_values_spec::is_valid(int64_t const &number) const
{
    if(min_set && number < min)
    {
        return false;
    }
    if(max_set && number > max)
    {
        return false;
    }
    if(!whitelist.empty() && !contains<int64_t>(whitelist, [=](const auto& n) { return n == number; }))
    {
        return false;
    }
    if(!blacklist.empty() && contains<int64_t>(blacklist, [=](const auto& n) { return n == number; }))
    {
        return false;
    }
    return true;
}

}}}
