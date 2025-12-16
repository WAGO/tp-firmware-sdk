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
///  \brief    Implementation of a the APITest exceptions provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "exception_parameter_provider.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

exception_parameter_provider::exception_parameter_provider(wdx::device_selector                              device_selector,
                                                           std::map<wdx::parameter_id_t, std::exception_ptr> exceptions,
                                                           exception_mode                                    throw_or_notify)
: device_selector_m(device_selector)
, exceptions_m(std::move(exceptions))
, exception_mode_m(throw_or_notify)
{ }

exception_parameter_provider::~exception_parameter_provider() noexcept = default;

wdx::parameter_selector_response exception_parameter_provider::get_provided_parameters()
{
    std::vector<wdx::parameter_selector> selectors;
    for (auto const &selector : exceptions_m)
    {
        selectors.push_back(wdx::parameter_selector::all_with_definition(selector.first, device_selector_m));
    }
    return wdx::parameter_selector_response(selectors);
}

future<std::vector<wdx::value_response>> exception_parameter_provider::get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids)
{
    promise<std::vector<wdx::value_response>> prom;
    for (auto const &id : parameter_ids) {
        if(exceptions_m.count(id.id))
        {
            if(exception_mode_m == exception_mode::throwing)
            {
                std::rethrow_exception(exceptions_m.at(id.id));
            }
            else // exception_mode_m == exception_mode::future_notifier
            {
                prom.set_exception(exceptions_m.at(id.id));
            }
            break;
        }
    }
    return prom.get_future();
}

future<std::vector<wdx::set_parameter_response>> exception_parameter_provider::set_parameter_values(std::vector<wdx::value_request> value_requests)
{
    promise<std::vector<wdx::set_parameter_response>> prom;
    for (auto const &req : value_requests) {
        auto const &id = req.param_id;
        if(exceptions_m.count(id.id))
        {
            if(exception_mode_m == exception_mode::throwing)
            {
                std::rethrow_exception(exceptions_m.at(id.id));
            }
            else // exception_mode_m == exception_mode::future_notifier
            {
                prom.set_exception(exceptions_m.at(id.id));
            }
            break;
        }
    }
    return prom.get_future();
}

future<wdx::method_invocation_response> exception_parameter_provider::invoke_method(wdx::parameter_instance_id                         method_id,
                                                                                        std::vector<std::shared_ptr<wdx::parameter_value>>)
{
    promise<wdx::method_invocation_response> prom;
    if(exceptions_m.count(method_id.id))
    {
        if(exception_mode_m == exception_mode::throwing)
        {
            std::rethrow_exception(exceptions_m.at(method_id.id));
        }
        else // exception_mode_m == exception_mode::future_notifier
        {
            prom.set_exception(exceptions_m.at(method_id.id));
        }
    }
    return prom.get_future();
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
