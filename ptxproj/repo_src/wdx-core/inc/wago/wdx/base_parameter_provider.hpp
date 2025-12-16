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

#ifndef INC_WAGO_WDX_BASE_PARAMETER_PROVIDER_HPP_
#define INC_WAGO_WDX_BASE_PARAMETER_PROVIDER_HPP_

#include <string>
#include <vector>
#include <memory>
#include "wago/wdx/parameter_provider_i.hpp"

namespace wago {
namespace wdx {

/**
Simple implementation of `parameter_provider_i` that does just enough to satisfy the requirements.
 */
class base_parameter_provider : public parameter_provider_i
{
    public:
    std::string display_name() override;

    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values_connection_aware(std::vector<value_request> value_requests, bool defer_wda_web_connection_changes) override;
    wago::future<method_invocation_response> invoke_method(parameter_instance_id method_id, std::vector<std::shared_ptr<parameter_value>> in_args) override;
    wago::future<file_id_response> create_parameter_upload_id(parameter_id_t context) override;
    wago::future<response> remove_parameter_upload_id(file_id id, parameter_id_t context) override;
};

}
}

#endif // INC_WAGO_WDX_BASE_PARAMETER_PROVIDER_HPP_
