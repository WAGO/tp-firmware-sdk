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

#ifndef INC_WAGO_WDX_RESPONSES_HPP_
#define INC_WAGO_WDX_RESPONSES_HPP_

#include <memory>
#include <iostream>
#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include "wago/wdx/wdmm/base_types.hpp"
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/parameter_value.hpp"
#include "wago/wdx/status_codes.hpp"
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/parameter_instance_path.hpp"
#include "wago/wdx/parameter_selector.hpp"

namespace wago {
namespace wdx {

/**
Base struct for responses.
Its member `status` contains the information for the status of the request result.
*/
struct response
{
    /**
    The current state of the response.
    It is initialized with the temporary state `NO_ERROR_YET`, and should be set to `SUCCESS` if the request could be processed as expected, or else to some other `status_codes` member.
     */
    status_codes status = status_codes::no_error_yet;

    /**
    An optional message explaining an unsuccessful response.
    NOTE: This message might be visible outside the parameter service and outside the device.
    NOTE: Do not set directly, but with the constructor or appropriate methods of subclasses.
     */
    std::string message;

    /**
    Returns the message.
     */
    std::string get_message() const;

    /**
    True if a final erroneous state has been reached. The semantics of being erroneous can be specialized, e.g. for `set_parameter_response`.
     */
    virtual bool has_error() const;
    
    /**
    True if a final state has been reached and the request does not have to be processed any further.
     */
    bool is_determined() const;

    /**
    True if the state is ::success.
     */
    bool is_success() const;

    /**
     * Marks the response as successful.
     */
    void set_success();

    /**
     * Marks the response as unsuccessful with the given error.
     * Use this for generic errors, e.g. to signal missing resources.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_error(status_codes error, std::string error_message = "");

    response() = default;
    virtual ~response() noexcept = default;

    /**
    Creates an empty response with given status, optionally with `message`.
    NOTE: This message might be visible outside the parameter service and outside the device.
     */
    explicit response(status_codes status_code, std::string error_message = "");
};

struct response_with_domain_specific_status_code : public response
{

    /**
     * Contains a value if the parameter_provider found a domain specific problem.
     * NOTE: Do not set directly, but with `set_domain_specific_error`.
     */
    uint16_t domain_specific_status_code = 0;

    /**
     * Marks the response as unsuccessful with the given `domain_specific_status_code_`.
     * `domain_specific_status_code_` must correspond to the specified status codes in the model.
     * The general `status` will be `::parameter_value_unavailable`.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    virtual void set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message = "") = 0;

    using response::response;
    response_with_domain_specific_status_code(response_with_domain_specific_status_code const &) = default;
    response_with_domain_specific_status_code(response_with_domain_specific_status_code &&) = default;
    response_with_domain_specific_status_code& operator=(response_with_domain_specific_status_code const &) = default;
    response_with_domain_specific_status_code& operator=(response_with_domain_specific_status_code &&) = default;

protected: // Avoid instantiation of this base response type
    ~response_with_domain_specific_status_code() override = default;
    void set_domain_specific_data(uint16_t domain_specific_status_code_to_set, std::string error_message);
};

struct response_with_value : public response_with_domain_specific_status_code
{

    std::shared_ptr<parameter_value> value;

    using response_with_domain_specific_status_code::response_with_domain_specific_status_code;
    response_with_value(response_with_value const &) = default;
    response_with_value(response_with_value &&) = default;
    response_with_value& operator=(response_with_value const &) = default;
    response_with_value& operator=(response_with_value &&) = default;

protected: // Avoid instantiation of this base response type
    ~response_with_value() override = default;
};

/**
The basic information for a device.
Problems are reported with the following `status_codes`:
- UNKNOWN_DEVICE
- INVALID_DEVICE_COLLECTION
- INVALID_DEVICE_SLOT
- INTERNAL_ERROR
*/
struct device_response : public response
{
    /**
    The id of the device
    */
    device_id id;

    /**
    The order number as specified in the model, i.e. the `ParameterDefinition` "Identity/OrderNumber".
    */
    std::string order_number;

    /**
    The firmware version as specified in the model, i.e. the `ParameterDefinition` "Identity/FirmwareVersion".
    */
    std::string firmware_version;

