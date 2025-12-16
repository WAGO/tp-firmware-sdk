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
#ifndef SRC_LIBWDXCORE_INSTANCES_PARAMETER_INSTANCE_HPP_
#define SRC_LIBWDXCORE_INSTANCES_PARAMETER_INSTANCE_HPP_

#include <memory>
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/parameter_value.hpp"
#include "wago/wdx/parameter_provider_i.hpp"

namespace wago {
namespace wdx {

/*
WARNING: There might be a lot of ParameterInstances (20.000 is a realistic amount), so we will have to minimize memory usage in here.
*/

struct parameter_instance
{
    parameter_instance_id id;
    parameter_definition const *definition; // raw pointer to minimize memory usage
    parameter_provider_i* provider = nullptr;
    std::shared_ptr<parameter_value> fixed_value = nullptr; // TODO: document
    bool status_unavailable_if_not_provided = false;

    parameter_instance(parameter_instance_id id_,
                       parameter_definition const *definition_)
    : id(id_)
    , definition(definition_)
    { }
};

}
}
#endif // SRC_LIBWDXCORE_INSTANCES_PARAMETER_INSTANCE_HPP_
