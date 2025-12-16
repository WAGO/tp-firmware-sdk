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
#include "wago/wdx/parameter_instance_id.hpp"

namespace wago {
namespace wdx {

parameter_instance_id::parameter_instance_id(parameter_id_t id_, instance_id_t instance_id_,
                                                                     device_id device_id_)
    : id(id_),
      instance_id(instance_id_),
      device(device_id_)
{
}

/// Compares one `parameter_instance_id` to another, but ignoring the `device_id`.
bool parameter_instance_id::equals_ignoring_device(const parameter_instance_id& other) const
{
    return id == other.id && instance_id == other.instance_id;
}

}
}
