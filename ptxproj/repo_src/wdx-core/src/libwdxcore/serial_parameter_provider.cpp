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
#include "serial_parameter_provider.hpp"
#include "utils/provider_job.hpp"

namespace wago {
namespace wdx {

serial_parameter_provider::serial_parameter_provider(parameter_provider_i* wrapped_provider_)
: wrapped_provider(wrapped_provider_)
{}

serial_parameter_provider::~serial_parameter_provider() noexcept = default;

std::string serial_parameter_provider::display_name()
{
    return wrapped_provider->display_name();
}

parameter_selector_response serial_parameter_provider::get_provided_parameters()
{
    return wrapped_provider->get_provided_parameters();
}

wago::future<std::vector<value_response>> serial_parameter_provider::get_parameter_values(
    std::vector<parameter_instance_id> parameter_ids)
{
    auto promise = std::make_shared<wago::promise<std::vector<value_response>>>();

    using get_job = provider_job<std::vector<value_response>>;
    auto job = std::make_shared<get_job>(promise, [provider=wrapped_provider, parameter_ids=std::move(parameter_ids)] () {
        return provider->get_parameter_values(std::move(parameter_ids));
    });
    queue.enqueue_job(job);
 
    return promise->get_future();
}

wago::future<std::vector<set_parameter_response>> serial_parameter_provider::set_parameter_values(
    std::vector<value_request> value_requests)
{
    return set_parameter_values_connection_aware(std::move(value_requests), false);
}

wago::future<std::vector<set_parameter_response>> serial_parameter_provider::set_parameter_values_connection_aware(
    std::vector<value_request> value_requests,
    bool                       defer_wda_web_connection_changes)
{
    auto promise = std::make_shared<wago::promise<std::vector<set_parameter_response>>>();

    using set_job = provider_job<std::vector<set_parameter_response>>;
    auto job = std::make_shared<set_job>(promise, [provider=wrapped_provider, value_requests=std::move(value_requests), defer_wda_web_connection_changes] () {
        return provider->set_parameter_values_connection_aware(std::move(value_requests), defer_wda_web_connection_changes);
    });
    queue.enqueue_job(job);
 
    return promise->get_future();
}

wago::future<method_invocation_response> serial_parameter_provider::invoke_method(
    parameter_instance_id                          method_id,
    std::vector<std::shared_ptr<parameter_value>>  in_args)
{
    auto promise = std::make_shared<wago::promise<method_invocation_response>>();

    using invoke_job = provider_job<method_invocation_response>;
    auto job = std::make_shared<invoke_job>(promise, [provider=wrapped_provider, method_id=std::move(method_id), in_args=std::move(in_args)] () {
        return provider->invoke_method(std::move(method_id), std::move(in_args));
    });
    queue.enqueue_job(job);
 
    return promise->get_future();
}

wago::future<file_id_response> serial_parameter_provider::create_parameter_upload_id(parameter_id_t context)
{
    auto promise = std::make_shared<wago::promise<file_id_response>>();

    using upload_id_job = provider_job<file_id_response>;
    auto job = std::make_shared<upload_id_job>(promise, [provider=wrapped_provider, context] () {
        return provider->create_parameter_upload_id(context);
    });
    queue.enqueue_job(job);
 
    return promise->get_future();
}

wago::future<response> serial_parameter_provider::remove_parameter_upload_id(file_id id, parameter_id_t context)
{
    auto promise = std::make_shared<wago::promise<response>>();

    using remove_upload_id_job = provider_job<response>;
    auto job = std::make_shared<remove_upload_id_job>(promise, [provider=wrapped_provider, id=std::move(id), context] () {
        return provider->remove_parameter_upload_id(std::move(id), context);
    });
    queue.enqueue_job(job);
 
    return promise->get_future();
}

}
}
