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

#ifndef INC_WAGO_WDX_PARAMETER_PROVIDER_I_HPP_
#define INC_WAGO_WDX_PARAMETER_PROVIDER_I_HPP_

#include <vector>
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/responses.hpp"
#include "wago/wdx/requests.hpp"
#include <wago/future.hpp>
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"

namespace wago {
namespace wdx {

/**
\interface parameter_provider_i
    A ParameterProvider implements reading and writing parameters for a specific domain,
    and encapsulates its necessary knowledge and mechanisms of specific technologies, e.g. the kbus register communication or the linux network setting files.
*/
class parameter_provider_i
{
public:

    /**
        The display name of the ParameterProvider, used only for logging and troubleshooting.
        It is expected not to throw.
        If somehow the parameter provider is not able to return a meaningful value, it is okay to return e.g. 'unknown'.
    */
    virtual std::string display_name() = 0;

    /**
        Tells the `ParameterService` which parameters the ParameterProvider will provide values for.
        The `ParameterService` will call this method once at the beginning.
        It is expected not to throw. Errors should instead be reported with the response status code.
        If the returned status code is not ::success, the ParameterProvider will not be used in any way.
     */
    virtual parameter_selector_response get_provided_parameters() = 0;

    /** The `ParameterService` will call this method to retrieve the actual values of parameters.
        It is expected not to throw. Errors should instead be reported (separately for each parameter) with the `response.status`:
        - If the underlying technology has some problems (e.g. connection loss, missing resources): `status_codes::parameter_value_unavailable`
        - If the requested class instance could not be found: `status_codes::unknown_class_instance_path`
        - Any unexpected error in the ParameterProvider itself: `status_codes::internal_error`
        If the parameter is a status value (not a user setting) and its value expectedly cannot be determined,
        `status_codes::status_value_unavailable` indicates absence of a value without indicating an error.
        The `parameter_ids` will only contain those that this ParameterProvider has provided for (e.g. via `provide_devices`),
        and only those that where already validated for existence, access rights and other model attributes.
        If an unknown parameter id is encountered, a ParameterProvider can either ignore it or return `::parameter_not_provided`.
        If a parameter is ignored (the `status_code` for the respective `value_response` remains ::no_error_yet),
        the ParameterService will interpret it as a parameter that is not provided and return `::parameter_not_provided` to clients.
        The ParameterProvider should, if possible, retrieve multiple values en block or in parallel, depending on its underlying technology.
    */
    virtual future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) = 0;

    /** The `ParameterService` will call this method to set values of parameters.
        It is expected not to throw. Errors should instead be reported (separately for each parameter) with the `response.status`:
        - If the underlying technology has some problems (e.g. connection loss, missing resources): `status_codes::could_not_set_parameter`
        - One or more parameters did not pass final validation: `status_codes::invalid_value`
        - Parameter was valid, but was withheld because other parameters in consistency set were invalid or otherwise erroneous: `other_invalid_value_in_set`
        - For problems with values of type file_id: `status_codes::logic_error` and `status_codes::file_id_mismatch`, see `create_parameter_upload_id` for more information.
        - Any unexpected error in the ParameterProvider itself: `status_codes::internal_error`
        The `value_requests` will only contain those that this ParameterProvider has provided for (e.g. via `provide_devices`).
        If an unknown parameter id is encountered, a ParameterProvider can either ignore it or return `::parameter_not_provided`.
        If an inactive parameter id is encountered and won't be persisted, a ParameterProvider should return `::ignored`.
        The values will already be checked for their type, rank, access rights and other model attributes by the `ParameterService`.
        The logical checks must be done by the ParameterProvider or its underlying technology.
        The parameter provider should set `status_codes::success_but_value_adjusted` if a written value is (or will be) adjusted in a way that it's representation will differ on read. 
        The ParameterProvider should, if possible, set multiple values en block or in parallel, depending on its underlying technology.
        IMPORTANT: The ParameterProvider should, if possible, treat the batch of parameters as one consistent set (either all are written or none). 
        - If validation can be made beforehand, that should be done.
        - If the underlying technology has the ability to check and set the whole batch, that should be taken advantage of.
        - If possible, rollback on errors.
        - There can be exceptions where it is not possible or not feasible to have consistency. These cases should be documented.
        NOTE: ParameterProviders for writeable parameters of type `file_id` should see `create_parameter_upload_id` for information how to handle these.
    */
    virtual future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) = 0;

