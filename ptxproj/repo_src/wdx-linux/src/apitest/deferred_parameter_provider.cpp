//------------------------------------------------------------------------------
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of a the APITest deffered parameter provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "deferred_parameter_provider.hpp"


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

using wdx::parameter_selector;

deferred_parameter_provider::deferred_parameter_provider(wdx::device_selector                                       device_selector,
                                                         std::map<parameter_id_t, std::shared_ptr<parameter_value>> initial_values)
: device_selector_m(device_selector)
, initial_values_m(std::move(initial_values))
{

}

deferred_parameter_provider::~deferred_parameter_provider() noexcept = default;

parameter_selector_response deferred_parameter_provider::get_provided_parameters()
{
    std::vector<parameter_selector> selectors;
    for (auto const &initial : initial_values_m)
    {
        selectors.push_back(parameter_selector::all_with_definition(initial.first, device_selector_m));
    }
    return parameter_selector_response(selectors);
}

future<std::vector<value_response>> deferred_parameter_provider::get_parameter_values(std::vector<parameter_instance_id> parameter_ids)
{
    std::vector<value_response> responses;
    for (auto const &id : parameter_ids) {
        value_response resp;
        if (values_m.count(id) > 0)
        {
            resp = value_response(values_m.at(id));
        }
        else if(initial_values_m.count(id.id))
        {
            values_m.emplace(id, initial_values_m.at(id.id));
            resp = value_response(values_m.at(id));
        }
        else
        {
            // cannot happen
            resp = value_response(wdx::status_codes::internal_error);
        }
        responses.push_back(resp);
    }
    return resolved_future(std::move(responses));
}

future<std::vector<set_parameter_response>> deferred_parameter_provider::set_parameter_values(std::vector<value_request> value_requests)
{
    return set_parameter_values_connection_aware(std::move(value_requests), false);
}

future<std::vector<set_parameter_response>> deferred_parameter_provider::set_parameter_values_connection_aware(std::vector<value_request> value_requests, bool defer_wda_web_connection_changes)
{
    std::vector<set_parameter_response> responses;
    for (auto const &req : value_requests) {
        set_parameter_response resp;
        auto id = req.param_id;
        if(initial_values_m.count(id.id))
        {
            if (defer_wda_web_connection_changes)
            {
                resp = set_parameter_response(wdx::status_codes::wda_connection_changes_deferred);
            }
            else
            {
                values_m.erase(id);
                values_m.emplace(id, req.value);
                resp = set_parameter_response(wdx::status_codes::success);
            }
        }
        else
        {
            // cannot happen
            resp = set_parameter_response(wdx::status_codes::internal_error);
        }
        responses.push_back(resp);
    }
    return resolved_future(std::move(responses));
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
