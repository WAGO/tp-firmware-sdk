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

#include <vector>
#include "wago/wdx/wdmm/base_types.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm {

struct enum_member {
    std::string name;
    enum_member_id_t id;
};

struct enum_definition
{
    name_t name;
    std::vector<enum_member> members;
};

}
}
}