    /** 
     * Advanced `set_parameter_values` for ParameterProviders that are aware that some of their provided parameters might change
     * the WDA web connection in a way that a client cannot receive a response anymore.
     * ParameterProviders that subclass `base_parameter_provider` can choose to only override the simple one.
     * Otherwise, both methods must be implemented.
     * 
     * The description of the simple `set_parameter_values`-variant applies here as well,
     * with additional requirement:
     * If `defer_wda_web_connection_changes` is `false`, the ParameterProvider must apply parameter values just as the simple `set_parameter_values` variant would.
     * If `defer_wda_web_connection_changes` is `true`, the ParameterProvider must not actually apply
     * parameter values that change the WDA web connection in a way that could prevent an answer (https://waw/display/ENGSOFT/Verhalten+beim+Schreiben+von+verbindungsrelevanten+Einstellungen).
     * Instead, the ParameterProvider must only validate them and then return `status_codes::wda_connection_changes_deferred`.
     * Other parameters can still be applied as usual if appropriate.
     * 
     * If the result set contains at least one `set_parameter_response` with `status_codes::wda_connection_changes_deferred`,
     * the `ParameterService` will first deliver the complete response to the client,
     * and then make another call to `set_parameter_values`, only with the `value_request`s that were deferred,
     * and this time with `defer_wda_web_connection_changes` = `false`.
    */
    virtual future<std::vector<set_parameter_response>> set_parameter_values_connection_aware(std::vector<value_request> value_requests, bool defer_wda_web_connection_changes) = 0;

    /** The `ParameterService` will call this method to invoke a device function.
        It is expected not to throw. Errors should instead be reported with the `response.status`:
        - If the underlying technology has some problems (e.g. connection loss, missing resources): `status_codes::could_not_invoke_method`
        - If the requested class instance of the method could not be found: `status_codes::unknown_class_instance_path`
        - One or more arguments did not pass final validation: `status_codes::invalid_value`
        - Any unexpected error in the ParameterProvider itself: `status_codes::internal_error`
        If an unknown method id is encountered, a ParameterProvider can either ignore it or return `::parameter_not_provided`.
        The method and its inArgs will already be checked for their type, rank, access rights and other model attributes by the `ParameterService`.
        The logical checks must be done by the ParameterProvider or its underlying technology.
        For method-specific errors, the `method_invocation_response::domain_specific_status_code` should be used, according to the status codes defined for this method in the model.
        In that case, the base status_code will be status_codes::could_not_invoke_method.
    */
    virtual future<method_invocation_response> invoke_method(parameter_instance_id method_id, std::vector<std::shared_ptr<parameter_value>> in_args) = 0;

    /**
    For ParameterProviders that provide writeable parameters with type `file_id`.
    The `ParameterService` will call this method to create a `file_id` for an upload associated with a parameter `context` of type `file_id`.
    The ParameterProvider should treat the uploaded file as temporary;
    it will be finalized with a call to `set_parameter_values` where the parameter `context`s value will be set to the upload file_id.
    During that call, the ParameterProvider has the opportunity to validate the uploaded file content alongside other parameter values,
    and can reject the content with `status_codes::invalid_value`.
    - If no upload file_id is known: `status_codes::logic_error`
    - If `id` does not match a known upload file_id: `status_codes::file_id_mismatch`
    If the content is valid, the ParameterProvider should
    - take over the uploaded file contents as new content
    - return the set `file_id` as new value for subsequent parameter reads
    - make sure the associated file provider only accepts reads and no further writes, until a new file_id for an upload is requested.
    It is expected not to throw. An unexpected error should be reported with `status_codes::internal_error`.
    */
    virtual future<file_id_response> create_parameter_upload_id(parameter_id_t context) = 0;
    
    /**
    For ParameterProviders that provide writeable parameters with type `file_id`.
    The `ParameterService` will call this method to cleanup a timed out `file_id` previously created with `create_parameter_upload_id`.
    The ParameterProvider should cleanup the temporary resources and unregister the associated file_provider.
    It is expected not to throw. Errors should instead be reported with the `response.status`:
    - If no upload file_id is known: `status_codes::logic_error`
    - If `id` does not match a known upload file_id: `status_codes::file_id_mismatch`
    - Any unexpected error in the ParameterProvider itself: `status_codes::internal_error`
    */
    virtual future<response> remove_parameter_upload_id(file_id id, parameter_id_t context) = 0;

    virtual ~parameter_provider_i() noexcept = default;
};

}
}
#endif // INC_WAGO_WDX_PARAMETER_PROVIDER_I_HPP_
