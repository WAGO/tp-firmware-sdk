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
#include "wago/wdx/base_parameter_provider.hpp"
#include <typeinfo>

using namespace std;

namespace wago {
namespace wdx {

string base_parameter_provider::display_name()
{
    return typeid(*this).name();
}

wago::future<vector<value_response>> base_parameter_provider::get_parameter_values(
    const vector<parameter_instance_id> parameter_ids)
{
    return resolved_future(vector<value_response>(parameter_ids.size()));
}

wago::future<vector<set_parameter_response>> base_parameter_provider::set_parameter_values(
    const vector<value_request> value_requests)
{
    return resolved_future(vector<set_parameter_response>(value_requests.size()));
}

wago::future<vector<set_parameter_response>> base_parameter_provider::set_parameter_values_connection_aware(
     vector<value_request> value_requests, bool defer_wda_web_connection_changes)
{
    (void)(defer_wda_web_connection_changes);
    return set_parameter_values(std::move(value_requests));
}

wago::future<method_invocation_response> base_parameter_provider::invoke_method(
    const parameter_instance_id method_id, const vector<shared_ptr<parameter_value>> in_args)
{
    (void)(method_id);
    (void)(in_args);
    return resolved_future(method_invocation_response());
}

wago::future<file_id_response> base_parameter_provider::create_parameter_upload_id(parameter_id_t context) {
    (void)(context);
    return resolved_future(file_id_response());
}

wago::future<response> base_parameter_provider::remove_parameter_upload_id(file_id id, parameter_id_t context) {
    (void)(id);
    (void)(context);
    return resolved_future(response());
}

}
}
