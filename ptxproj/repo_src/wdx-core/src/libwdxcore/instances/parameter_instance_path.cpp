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
#include "wago/wdx/parameter_instance_path.hpp"
#include "utils/string_util.hpp"

namespace wago {
namespace wdx {

parameter_instance_path::parameter_instance_path(parameter_instance_path_t parameter, device_path_t device)
    : parameter_path(std::move(parameter)),
      device_path(std::move(device))
{
}

bool operator==(const parameter_instance_path& lhs, const parameter_instance_path& rhs) noexcept
{
    return    (             lhs.parameter_path.size()  ==              rhs.parameter_path.size())
           && (             lhs.device_path            ==              rhs.device_path)
           && (to_lower_copy(lhs.parameter_path)       == to_lower_copy(rhs.parameter_path));
}

bool operator!=(const parameter_instance_path& lhs, const parameter_instance_path& rhs) noexcept
{
    return !(lhs == rhs);
}

}
}
