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
#ifndef SRC_LIBWDXCORE_WDMM_DEVICE_DESCRIPTION_HPP_
#define SRC_LIBWDXCORE_WDMM_DEVICE_DESCRIPTION_HPP_


#include <vector>
#include "wdmm/class_instantiation_internal.hpp"
#include "wdmm/value_declaration.hpp"

#include <memory>

namespace wago {
namespace wdx {
inline namespace wdmm {

struct device_description
{
    std::string wdmm_version;
    std::string model_name;
    std::vector<name_t> features;
    std::vector<class_instantiation_internal> instantiations;
    std::vector<value_declaration> values;
    std::vector<std::shared_ptr<parameter_definition_override const>> overrides;
};

}}}
#endif // SRC_LIBWDXCORE_WDMM_DEVICE_DESCRIPTION_HPP_
