//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    Implementation of a the APITest generic parameter provider for constant responses
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "generic_response_provider.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

using wdx::parameter_selector;

generic_response_provider::generic_response_provider(wdx::device_selector                             device_selector,
                                                     std::map<parameter_id_t, value_response>         responses,
                                                     std::map<parameter_id_t, set_parameter_response> set_responses)
: device_selector_m(device_selector)
, responses_m(std::move(responses))
, set_responses_m(std::move(set_responses))
{

}

generic_response_provider::~generic_response_provider() noexcept = default;

parameter_selector_response generic_response_provider::get_provided_parameters()
{
    std::vector<parameter_selector> selectors;
    for (auto const &response_entry : responses_m)
    {
        selectors.push_back(parameter_selector::all_with_definition(response_entry.first, device_selector_m));
    }
    return parameter_selector_response(selectors);
}

future<std::vector<value_response>> generic_response_provider::get_parameter_values(std::vector<parameter_instance_id> parameter_ids)
{
    std::vector<value_response> responses;
    for (auto const &id : parameter_ids) {
        value_response resp;
        if(responses_m.count(id.id))
        {
            resp = responses_m.at(id.id);
        }
        else
        {
            // not intended
            resp = value_response(wdx::status_codes::internal_error);
        }
        responses.push_back(resp);
    }
    return resolved_future(std::move(responses));
}

future<std::vector<set_parameter_response>> generic_response_provider::set_parameter_values(std::vector<value_request> value_requests)
{
    std::vector<set_parameter_response> responses;
    for (auto const &id : value_requests) {
        set_parameter_response resp;
        if(set_responses_m.count(id.param_id.id))
        {
            resp = set_responses_m.at(id.param_id.id);
        }
        else
        {
            // not intended
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
