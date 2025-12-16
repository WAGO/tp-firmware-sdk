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
#ifndef SRC_LIBWDXCORE_WDA_IPC_IPC_HPP_
#define SRC_LIBWDXCORE_WDA_IPC_IPC_HPP_

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include "wago/wdx/requests.hpp"
#include "wago/wdx/responses.hpp"
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"

namespace wago {
namespace wda_ipc {

enum serialization_method {
    JSON,
    BINARY
};

template<typename T>
std::vector<std::uint8_t> to_bytes(const T& thing, serialization_method method = serialization_method::JSON);
template<typename T>
T from_bytes(const std::vector<std::uint8_t>& s, serialization_method method = serialization_method::JSON);

template<typename T>
std::string to_ipc_string(const T& thing);
template<typename T>
T from_ipc_string(const std::string& s);

#define EXPLICIT_SERIALIZATION(T) \
template <> \
std::vector<std::uint8_t> to_bytes<T>(const T& thing, serialization_method method); \
template<> \
T from_bytes<T>(const std::vector<std::uint8_t>& s, serialization_method method); \
template <> \
std::vector<std::uint8_t> to_bytes<std::vector<T>>(const std::vector<T>& thing, serialization_method method); \
template<> \
std::vector<T> from_bytes<std::vector<T>>(const std::vector<std::uint8_t>& s, serialization_method method); \
template <> \
std::string to_ipc_string<T>(const T& thing); \
template<> \
T from_ipc_string<T>(const std::string& s); \
template <> \
std::string to_ipc_string<std::vector<T>>(const std::vector<T>& thing); \
template<> \
std::vector<T> from_ipc_string<std::vector<T>>(const std::string& s);

EXPLICIT_SERIALIZATION(wago::wdx::response)
EXPLICIT_SERIALIZATION(wago::wdx::class_instantiation)
EXPLICIT_SERIALIZATION(wago::wdx::device_id)
EXPLICIT_SERIALIZATION(wago::wdx::register_device_request)
EXPLICIT_SERIALIZATION(std::uint8_t)
EXPLICIT_SERIALIZATION(std::uint32_t)
EXPLICIT_SERIALIZATION(std::uint64_t)
EXPLICIT_SERIALIZATION(wago::wdx::file_id_response)
EXPLICIT_SERIALIZATION(wago::wdx::parameter_selector_response)
EXPLICIT_SERIALIZATION(wago::wdx::parameter_filter)
EXPLICIT_SERIALIZATION(wago::wdx::value_response)
EXPLICIT_SERIALIZATION(wago::wdx::parameter_instance_id)
EXPLICIT_SERIALIZATION(wago::wdx::parameter_instance_path)
EXPLICIT_SERIALIZATION(wago::wdx::set_parameter_response)
EXPLICIT_SERIALIZATION(wago::wdx::value_request)
EXPLICIT_SERIALIZATION(wago::wdx::value_path_request)
EXPLICIT_SERIALIZATION(wago::wdx::parameter_response)
EXPLICIT_SERIALIZATION(wago::wdx::method_invocation_response)
EXPLICIT_SERIALIZATION(wago::wdx::method_invocation_named_response)
EXPLICIT_SERIALIZATION(std::shared_ptr<wago::wdx::parameter_value>)
EXPLICIT_SERIALIZATION(wago::wdx::device_selector_response)
EXPLICIT_SERIALIZATION(wago::wdx::wdd_response)
EXPLICIT_SERIALIZATION(wago::wdx::device_extension_response)
EXPLICIT_SERIALIZATION(wago::wdx::wdm_response)
EXPLICIT_SERIALIZATION(wago::wdx::file_info_response)
EXPLICIT_SERIALIZATION(wago::wdx::file_read_response)
EXPLICIT_SERIALIZATION(wago::wdx::device_collection_response)
EXPLICIT_SERIALIZATION(wago::wdx::parameter_response_list_response)

#undef EXPLICIT_SERIALIZATION

// BACKEND
// response ✓
// std::vector<response> ✓
// std::vector<register_device_request> ✓
// device_collection_id_t (uint8_t) ✓
// file_id_response ✓
// parameter_id_t (uint32_t) ✓

// PP
// parameter_selector_response ✓
// std::vector<value_response> ✓
// std::vector<parameter_instance_id> ✓
// std::vector<set_parameter_response> ✓
// std::vector<value_request> ✓
// method_invocation_response ✓
// parameter_instance_id ✓
// std::vector<std::shared_ptr<parameter_value>> ✓

// DDP
// device_selector_response ✓
// wdd_response ✓

// DEP
// device_extension_response ✓

// MP
// wdm_response ✓

// FRONTEND

// std::vector<parameter_response> ✓
// std::vector<parameter_instance_id> ✓
// std::vector<parameter_instance_path> ✓
// device_collection_response ✓
// device_collection_id_t (uint8_t) ✓
// method_invocation_named_response ✓
// parameter_instance_id ✓
// parameter_instance_path ✓
// std::map<std::string, std::shared_ptr<parameter_value>> ✓
// std::vector<set_parameter_response> ✓
// std::vector<value_request> ✓
// std::vector<value_path_request> ✓
// device_id ✓

// std::vector<device_id> ✓
// std::vector<parameter_response_list_response> ✓
// parameter_filter ✓
// size_t ✓

// FILE_API

// file_read_response ✓
// file_info_response ✓

// FRONTEND EXTENDED

// std::vector<feature_list_response>
// std::vector<device_path_t> (device_path_t: string) ✓
// std::vector<instance_list_response>
// std::vector<class_list_response>
// monitor_list_response
// uint16_t ✓
// monitor_list_values_response
// monitor_list_id_t (uint64_t) ✓
// monitor_lists_response
// delete_monitor_list_response

}}

#endif // SRC_LIBWDXCORE_WDA_IPC_IPC_HPP_