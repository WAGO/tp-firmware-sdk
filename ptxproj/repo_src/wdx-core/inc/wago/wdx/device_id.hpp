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

#ifndef INC_WAGO_WDX_DEVICE_ID_HPP_
#define INC_WAGO_WDX_DEVICE_ID_HPP_

#include <cstdint>
#include <string>
#include "wago/wdx/wdmm/base_types.hpp"

namespace wago {
namespace wdx {

/**
The numerical identifier for a device, possibly located within a device collection (like a localbus).
*/
struct device_id
{
    /**
    Logical position in the device collection, beginning with 1.
    The root device is 0, in combination with the device collection 0.
    */
    slot_index_t slot{0};

    /**
    ID of the device collection as defined in the model.
    At the moment, the device collections and their IDs are hard coded.
    */
    device_collection_id_t device_collection_id{0}; //Root

    /** Creates a device_id with slot and device_collection, which both default to 0.
     * \param slot \copydetails device_id::slot
     * \param device_collection \copydetails device_id::device_collection_id
     */
    device_id(slot_index_t slot = 0, device_collection_id_t device_collection = device_collections::root) noexcept;
    
    /** Creates a device_id with slot and device_collection.
     * \param slot \copydetails device_id::slot
     * \param device_collection \copydetails device_id::device_collection_id
     */
    device_id(slot_index_t slot, device_collections device_collection) noexcept;

    /**
     * Constant for identifying the head station (slot = 0, device_collection = 0)
     */
    static const device_id headstation;
};

bool operator==(const device_id& lhs, const device_id& rhs) noexcept;
bool operator!=(const device_id& lhs, const device_id& rhs) noexcept;
bool operator<(const device_id& lhs, const device_id& rhs) noexcept;

inline std::string to_description(device_id id) {
    return "device_id{slot=" + std::to_string(id.slot) + ",collection=" + std::to_string(id.device_collection_id) + "}";
}

inline std::string to_short_description(device_id id) {
    return "{slot=" + std::to_string(id.slot) + ",coll=" + std::to_string(id.device_collection_id) + "}";
}

}}


#endif // INC_WAGO_WDX_DEVICE_ID_HPP_
