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

#ifndef INC_WAGO_WDX_STATUS_CODES_HPP_
#define INC_WAGO_WDX_STATUS_CODES_HPP_

#include <wc/assertion.h>
#include <wc/structuring.h>

#include <string>
#include <cstdint>

// NOTE: Numbers must be valid towards JSON-RPC spec: http://www.jsonrpc.org/specification#error_object

namespace wago {
namespace wdx {

enum class status_codes : uint16_t
{
    success = 0,

    /**
    This is only used internally by the `ParameterService` to keep track of requests that are not yet determined.
    */
    no_error_yet = 1,

    internal_error = 2,
    not_implemented = 3,
    unknown_device_collection = 4,
    unknown_device = 5,
    device_already_exists = 6,
    device_description_inaccessible = 7,
    device_description_parse_error = 8,
    device_model_inaccessible = 9,
    device_model_parse_error = 10,
    unknown_include = 11,
    ambiguous_base_path = 12,
    unknown_parameter_id = 13,
    parameter_already_provided = 14,
    /// Used by `parameter_provider_i`s if the parameter cannot be read because its underlying technology has some problems (e.g. connection loss, missing resources), indicating an error
    parameter_value_unavailable = 15,
    /// parameter is known, but no parameter_provider_i is registered that can provide a value
    parameter_not_provided = 16,
    unknown_parameter_path = 17,
    /// Used by `parameter_provider_i`s if the dynamic class instance cannot be found
    unknown_class_instance_path = 18,
    not_a_method = 19,
    wrong_argument_count = 20,
    /// Used by `parameter_provider_i`s if the parameter cannot be written because its underlying technology has some problems (e.g. connection loss, missing resources)
    could_not_set_parameter = 21,
    missing_argument = 22,
    wrong_out_argument_count = 23,
    wrong_value_type = 24,
    wrong_value_representation = 25,
    /// Used by `parameter_provider_i`s if the method cannot be invoked because its underlying technology has some problems (e.g. connection loss, missing resources)
    could_not_invoke_method = 26,
    provider_not_operational = 27,
    monitoring_list_max_exceeded = 28,
    unknown_monitoring_list = 29,
    wrong_value_pattern = 30,
    parameter_not_writeable = 31,
    value_not_possible = 32,
    wdmm_version_not_supported = 33,
    invalid_device_collection = 34,
    invalid_device_slot = 35,
    value_null = 36,
    unknown_file_id = 37,
    file_not_accessible = 38,
    /// Used by `parameter_provider_i`s if the parameter value did not pass validation
    invalid_value = 39,
    file_size_exceeded = 40,
    /// Used by `parameter_provider_i`s if parameter value was valid, but was withheld because other parameters in consistency set were not
    other_invalid_value_in_set = 41,
    /// Used by `parameter_provider_i`s if an inactive parameter was encountered and was not persisted
    ignored = 42,
    /// Used by `parameter_provider_i`s if the parameter might change the WDA web connection in a way that a client cannot receive a response anymore.
    /// See `parameter_provider_i::set_parameter_values_connection_aware` for more information.
    wda_connection_changes_deferred = 43,
    methods_do_not_have_value = 44,
    not_a_file_id = 45,
    file_id_mismatch = 46,
    logic_error = 47,
    upload_id_max_exceeded = 48,
    /// Used by `parameter_provider_i`s if the status parameter value cannot be determined, but does not indicate an error. Not used for user settings.
    status_value_unavailable = 49,
    unknown_enum_name = 50,
    unknown_feature_name = 51,
    feature_not_available = 52,
    instance_key_not_writeable = 53,
    missing_parameter_for_instantiation = 54,
    not_existing_for_instance = 55,

    // Used by `parameter_provider_i`s if the value for a parameter has been accepted to be written, but the now effective value differs in a nonessential way.
    success_but_value_adjusted = 56,

    unauthorized = 57,
    other_unauthorized_request_in_set = 58,

