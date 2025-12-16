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
#ifndef SRC_LIBWDXCORE_WDA_IPC_REPRESENTATION_HPP_
#define SRC_LIBWDXCORE_WDA_IPC_REPRESENTATION_HPP_

#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/device_id.hpp"
#include "wago/wdx/parameter_instance_path.hpp"
#include "wago/wdx/status_codes.hpp"
#include "wago/wdx/responses.hpp"

namespace wago {
namespace wda_ipc {

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 * Format: <device_collection_id>-<slot>
 */
std::string to_string(wago::wdx::device_id const &id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 * Format: <device.device_collection_id>-<device.slot>-<id>-<instance_id>
 */
std::string to_string(wago::wdx::parameter_instance_id const &id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 * Format: <device_path>-<parameter_path>
 */
std::string to_string(wago::wdx::parameter_instance_path const &path);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::parameter_id_t const &id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::instance_id_t const &id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::device_collection_id_t const &id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 * NOTE: This declaration is just for documentation purposes, its definition is equivalent with that of `instance_id_t` since they both alias to uint16_t.
 */
std::string to_string(wago::wdx::slot_index_t const &id);

/**
* Creates a round-trippable string representation, compatible to be used within URLs. Implementation lowercases.
 */
std::string to_string(wago::wdx::name_t const &name);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::monitoring_list_id_t const &id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::status_codes const &status);

/**
 * Creates a one-way string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::parameter_id_t id, wago::wdx::instance_id_t const &instance_id);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::parameter_value_types const &t);

/**
 * Creates a round-trippable string representation, compatible to be used within URLs.
 */
std::string to_string(wago::wdx::parameter_value_rank const &t);

/**
 * Uses the round-trippable string representation to (re)create `T`.
 * Will throw if the string is malformed.
 */
template <typename T>
T from_string(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create `device_id`.
 * Will throw if the string is malformed.
 */
template <>
wago::wdx::device_id from_string<wago::wdx::device_id>(std::string const &s);

wago::wdx::status_codes parse_device_collection_and_slot(std::string                       const &collectionStr,
                                                         std::string                       const &slotStr,
                                                         wago::wdx::device_collection_id_t       &coll,
                                                         wago::wdx::slot_index_t                 &slot);

/**
 * Uses the round-trippable string representation to (re)create a `parameter_instance_id`.
 * Will throw if the string is malformed.
 */
template <>
wago::wdx::parameter_instance_id from_string<wago::wdx::parameter_instance_id>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `parameter_instance_path`.
 * Will throw if the string is malformed. NOTE: Will not check if the members make sense.
 */
template <>
wago::wdx::parameter_instance_path from_string<wago::wdx::parameter_instance_path>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `parameter_id_t`.
 */
template <>
wago::wdx::parameter_id_t from_string<wago::wdx::parameter_id_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `instance_id_t`.
 */
template <>
wago::wdx::instance_id_t from_string<wago::wdx::instance_id_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `device_collection_id_t`.
 */
template <>
wago::wdx::device_collection_id_t from_string<wago::wdx::device_collection_id_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `slot_index_t`.
 * NOTE: This declaration is just for documentation purposes, its definition is equivalent with that of `instance_id_t` since they both alias to uint16_t.
 */
template <>
wago::wdx::slot_index_t from_string<wago::wdx::slot_index_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `parameter_instance_path_t`.
 * NOTE: Will not validate the path contents.
 */
template <>
wago::wdx::parameter_instance_path_t from_string<wago::wdx::parameter_instance_path_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `name_t`.
 * NOTE: This declaration is just for documentation purposes, its definition is equivalent with that of `parameter_instance_path_t` since they both alias to std::string.
 */
template <>
wago::wdx::name_t from_string<wago::wdx::name_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `monitoring_list_id_t`.
 */
template <>
wago::wdx::monitoring_list_id_t from_string<wago::wdx::monitoring_list_id_t>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `status_codes` member.
 * NOTE: Does not check for valid enum membership.
 */
template <>
wago::wdx::status_codes from_string<wago::wdx::status_codes>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `parameter_value_types`.
 */
template <>
wago::wdx::parameter_value_types from_string<wago::wdx::parameter_value_types>(std::string const &s);

/**
 * Uses the round-trippable string representation to (re)create a `parameter_value_rank`.
 */
template <>
wago::wdx::parameter_value_rank from_string<wago::wdx::parameter_value_rank>(std::string const &s);


}}

#endif // SRC_LIBWDXCORE_WDA_IPC_REPRESENTATION_HPP_
