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
///  \brief    Implementation of a the APITest generic method provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "generic_method_provider.hpp"

#include <thread>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

generic_method_provider::generic_method_provider(wdx::device_selector device_selector,
                                                 wdx::parameter_id_t  method_id,
                                                 method_function      method,
                                                 bool                 using_extra_thread)
: device_selector_m(device_selector)
, method_id_m(method_id)
, method_m(std::move(method))
, using_extra_thread_m(using_extra_thread)
{ }

generic_method_provider::~generic_method_provider() noexcept = default;

wdx::parameter_selector_response generic_method_provider::get_provided_parameters()
{
    return wdx::parameter_selector_response({wdx::parameter_selector::all_with_definition(method_id_m, device_selector_m)});
}

future<wdx::method_invocation_response> generic_method_provider::invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args)
{
    auto response_promise = std::make_shared<promise<wdx::method_invocation_response>>();
    if (method_id.id == method_id_m)
    {
        if (method_m != nullptr) 
        {
            if(using_extra_thread_m)
            {
                std::thread worker([response_promise,
                                    in_args,
                                    method = method_m] () {
                    response_promise->set_value(wdx::method_invocation_response(method(in_args)));
                });
                worker.detach();
            }
            else
            {
                response_promise->set_value(wdx::method_invocation_response(method_m(in_args)));
            }
        }
        else
        {
            wdx::method_invocation_response response;
            response.set_out_args({});
            response_promise->set_value(std::move(response));
        }
    }
    return response_promise->get_future();
}

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
