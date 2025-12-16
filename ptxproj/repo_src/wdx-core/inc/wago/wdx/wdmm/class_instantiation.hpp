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
#ifndef INC_WAGO_WDX_WDMM_CLASS_INSTANTIATION_HPP_
#define INC_WAGO_WDX_WDMM_CLASS_INSTANTIATION_HPP_

#include <vector>
#include "wago/wdx/wdmm/class_definition.hpp"
#include "wago/wdx/wdmm/base_types.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

/**
 * Instantiation of a class with possible multiple additional classes the instantiation supports.
 */
struct class_instantiation
{
    /**
     * Names of the classes the instantiation supports. The `parameter_service` will augment the classes' includes.
     */
    std::vector<name_t> classes;

    /**
     * Instance ID of the instantiation, unique for all instantiations with the same `base_path`.
     */
    instance_id_t id = 0;

    /** Use to check if instantiation supports given class */
    bool has_class(name_t const &class_name) const;

    class_instantiation() = default;

    class_instantiation(class_instantiation const &) = default;
    
    /** Creates a class instantiation with given `id`, `cls` and, optionally, addition classes the instantiation supports.
     * The classes have to have the same base path. */
    class_instantiation(instance_id_t const &id_, name_t const &cls, std::vector<name_t> const &additionalClasses = {});
    
    /** Creates a class instantiation with given `id` and `classes` the instantiation supports.
     * The classes have to have the same base path. */
    class_instantiation(instance_id_t const &id_, std::vector<name_t> const &classes);

    virtual ~class_instantiation() = default;
};

}}}
#endif // INC_WAGO_WDX_WDMM_CLASS_INSTANTIATION_HPP_
