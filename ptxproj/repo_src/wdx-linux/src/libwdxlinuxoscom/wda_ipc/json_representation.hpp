//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_WDA_IPC_JSON_REPRESENTATION_HPP_
#define SRC_LIBWDXLINUXOSCOM_WDA_IPC_JSON_REPRESENTATION_HPP_

#include <memory>
#include "nlohmann/json.hpp"
#include <wago/wdx/wdmm/class_instantiation.hpp>
#include <wago/wdx/responses.hpp>
#include <wago/wdx/requests.hpp>
#include <wago/wdx/file_transfer/file_transfer_definitions.hpp>

using json = nlohmann::json;

namespace wago {
namespace wdx {
inline namespace wdmm {

void to_json(json& j, const status_codes& p);
void from_json(const json& j, status_codes& p);

void to_json(json& j, const device_id& p);
void from_json(const json& j, device_id& p);

void to_json(json& j, const device_selector& p);
void from_json(const json& j, device_selector& p);

void to_json(json& j, const parameter_selector& p);
void from_json(const json& j, parameter_selector& p);

void to_json(json& j, const parameter_filter& p);
void from_json(const json& j, parameter_filter& p);

void to_json(json& j, const response& p);
void from_json(const json& j, response* p);
void from_json(const json& j, response& p);

void to_json(json& j, const parameter_selector_response& p);
void from_json(const json& j, parameter_selector_response& p);

void to_json(json& j, const register_device_request& p);
void from_json(const json& j, register_device_request& p);

void to_json(json& j, const class_instantiation& p);
void from_json(const json& j, class_instantiation& p);

void to_json(json& j, const std::shared_ptr<parameter_value>& p);
void from_json(const json& j, std::shared_ptr<parameter_value>& p);

void to_json(json& j, const parameter_instance_id& p);
void from_json(const json& j, parameter_instance_id& p);

void to_json(json& j, const parameter_instance_path& p);
void from_json(const json& j, parameter_instance_path& p);

void to_json(json& j, const value_request& p);
void from_json(const json& j, value_request& p);

void to_json(json& j, const value_path_request& p);
void from_json(const json& j, value_path_request& p);

void to_json(json& j, const value_response& p);
void from_json(const json& j, value_response& p);

void to_json(json& j, const parameter_response& p);
void from_json(const json& j, parameter_response& p);

void to_json(json& j, const method_invocation_response& p);
void from_json(const json& j, method_invocation_response& p);

void to_json(json& j, const method_invocation_named_response& p);
void from_json(const json& j, method_invocation_named_response& p);

void to_json(json& j, const set_parameter_response& p);
void from_json(const json& j, set_parameter_response& p);

void to_json(json& j, const device_selector_response& p);
void from_json(const json& j, device_selector_response& p);

void to_json(json& j, const wdd_response& p);
void from_json(const json& j, wdd_response& p);

void to_json(json& j, const wdm_response& p);
void from_json(const json& j, wdm_response& p);

void to_json(json& j, const device_extension_response& p);
void from_json(const json& j, device_extension_response& p);

void to_json(json& j, const device_response& p);
void from_json(const json& j, device_response& p);

void to_json(json& j, const device_collection_response& p);
void from_json(const json& j, device_collection_response& p);

void to_json(json& j, const parameter_response_list_response& p);
void from_json(const json& j, parameter_response_list_response& p);

}

inline namespace file_transfer {

void to_json(json& j, const file_id_response& p);
void from_json(const json& j, file_id_response& p);
void to_json(json& j, const file_read_response& p);
void from_json(const json& j, file_read_response& p);
void to_json(json& j, const file_info_response& p);
void from_json(const json& j, file_info_response& p);

}

}}

#endif // SRC_LIBWDXLINUXOSCOM_WDA_IPC_JSON_REPRESENTATION_HPP_