    /**
    Device has parameters that are marked as beta.
     */
    bool has_beta_parameters = false;

    /**
    Device has parameters that are marked as deprecated.
     */
    bool has_deprecated_parameters = false;

    device_response() = default;
    device_response(device_id   const &id,
                    std::string const &order_number,
                    std::string const &firmware_version,
                    bool               has_beta_parameters = false,
                    bool               has_deprecated_parameters = false);

    using response::response;
};

/**
Contains the found devices with most basic information.
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICECOLLECTION
*/
struct device_collection_response : public response
{
    std::vector<device_response> devices;

    device_collection_response() = default;

    using response::response;
};

/**
This response contains the value of a requested parameter instance, if successful.
The type of `value` must then match the type defined in the model.
Use `set_value` which will also mark the response as successful, or `set_error` to report a problem.
If none is used and the `status_code` remains at its initial value (`::no_error_yet`),
the response will be interpreted as if the parameter was not provided at all.
Problems are to be reported with the following `status_codes`, conforming to the `parameter_provider_i` interface:
- INTERNAL_ERROR
- PARAMETER_VALUE_UNAVAILABLE
If the parameter is a status value (not a user setting) and its value expectedly could not be determined,
`status_codes::status_value_unavailable` indicates absence of a value without indicating an error.
 */
struct value_response : public response_with_value
{
    value_response(value_response const &) = default;
    value_response(value_response &&) = default;
    value_response& operator=(value_response const &) = default;
    value_response& operator=(value_response &&) = default;

    /**
    Creates an unsuccessful `value_response` with given error code and, optionally, a message.
    NOTE: This message might be visible outside the parameter service and outside the device.
     */
    explicit value_response(status_codes error, std::string error_message = "");

    /**
    Creates a `value_response` with optionally given `message`.
    NOTE: This message might be visible outside the parameter service and outside the device.
    */
    explicit value_response(uint16_t domain_specific_status_code_to_set, std::string error_message = "");

    /**
    Creates a successful `value_response` with given value
     */
    explicit value_response(std::shared_ptr<parameter_value> response_value);

    /**
    True if a final erroneous state has been reached. `status_value_unavailable` is not erroneous.
     */
    bool has_error() const override;

    /**
     * Marks the response as unsuccessful with the given `domain_specific_status_code_`.
     * `domain_specific_status_code_` must correspond to the specified status codes in the model.
     * The general `status` will be `::parameter_value_unavailable`.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message = "") override;

    /**
     * Marks the response as successful with the given value.
     */
    void set_value(std::shared_ptr<parameter_value> response_value);

    /**
     * Indicates absence of a value without indicating an error.
     * Not allowed for user settings.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_status_unavailable(std::string error_message = "");

    using response_with_value::response_with_value;
};

/**
This response contains the value or other attributes of a requested parameter instance, if successful.
The type of `value` must then match the type defined in the model.
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
- UNKNOWN_PARAMETER_ID (if request was given by `parameter_instance_id`)
- UNKNOWN_PARAMETER_PATH (if request was given by `parameter_instance_path`)
- PARAMETER_NOT_PROVIDED
- PARAMETER_VALUE_UNAVAILABLE (the `ParameterProvider`s underlying technology has some problems, e.g. connection loss, missing resources)
If the parameter is a status value (not a user setting) and its value expectedly could not be determined,
`status_codes::status_value_unavailable` indicates absence of a value without indicating an error.
 */
struct parameter_response : public response_with_value
{
    /** The associated `parameter_definition` for the parameter instance.
     * It will also be present if the actual value of the parameter instance could not be provided.
     * NOTE: Not available over IPC.*/
    std::shared_ptr<parameter_definition> definition;

    /** The id of the requested parameter instance.
     * It will also be present if the actual value of the parameter instance could not be provided.
    */
    parameter_instance_id id;

    /** The path of the requested parameter instance.
     * It will also be present if the actual value of the parameter instance could not be provided.
    */
    parameter_instance_path path;

    /**
    True if a final erroneous state has been reached. `status_value_unavailable` is not erroneous.
     */
    bool has_error() const override;

