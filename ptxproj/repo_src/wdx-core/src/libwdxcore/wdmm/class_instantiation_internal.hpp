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
#ifndef SRC_LIBWDXCORE_WDMM_CLASS_INSTANTIATION_INTERNAL_HPP_
#define SRC_LIBWDXCORE_WDMM_CLASS_INSTANTIATION_INTERNAL_HPP_

#include "wago/wdx/wdmm/class_instantiation.hpp"

#include <vector>

namespace wago {
namespace wdx {
inline namespace wdmm {

/**
 * Adds overrides and resolved definition for used classes to class_instantiation
 */
struct class_instantiation_internal : public class_instantiation
{
    /** Do not use outside of parameter_service. Will not be valid outside of parameter_service. */
    class_definition collected_classes;

    /** Instance specific overrides */
    std::vector<std::shared_ptr<parameter_definition_override const>> overrides;

    using class_instantiation::class_instantiation;

    class_instantiation_internal(class_instantiation_internal const &) = default;

    class_instantiation_internal(class_instantiation const &inst)
    : class_instantiation(inst) 
    { };

    ~class_instantiation_internal() override = default;
};

}}}
#endif // SRC_LIBWDXCORE_WDMM_CLASS_INSTANTIATION_INTERNAL_HPP_
