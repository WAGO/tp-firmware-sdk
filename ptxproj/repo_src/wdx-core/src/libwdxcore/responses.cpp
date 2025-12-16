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
#include "wago/wdx/responses.hpp"

namespace wago {
namespace wdx {

bool response::has_error() const
{
    return wago::wdx::has_error(status);
}

bool response::is_determined() const
{
    return wago::wdx::is_determined(status);
}

bool response::is_success() const
{
    return wago::wdx::is_success(status);
}

std::string response::get_message() const
{
    return message;
}

void response::set_success()
{
    status = status_codes::success;
}

void response::set_error(status_codes error, std::string error_message)
{
    status  = error;
    message = std::move(error_message);
}

response::response(status_codes status_code, std::string error_message)
: status(status_code)
, message(std::move(error_message))
{ }

void response_with_domain_specific_status_code::set_domain_specific_data(uint16_t domain_specific_status_code_to_set, std::string error_message)
{
    domain_specific_status_code = domain_specific_status_code_to_set;
    message                     = std::move(error_message);

}

device_response::device_response(device_id   const &id_,
                                 std::string const &order_number_,
                                 std::string const &firmware_version_,
                                 bool               has_beta_parameters_,
                                 bool               has_deprecated_parameters_)
: response(status_codes::success)
, id(id_)
, order_number(order_number_)
, firmware_version(firmware_version_)
, has_beta_parameters(has_beta_parameters_)
, has_deprecated_parameters(has_deprecated_parameters_)
{ }

value_response::value_response(status_codes error, std::string error_message)
: response_with_value(error, std::move(error_message))
{ }

value_response::value_response(uint16_t domain_specific_status_code_to_set, std::string error_message)
: response_with_value()
{
    value_response::set_domain_specific_error(domain_specific_status_code_to_set, std::move(error_message));
}

value_response::value_response(std::shared_ptr<parameter_value> response_value)
: response_with_value(status_codes::success)
{
    value = std::move(response_value);
}

bool value_response::has_error() const
{
    return wago::wdx::has_error(status, status_codes_context::parameter_read);
}

void value_response::set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message)
{
    status = status_codes::parameter_value_unavailable;
    set_domain_specific_data(domain_specific_status_code_to_set, std::move(error_message));
}

void value_response::set_value(std::shared_ptr<parameter_value> value_)
{
    status = status_codes::success;
    domain_specific_status_code = 0;
    value = std::move(value_);
}

void value_response::set_status_unavailable(std::string error_message)
{
    status  = status_codes::status_value_unavailable;
    message = std::move(error_message);
}

bool parameter_response::has_error() const
{
    return wago::wdx::has_error(status, status_codes_context::parameter_read);
}

void parameter_response::set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message)
{
    status = status_codes::parameter_value_unavailable;
    set_domain_specific_data(domain_specific_status_code_to_set, std::move(error_message));
}

parameter_response::parameter_response(uint16_t domain_specific_status_code_to_set, std::string error_message)
: response_with_value()
{
    parameter_response::set_domain_specific_error(domain_specific_status_code_to_set, std::move(error_message));
}

bool set_parameter_response::has_error() const
{
    return wago::wdx::has_error(status, status_codes_context::parameter_write);
}

void set_parameter_response::set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message)
{
    status = status_codes::could_not_set_parameter;
    set_domain_specific_data(domain_specific_status_code_to_set, std::move(error_message));
}

set_parameter_response::set_parameter_response(std::shared_ptr<parameter_value> adjusted_value)
: response_with_value(status_codes::success_but_value_adjusted)
{ 
    value = std::move(adjusted_value);
}

void set_parameter_response::set_success_but_value_adjusted(std::shared_ptr<parameter_value> adjusted_value)
{
    status = status_codes::success_but_value_adjusted;
    domain_specific_status_code = 0;
    value = adjusted_value;
}

void set_parameter_response::set_deferred()
{
    status = status_codes::wda_connection_changes_deferred;
}

set_parameter_response::set_parameter_response(uint16_t domain_specific_status_code_to_set, std::string error_message)
: response_with_value()
{
    set_parameter_response::set_domain_specific_error(domain_specific_status_code_to_set, std::move(error_message));
}

void method_invocation_named_response::set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message)
{
    status = status_codes::could_not_invoke_method;
    set_domain_specific_data(domain_specific_status_code_to_set ,std::move(error_message));
}

method_invocation_named_response::method_invocation_named_response(uint16_t domain_specific_status_code_to_set, std::string error_message)
: response_with_domain_specific_status_code()
{
    method_invocation_named_response::set_domain_specific_error(domain_specific_status_code_to_set, std::move(error_message));
}

method_invocation_response::method_invocation_response(std::vector<std::shared_ptr<parameter_value>> out_args_to_set)
: response_with_domain_specific_status_code(status_codes::success)
, out_args(out_args_to_set)
{ }

method_invocation_response::method_invocation_response(uint16_t domain_specific_status_code_to_set, std::string error_message)
: response_with_domain_specific_status_code()
{
    method_invocation_response::set_domain_specific_error(domain_specific_status_code_to_set, std::move(error_message));
}

void method_invocation_response::set_domain_specific_error(uint16_t domain_specific_status_code_to_set, std::string error_message)
{
    status = status_codes::could_not_invoke_method;
    set_domain_specific_data(domain_specific_status_code_to_set, std::move(error_message));
}

void method_invocation_response::set_out_args(std::vector<std::shared_ptr<parameter_value>> out_args_to_set)
{
    status = status_codes::success;
    domain_specific_status_code = 0;
    out_args = out_args_to_set;
}

monitoring_list_values_response::monitoring_list_values_response(std::vector<parameter_response> const &parameter_values_)
: response(status_codes::success)
, parameter_values(parameter_values_)
{ }

parameter_selector_response::parameter_selector_response(std::vector<parameter_selector> const &selected_parameters_)
: response(status_codes::success)
, selected_parameters(selected_parameters_)
{ }

device_selector_response::device_selector_response(std::vector<device_selector> selected_devices_)
: response(status = status_codes::success)
, selected_devices (std::move(selected_devices_))
{ }

wdd_response::wdd_response(wdd_bundle_content_t const &content_)
: response(status_codes::success)
, content(content_)
{ }

wdd_response wdd_response::from_pure_wdd(wdd_content_t const &wdd_content)
{
    // TODO: wrap into bundle
    return wdd_response(wdd_content);
}

wdm_response::wdm_response(wdm_content_t const &content_)
: wdm_content(content_)
{
    status = status_codes::success;
}

device_extension_response::device_extension_response(device_selector const &devices, std::vector<name_t> const &feature_names_)
:response(status_codes::success)
, selected_devices(devices)
{
    extension_features = feature_names_;
}

}
}
