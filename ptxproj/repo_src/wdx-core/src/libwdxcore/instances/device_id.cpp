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
#include "wago/wdx/device_id.hpp"

namespace wago {
namespace wdx {

device_id::device_id(slot_index_t slot_, const device_collection_id_t device_collection) noexcept
    : slot(slot_),
      device_collection_id(device_collection)
{
}

device_id::device_id(slot_index_t slot_, device_collections device_collection) noexcept
    : slot(slot_),
      device_collection_id(device_collection)
{
}

const device_id device_id::headstation = device_id{0, 0};

bool operator==(const device_id& lhs, const device_id& rhs) noexcept
{
    return lhs.slot == rhs.slot && lhs.device_collection_id == rhs.device_collection_id;
}

bool operator!=(const device_id& lhs, const device_id& rhs) noexcept
{
    return !(lhs == rhs);
}

bool operator<(const device_id& lhs, const device_id& rhs) noexcept
{
    return    (lhs.device_collection_id < rhs.device_collection_id)
           || (lhs.device_collection_id == rhs.device_collection_id && lhs.slot < rhs.slot);
}

}
}
