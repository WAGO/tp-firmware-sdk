//------------------------------------------------------------------------------
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
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
///  \brief    Implementation of a simple parameter provider with method.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "example_method_provider.hpp"
#include "defines.hpp"

#include <thread>
#include <chrono>
#include <exception>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace examples {

using wdx::parameter_selector;
using wdx::parameter_id_t;

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace {
// Internal exception to throw/catch on calculation overflow
class calculation_overflow : public std::exception {};

// \brief Helper function used to calculate our sum result.
//
// This helper function calculates our sum result.
// It's difficult to calculate and need a serious amount of time.
//
// \param  value_a  First value to add to sum.
// \param  value_b  Second value to add to sum.
uint32_t calculate_sum(uint32_t const value_a,
                       uint32_t const value_b)
{
    if(value_a > (UINT32_MAX - value_b))
    {
        throw calculation_overflow();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return value_a + value_b;
}

// \brief Helper function used to calculate our product result.
//
// This helper function calculates our product result.
// It's difficult to calculate and need a serious amount of time.
//
// \param  value_a  First value for product.
// \param  value_b  Second value for product.
uint32_t calculate_product(uint32_t const value_a,
                           uint32_t const value_b)
{
    if(value_a > (UINT32_MAX / value_b))
    {
        throw calculation_overflow();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return value_a * value_b;
}

void async_calculation(std::vector<std::shared_ptr<parameter_value>> in_args,
                       promise<method_invocation_response>           invocation_promise)
{
    // Do the work
    try
    {
        uint32_t const value_a = in_args.at(0)->get_uint32();
        uint32_t const value_b = in_args.at(1)->get_uint32();
        uint32_t const sum     = calculate_sum(value_a, value_b);
        uint32_t const product = calculate_product(value_a, value_b);

        // Proclaim result (satisfy promise)
        std::vector<std::shared_ptr<parameter_value>> out_args;
        out_args.push_back(parameter_value::create_uint32(sum));
        out_args.push_back(parameter_value::create_uint32(product));
        method_invocation_response method_response(out_args);
        invocation_promise.set_value(std::move(method_response));
    }
    catch(calculation_overflow&)
    {
        method_invocation_response method_response(1337);
        invocation_promise.set_value(std::move(method_response));
    }
    catch(...)
    {
        method_invocation_response method_response(wdx::status_codes::internal_error);
        invocation_promise.set_value(std::move(method_response));
    }

    // Work is finished, end asynchronous context
}
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
example_method_provider::example_method_provider()
{ }

example_method_provider::~example_method_provider() noexcept = default;

parameter_selector_response example_method_provider::get_provided_parameters()
{
    return parameter_selector_response({
        parameter_selector::all_with_definition(g_sandbox_id_example_calculation)
    });
}

wago::future<method_invocation_response> example_method_provider::invoke_method(parameter_instance_id                         method_id,
                                                                                std::vector<std::shared_ptr<parameter_value>> in_args)
{
    // Create a promise
    promise<method_invocation_response> invocation_promise;
    auto result = invocation_promise.get_future();

    // Handle all requests
    // From here we may do our work asynchronous,
    // but due to the value being stored in a member variable, this provider is so
    // fast that we decided to stay synchronous for this example.
    // Check requested ID
    if(method_id.id != g_sandbox_id_example_calculation)
    {
        // We should not be called for IDs we do not provide!
        // Some error occurred in parameter service, we tell him the truth...
        invocation_promise.set_value(std::move(method_invocation_response(wdx::status_codes::parameter_not_provided)));
    }
    else
    {
        // We are asked for the expected ID -> do our calculation asynchronously
        std::thread async_calculation_context(async_calculation, in_args, std::move(invocation_promise));
        async_calculation_context.detach();
    }

    // Return future (not necessarily resolved/satisfied, yet)
    return result;
}


} // Namespace examples
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