    /**
     * Marks the response as unsuccessful with the given `domain_specific_status_code_`.
     * `domain_specific_status_code_` must correspond to the specified status codes in the model.
     * The general `status` will be `::parameter_value_unavailable`.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message = "") override;

    explicit parameter_response(uint16_t domain_specific_status_code_to_set, std::string error_message = "");
    using response_with_value::response_with_value;
};

struct parameter_response_list_response : public response
{
    std::vector<parameter_response> param_responses;
    /** Known total entries available for paging through.
     *  The actual number may be larger, because parameters of dynamic instantiations might not be counted
     *  as long as the paging window has not reached the dynamic instantiations.
     */
    std::size_t total_entries = 0;

    using response::response;
};

/**
This response describes the result of setting a parameter value.
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
- UNKNOWN_PARAMETER_ID (if request was given by `parameter_instance_id`)
- UNKNOWN_PARAMETER_PATH (if request was given by `parameter_instance_path`)
- PARAMETER_NOT_PROVIDED
- WRONG_VALUE_TYPE
- WRONG_VALUE_REPRESENTATION
- INVALID_VALUE (the `ParameterProvider`s final checks were not passed)
- IGNORED (the parameter is inactive and won't be persisted)
- COULD_NOT_SET_PARAMETER (the `ParameterProvider`s underlying technology has some problems, e.g. connection loss, missing resources)
- OTHER_INVALID_VALUE_IN_SET (value was valid, but was withheld because other parameters in consistency set were not)
- LOGIC_ERROR (wrong usage of the workflow for uploading file content via parameter)
- FILE_ID_MISMATCH (wrong file_id for uploading file content via parameter)
- INSTANCE_KEY_NOT_WRITEABLE
 If the parameter might change the WDA web connection in a way that a client cannot receive a response anymore, WDX_CONNECTION_CHANGES_DEFERRED will be returned.
 See also `parameter_provider_i::set_parameter_values_connection_aware`.
 */
struct set_parameter_response : public response_with_value
{
    /**
    True if a final erroneous state has been reached. `wda_connection_changes_deferred`
    and `ignored` are not erroneous.
     */
    bool has_error() const override;

    /**
     * Marks the response as unsuccessful with the given `domain_specific_status_code_`.
     * `domain_specific_status_code_` must correspond to the specified status codes in the model.
     * The general `status` will be `::parameter_value_unavailable`.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message = "") override;

    /**
     * Marks the response as successful but with adjusted value.
     */
    void set_success_but_value_adjusted(std::shared_ptr<parameter_value> adjusted_value);

    /**
     * Marks the response as deferred, see `parameter_provider_i::set_parameter_values_connection_aware`.
     */
    void set_deferred();

    /**
    Creates a successful `set_parameter_response` with adjusted value.
     */
    explicit set_parameter_response(std::shared_ptr<parameter_value> adjusted_value);

    explicit set_parameter_response(uint16_t domain_specific_status_code_to_set, std::string error_message = "");
    using response_with_value::response_with_value;
};

/**
This response describes the result of the invoked method.
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
- UNKNOWN_PARAMETER_ID (if request was given by `parameter_instance_id`)
- UNKNOWN_PARAMETER_PATH (if request was given by `parameter_instance_path`)
- PARAMETER_NOT_PROVIDED
- WRONG_ARGUMENT_COUNT
- MISSING_ARGUMENT
- WRONG_VALUE_TYPE
- WRONG_VALUE_REPRESENTATION
- INVALID_VALUE (the `ParameterProvider`s final checks were not passed)
- COULD_NOT_INVOKE_METHOD (the `ParameterProvider`s underlying technology has some problems, e.g. connection loss, missing resources)
- WRONG_OUT_ARGUMENT_COUNT (the `ParameterProvider` has returned the wrong number of outArgs)

If the `ParameterService` can successfully relay the method invocation to the `ParameterProvider`,
the `ParameterProvider` might still run into issues.
General issues will be set in the response::status, e.g. INTERNAL_ERROR, INVALID_VALUE, COULD_NOT_INVOKE_METHOD (as described in `parameter_provider_i` interface).
For method-specific issues, the `ParameterProvider` will use the `domain_specific_status_code`.
`methodSpecificStatusCode` will correspond to the specified status codes in the model.
`out_args` will contain the resulting values defined in the model, but only if everything was successful.
 */
struct method_invocation_named_response : public response_with_domain_specific_status_code
{
    /**
     * If the method could be completed successfully, contains the out arguments according to the model, but organized as map from argument name to value.
     */
    std::map<std::string, std::shared_ptr<parameter_value>> out_args;