    highest = 58 // update this when new values are added
    // NOTE: if you add an enum value, also add it in status_code_strings[]!
};

constexpr const char* const status_code_strings[] =
{
    "SUCCESS",
    "NO_ERROR_YET",
    "INTERNAL_ERROR",
    "NOT_IMPLEMENTED",
    "UNKNOWN_DEVICE_COLLECTION",
    "UNKNOWN_DEVICE",
    "DEVICE_ALREADY_EXISTS",
    "DEVICE_DESCRIPTION_INACCESSIBLE",
    "DEVICE_DESCRIPTION_PARSE_ERROR",
    "DEVICE_MODEL_INACCESSIBLE",
    "DEVICE_MODEL_PARSE_ERROR",
    "UNKNOWN_INCLUDE",
    "AMBIGUOUS_BASE_PATH",
    "UNKNOWN_PARAMETER_ID",
    "PARAMETER_ALREADY_PROVIDED",
    "PARAMETER_VALUE_UNAVAILABLE",
    "PARAMETER_NOT_PROVIDED",
    "UNKNOWN_PARAMETER_PATH",
    "UNKNOWN_CLASS_INSTANCE_PATH",
    "NOT_A_METHOD",
    "WRONG_ARGUMENT_COUNT",
    "COULD_NOT_SET_PARAMETER",
    "MISSING_ARGUMENT",
    "WRONG_OUT_ARGUMENT_COUNT",
    "WRONG_VALUE_TYPE",
    "WRONG_VALUE_REPRESENTATION",
    "COULD_NOT_INVOKE_METHOD",
    "PROVIDER_NOT_OPERATIONAL",
    "MONITORING_LIST_MAX_EXCEEDED",
    "UNKNOWN_MONITORING_LIST",
    "WRONG_VALUE_PATTERN",
    "PARAMETER_NOT_WRITEABLE",
    "VALUE_NOT_POSSIBLE",
    "WDMM_VERSION_NOT_SUPPORTED",
    "INVALID_DEVICE_COLLECTION",
    "INVALID_DEVICE_SLOT",
    "VALUE_NULL",
    "UNKNOWN_FILE_ID",
    "FILE_NOT_ACCESSIBLE",
    "INVALID_VALUE",
    "FILE_SIZE_EXCEEDED",
    "OTHER_INVALID_VALUE_IN_SET",
    "IGNORED",
    "WDX_CONNECTION_CHANGES_DEFERRED",
    "METHODS_DO_NOT_HAVE_VALUE",
    "NOT_A_FILE_ID",
    "FILE_ID_MISMATCH",
    "LOGIC_ERROR",
    "UPLOAD_ID_MAX_EXCEEDED",
    "STATUS_VALUE_UNAVAILABLE",
    "UNKNOWN_ENUM_NAME",
    "UNKNOWN_FEATURE_NAME",
    "FEATURE_NOT_AVAILABLE",
    "INSTANCE_KEY_NOT_WRITEABLE",
    "MISSING_PARAMETER_FOR_INSTANTIATION",
    "NOT_EXISTING_FOR_INSTANCE",
    "SUCCESS_BUT_VALUE_ADJUSTED",
    "UNAUTHORIZED",
    "OTHER_UNAUTHORIZED_REQUEST_IN_SET"
};

// Ensure status_code_strings are consistent with status_codes
WC_STATIC_ASSERT(((size_t)status_codes::highest + 1) == WC_ARRAY_LENGTH(status_code_strings));

enum class status_codes_context : uint16_t
{
    general,
    parameter_read,
    parameter_write
};

std::string to_string(status_codes const &status);
const char* to_c_string(status_codes const &status);
status_codes from_string(std::string const &s);
bool has_error(status_codes const &status, status_codes_context const &context = status_codes_context::general);
bool is_determined(status_codes const &status);
bool is_success(status_codes const &status, status_codes_context const &context = status_codes_context::general);

}
}
#endif // INC_WAGO_WDX_STATUS_CODES_HPP_
