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

#ifndef SRC_LIBWDXCORE_SERIAL_PARAMETER_PROVIDER_HPP_
#define SRC_LIBWDXCORE_SERIAL_PARAMETER_PROVIDER_HPP_

#include "utils/job_queue.hpp"
#include "wago/wdx/parameter_provider_i.hpp"

namespace wago {
namespace wdx {

/**
Wraps a `parameter_provider_i`-Implementation so that its methods are not called in parallel.
 */
class serial_parameter_provider : public parameter_provider_i
{
public:
    serial_parameter_provider(serial_parameter_provider const &) noexcept = default;
    serial_parameter_provider &operator=(serial_parameter_provider const &) noexcept = default;
    serial_parameter_provider(parameter_provider_i* wrapped_provider);
    ~serial_parameter_provider() noexcept override;

    std::string display_name() override;
    parameter_selector_response get_provided_parameters() override;
    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values_connection_aware(std::vector<value_request> value_requests, bool defer_wda_web_connection_changes) override;
    wago::future<method_invocation_response> invoke_method(parameter_instance_id method_id, std::vector<std::shared_ptr<parameter_value>> in_args) override;
    wago::future<file_id_response> create_parameter_upload_id(parameter_id_t context) override;
    wago::future<response> remove_parameter_upload_id(file_id id, parameter_id_t context) override;

private:
    parameter_provider_i* wrapped_provider;
    job_queue queue;
};

}
}

#endif // SRC_LIBWDXCORE_SERIAL_PARAMETER_PROVIDER_HPP_