    /**
     * Marks the response as unsuccessful with the given `domain_specific_status_code_`.
     * `domain_specific_status_code_` must correspond to the specified status codes in the model.
     * The general `status` will be `::parameter_value_unavailable`.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message = "") override;

    explicit method_invocation_named_response(uint16_t domain_specific_status_code_to_set, std::string error_message = "");
    using response_with_domain_specific_status_code::response_with_domain_specific_status_code;
};

/**
This response describes the result of the invoked method by the `ParameterProvider`.
If the `ParameterService` can successfully relay the method invocation to the `ParameterProvider`,
the `ParameterProvider` can still run into issues.
General issues should be set with `set_error`, e.g. INTERNAL_ERROR, INVALID_VALUE, COULD_NOT_INVOKE_METHOD (as described in `parameter_provider_i` interface).
For method-specific issues, the `ParameterProvider` should use `set_domain_specific_error`.
`domain_specific_status_code` must correspond to the specified status codes in the model.
If everything goes well, use `set_out_args`. `out_args` must contain the resulting values defined in the model.
If the response is unchanged and the `status_code` remains at its initial value (`::no_error_yet`),
the response will be interpreted as if the method was not provided at all.
 */
struct method_invocation_response : public response_with_domain_specific_status_code
{
    /**
     * If the method could be completed successfully, contains the out arguments according to the model
     */
    std::vector<std::shared_ptr<parameter_value>> out_args;

    /**
     * Marks the response as successful with the given `out_args`.
     * `outArgs` must contain the resulting values defined in the model.
     */
    void set_out_args(std::vector<std::shared_ptr<parameter_value>> out_args_to_set);

    /**
    Creates a successful `method_invocation_response` with given outArgs.
     */
    explicit method_invocation_response(std::vector<std::shared_ptr<parameter_value>> out_args_to_set);

    /**
     * Marks the response as unsuccessful with the given `domain_specific_status_code_`.
     * `domain_specific_status_code_` must correspond to the specified status codes in the model.
     * The general `status` will be `::parameter_value_unavailable`.
     * NOTE: The optional `message` might be visible outside the parameter service and outside the device.
     */
    void set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message = "") override;

    explicit method_invocation_response(uint16_t domain_specific_status_code_to_set, std::string error_message = "");
    using response_with_domain_specific_status_code::response_with_domain_specific_status_code;
};

/**
Contains feature information, already with applied device specific overrides.
 */
struct device_feature_information {
    name_t name;
    std::vector<device_feature_information> includes;
    std::vector<std::shared_ptr<parameter_definition>> parameter_definitions; // with applied overrides
    bool is_beta       = false;
    bool is_deprecated = false;
};

/**
If the request was successful, contains a list with all the features a device has.
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
 */
struct feature_list_response : public response
{
    device_path_t device_path; // TODO: move to device_feature_information
    std::vector<device_feature_information> features;
    using response::response;
};

/**
If the request was successful, contains feature information of a device.
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
- UNKNOWN_FEATURE_NAME
 */
struct feature_response : public response {
    device_path_t device_path;
    device_feature_information feature;
    using response::response;
};

/**
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
 */
struct instance_list_response : public response
{
    std::vector<instance_id_t> instances;
    using response::response;
};

/**
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_DEVICE
- UNKNOWN_CLASSINSTANCE_PATH
 */
struct class_list_response : public response
{
    std::vector<name_t> classes;
    using response::response;
};

/**
Type for identifying a monitor list.
Will be assigned by the `ParameterService` for the duration of its lifetime. 
 */
using monitoring_list_id_t = uint64_t;

/**
 * Type describing a monitor list. */
struct monitoring_list_info {
    /**
    Identifies the monitor list.
    Can be used to subsequently ask for its values.
    Will be assigned by the `ParameterService` for the duration of its lifetime.
    */
    monitoring_list_id_t id;

