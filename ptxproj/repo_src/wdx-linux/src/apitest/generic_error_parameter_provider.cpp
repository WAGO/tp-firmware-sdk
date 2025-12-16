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
///  \brief    Implementation of a the APITest a generic error parameter provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "generic_error_parameter_provider.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

generic_error_parameter_provider::generic_error_parameter_provider(wdx::device_selector                       device_selector,
                                                                   std::map<wdx::parameter_id_t, error_state> status_values)
: device_selector_m(device_selector)
, status_values_m(std::move(status_values))
{

}

generic_error_parameter_provider::~generic_error_parameter_provider() noexcept = default;

wdx::parameter_selector_response generic_error_parameter_provider::get_provided_parameters()
{
    std::vector<wdx::parameter_selector> selectors;
    for (auto const &selector : status_values_m)
    {
        selectors.push_back(wdx::parameter_selector::all_with_definition(selector.first, device_selector_m));
    }
    return wdx::parameter_selector_response(selectors);
}

future<std::vector<wdx::value_response>> generic_error_parameter_provider::get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids)
{
    std::vector<wdx::value_response> responses;
    for (auto const &id : parameter_ids)
    {
        wdx::value_response resp;
        if(status_values_m.count(id.id))
        {
            auto error_status = status_values_m.at(id.id);
            resp.set_error(std::get<wdx::status_codes>(error_status), std::get<std::string>(error_status));
            uint16_t domain_code = std::get<uint16_t>(error_status);
            if(domain_code > 0)
            {
                resp.set_domain_specific_error(domain_code, std::get<std::string>(error_status));
            }
        }
        else
        {
            // cannot happen
        }
        responses.push_back(resp);
    }
    return resolved_future(std::move(responses));
}

future<std::vector<wdx::set_parameter_response>> generic_error_parameter_provider::set_parameter_values(std::vector<wdx::value_request> value_requests)
{
    std::vector<wdx::set_parameter_response> responses;
    for (auto const &request : value_requests)
    {
        wdx::set_parameter_response resp;
        if(status_values_m.count(request.param_id.id))
        {
            auto error_status = status_values_m.at(request.param_id.id);
            resp.set_error(std::get<wdx::status_codes>(error_status), std::get<std::string>(error_status));
            uint16_t domain_code = std::get<uint16_t>(error_status);
            if(domain_code > 0)
            {
                resp.set_domain_specific_error(domain_code, std::get<std::string>(error_status));
            }
        }
        else
        {
            // cannot happen
        }
        responses.push_back(resp);
    }
    return resolved_future(std::move(responses));
}

future<wdx::method_invocation_response> generic_error_parameter_provider::invoke_method(wdx::parameter_instance_id                         method_id,
                                                                                        std::vector<std::shared_ptr<wdx::parameter_value>>)
{
    wdx::method_invocation_response resp;
    if(status_values_m.count(method_id.id))
    {
        auto error_status = status_values_m.at(method_id.id);
        resp.set_error(std::get<wdx::status_codes>(error_status), std::get<std::string>(error_status));
        uint16_t domain_code = std::get<uint16_t>(error_status);
        if(domain_code > 0)
        {
            resp.set_domain_specific_error(domain_code, std::get<std::string>(error_status));
        }
    }
    else
    {
        // cannot happen
    }
    return resolved_future(std::move(resp));
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
