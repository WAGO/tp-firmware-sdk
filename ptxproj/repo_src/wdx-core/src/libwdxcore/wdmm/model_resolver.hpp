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
#ifndef SRC_LIBWDXCORE_WDMM_MODEL_RESOLVER_HPP_
#define SRC_LIBWDXCORE_WDMM_MODEL_RESOLVER_HPP_

#include <vector>
#include <memory>
#include "wdmm/device_model.hpp"
#include "wago/wdx/wdmm/feature_definition.hpp"
#include "wago/wdx/wdmm/class_definition.hpp"
#include "wago/wdx/wdmm/class_instantiation.hpp"

using std::vector;

using namespace wago::wdx::wdmm;

namespace wago {
namespace wdx {
namespace wdmm {

class class_instantiation_internal;

class model_resolver
{
public:
    static void resolve_includes(device_model& model);

    template <typename T>
    static void resolve_includes(T& base, std::vector<std::shared_ptr<T>>& others);

    static void prepare_attributes(std::vector<std::shared_ptr<parameter_definition>>& definitions, device_model& model);

    static void prepare_class_instantiation(class_instantiation_internal& inst, device_model& model, bool only_known_classes);

private:

    template <typename T>
    static void takeover_specific_properties(T& base, T& include);
};

template <>
void model_resolver::takeover_specific_properties<class_definition>(class_definition& base, class_definition& include);

template <>
void model_resolver::takeover_specific_properties<feature_definition>(feature_definition& base,
                                                                      feature_definition& include);

}
}
}
#endif // SRC_LIBWDXCORE_WDMM_MODEL_RESOLVER_HPP_
