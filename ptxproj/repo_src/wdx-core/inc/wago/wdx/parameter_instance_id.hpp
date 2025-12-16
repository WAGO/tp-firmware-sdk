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

#ifndef INC_WAGO_WDX_PARAMETER_INSTANCE_ID_HPP_
#define INC_WAGO_WDX_PARAMETER_INSTANCE_ID_HPP_

#include <cstdint>
#include "wago/wdx/wdmm/base_types.hpp"
#include "wago/wdx/device_id.hpp"

namespace wago {
namespace wdx {

/**
The fundamental structure to address a specific parameter(-instance) in a specific device.
*/
class parameter_instance_id
{
public:
    /**
    The globally unique id of a `parameter_definition`.
    */
    wdmm::parameter_id_t id = 0;

    /**
    The id to distinguish a parameter that exists multiple times on a device (e.g. for each channel of a module).
    In the WAGO Device Model, these parameters will represented by `parameter_definition`s contained by `class_definition`s.
    For non-multiple parameters it will be 0.
    */
    wdmm::instance_id_t instance_id = 0;

    /**
    The id to distinguish the parameters of one device to another on the same node (e.g. the id of a module).
    Defaults to `device_id::headstation`.
    */
    device_id device = device_id::headstation;

    /** Creates a parameter_instance_id, with instance_id and device_id defaulting to 0 and headstation.
     * \param id \copydetails parameter_instance_id::id
     * \param instance_id \copydetails parameter_instance_id::instance_id
     * \param device_id \copydetails parameter_instance_id::device
     */
    explicit parameter_instance_id(wdmm::parameter_id_t id, wdmm::instance_id_t instance_id = 0,
                                      device_id device_id = device_id::headstation);
    
    /** Creates an empty parameter_instance_id, with all members zeroed.
     */
    parameter_instance_id() = default;

    /// Compares one `parameter_instance_id` to another, but ignoring the `device_id`.
    bool equals_ignoring_device(const parameter_instance_id& other) const;
};

inline bool operator==(const parameter_instance_id& lhs, const parameter_instance_id& rhs) noexcept {
  return lhs.equals_ignoring_device(rhs) && lhs.device == rhs.device;
}

inline bool operator!=(const parameter_instance_id& lhs, const parameter_instance_id& rhs) noexcept {
  return !(lhs == rhs);
}

inline bool operator<(const parameter_instance_id& lhs, const parameter_instance_id& rhs) noexcept
{
    if(lhs.device < rhs.device)
    {
        return true;
    }
    else if(lhs.device == rhs.device && lhs.id < rhs.id)
    {
        return true;
    }
    else if(lhs.device == rhs.device && lhs.id == rhs.id && lhs.instance_id < rhs.instance_id)
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline std::string to_description(parameter_instance_id id) {
  return "parameter_instance_id{definition=" + std::to_string(id.id) + ",instance=" + std::to_string(id.instance_id) + ",device=" + to_description(id.device) + "}";
}

inline std::string to_short_description(parameter_instance_id id) {
  return "{def=" + std::to_string(id.id) + ",inst=" + std::to_string(id.instance_id) + (id.device == device_id::headstation ? "" : ",dev=" + to_short_description(id.device)) + "}";
}

}
}
#endif // INC_WAGO_WDX_PARAMETER_INSTANCE_ID_HPP_
