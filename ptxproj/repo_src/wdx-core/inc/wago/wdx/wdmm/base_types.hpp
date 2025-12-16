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

#ifndef INC_WAGO_WDX_WDMM_BASE_TYPES_HPP_
#define INC_WAGO_WDX_WDMM_BASE_TYPES_HPP_

#include <limits>
#include <string>
#include <wc/std_includes.h>
#include <wc/assertion.h>

namespace wago {
namespace wdx {
inline namespace wdmm {

/**
 * Identifier for ParameterDefinitions
 */
using parameter_id_t = uint32_t;

constexpr parameter_id_t set_bits(uint8_t bit_count)
{
    parameter_id_t result = 0;
    for(uint8_t i = 0; i < bit_count; ++i)
    {
        result = (result << 1) + 1;
    }
    return result;
};

constexpr uint8_t        parameter_id_bit_size  = 32;
constexpr uint8_t        definition_id_bit_size = 16;
constexpr parameter_id_t definition_id_max      = set_bits(definition_id_bit_size);

WC_STATIC_ASSERT(parameter_id_bit_size  == sizeof(parameter_id_t) * 8);
WC_STATIC_ASSERT(definition_id_bit_size <  parameter_id_bit_size);
WC_STATIC_ASSERT(definition_id_max      <  set_bits(parameter_id_bit_size));

/**
 * Identifier for ClassDefinitions and FeatureDefinitions
 */
using name_t = std::string;


enum class parameter_value_types : uint8_t
{
    unknown = 0,
    string,
    boolean,
    uint8,
    uint16,
    uint32,
    uint64,
    int8,
    int16,
    int32,
    int64,
    float32,
    float64,
    bytes,
    instantiations,
    ipv4address,
    file_id,
    instance_ref,
    instance_identity_ref,
    enum_member,
    method
};

/*
    CHECKLIST for new value types: search the project for 'instance_ref' and complete all the methods/switches/tests for the new type.
*/

enum class parameter_value_rank : uint8_t
{
    scalar = 0,
    array = 1
};

/**
The id to distinguish a parameter that exists multiple times on a device (e.g. for each channel of a module).
In the WAGO Device Model, these parameters will represented by `ParameterDefinition`s contained by `ClassDefinition`s.
For non-multiple parameters it will be 0.
*/
// TODO: To improve type handling avoid using und use struct/class { uint16_t value; instance_id(uint16_t new_value)};
using instance_id_t = uint16_t;

/** Used internally by the `parameter_service` -> Cannot by used as instance id */
constexpr instance_id_t DYNAMIC_PLACEHOLDER_INSTANCE_ID = std::numeric_limits<instance_id_t>::max();

/** Maximum instance id */
constexpr instance_id_t INSTANCE_ID_MAX = std::numeric_limits<instance_id_t>::max() - 1;

/**
Logical position of a device in the device collection, beginning with 1.
The root device is 0, in combination with the device collection 0.
*/
using slot_index_t = uint16_t;

/**
ID of the device collection as defined in the model.
At the moment, the device collections and their IDs are hard coded.
*/
using device_collection_id_t = uint8_t;

using enum_member_id_t = uint16_t;

/**
The unique identifier for a file.
Uniqueness is only guaranteed for the lifetime of the `parameter_service` instance.
 */
using file_id = std::string;

using ipv4address = std::string;

enum device_collections
{
    root = 0,
    kbus = 1,
    rlb = 2,
    highest = 2 // update this when new collections are added!
};

}
}
}
#endif // INC_WAGO_WDX_WDMM_BASE_TYPES_HPP_