    /**
     * Determines if the monitor list will be used exactly once. The `ParameterService` will delete it after first usage.
     */
    bool one_off;

    /**
     * The timeout in seconds. The `ParameterService` will delete it if the timeout has been exceeded after last usage.
     * The timeout will be refreshed every the monitor list is accessed.
     */
    uint16_t timeout_seconds;
};

/**
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- MONITORING_LIST_MAX_EXCEEDED
 */
struct monitoring_list_response : public response
{
    /**
     * Description of the created monitor list.
     * Should be only used if the response was successful.
     */
    monitoring_list_info monitoring_list;
    using response::response;
};

/**
 * A collection of all active monitor lists. 
 * Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
 */
struct monitoring_lists_response : public response
{
    /** The descriptions of the monitor lists. */
    std::vector<monitoring_list_info> monitoring_lists;
    using response::response;
};

/**
Problems with the monitor list are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_MONITORING_LIST
Problems regarding single parameter values are reported in the `parameter_response`s.
 */
struct monitoring_list_values_response : public response
{
    std::vector<parameter_response> parameter_values;
    monitoring_list_values_response() = default;
    using response::response;
    monitoring_list_values_response(std::vector<parameter_response> const &parameter_values);
};

/**
Problems are reported with the following `status_codes`:
- INTERNAL_ERROR
- UNKNOWN_MONITORING_LIST
 */
using delete_monitoring_list_response = response;

/**
This response contains the selected parameters that a ParameterProvider will provide for.
 */
struct parameter_selector_response : public response
{
    /**
    Contains the selected parameters that a ParameterProvider will provide for.
    */
    std::vector<parameter_selector> selected_parameters;

    parameter_selector_response() = default;
    using response::response;

    /**
     * Creates a successful `parameter_selector_response` with given `parameter_selector`s.
    */
    parameter_selector_response(std::vector<parameter_selector> const &selected_parameters_);
};

/**
This response contains the selected devices that a `device_description_provider_i` will provide WDD information for.
 */
struct device_selector_response : public response
{
    /**
    The selected devices.
    */
    std::vector<device_selector> selected_devices;

    device_selector_response() = default;
    using response::response;

    /**
     * Creates a successful `device_selector_response` with given `device_selector`s.
    */
    device_selector_response(std::vector<device_selector> selected_devices_);
};

using wdm_content_t = std::string;

/**
 * Contains WDM information.
 */
struct wdm_response : public response
{
    /** WDM contents */
    wdm_content_t wdm_content;
    
    /**
    Creates an empty `wdm_response` that will be ignored.
     */
    wdm_response() = default;
    using response::response;
  
    /**
    Creates a `wdm_response` with the content of a WDM artifact.
     */
    explicit wdm_response(wdm_content_t const &wdm_content_);
};

using wdd_content_t = std::string;
using wdd_bundle_content_t = std::string;

/**
 * Contains WDD bundle information.
 */
struct wdd_response : public response
{
    /** The content of the WDD bundle */
    wdd_bundle_content_t content;

    /**
    Creates an empty `device_model_response` that will be ignored.
     */
    wdd_response() = default;
    using response::response;

    /**
     * Use an artifact that contains only WDD information
     */
    explicit wdd_response(wdd_bundle_content_t const &content_);

    /**
     * Uses pure WDD content to wrap it into a wdd_bundle_content_t
     */
    static wdd_response from_pure_wdd(wdd_content_t const &wdd_content);
};

/**
 * Contains device extension information.
 */
class device_extension_response : public response
{
public:
    /**
    Do not use directly.
     */
    std::vector<std::string> extension_features;
    
    device_selector selected_devices = device_selector::any;

    /**
    Creates an empty `device_extension_response` that will be ignored.
     */
    device_extension_response() = default;
    using response::response;

    /**
    All devices matching the given selector additionally have the given features, until this `device_extension_provider_i` is unregistered.
     */
    device_extension_response(device_selector const &devices, std::vector<name_t> const &feature_names);
};

/**
Contains enum definition information.
Problems may result in `status_codes:unknown_enum_name`.
 */
struct enum_definition_response : response {
    std::shared_ptr<enum_definition> definition;
    using response::response;
};

}
}
#endif // INC_WAGO_WDX_RESPONSES_HPP_
