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
///  \brief    Implementation of a simple parameter provider with one readonly parameter.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "example_parameter_provider.hpp"
#include "defines.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace examples {

using wdx::parameter_selector;
using wdx::parameter_id_t;
using wdx::parameter_value;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {
constexpr uint16_t const g_invalid_value = 0;
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace {
 // \brief Helper function used to get a response for a read-request with a given value.
 //
 // This helper function is meant to be called from \link get_parameter_values() \endlink.
 // It will fill the given vector with a response containing the value or an error, if:
 // - the instance id is not matching the expected id or
 // - the value is the invalid value
 //
 // \param  my_value  The value to be set in the response object
 // \param  instance  The parameter instance which the value was was requested for.
 // \param  responses The created response will be stored in this vector.
void handle_request(uint16_t                    const  my_value,
                    parameter_instance_id       const  instance,
                    std::vector<value_response>       &responses)
{
    // Check requested ID
    if(instance.id != examples::g_sandbox_id_example_readonly)
    {
        // We should not be called for IDs we do not provide!
        // Some error occurred in parameter service, we tell him the truth...
        value_response response(wdx::status_codes::parameter_not_provided);
        responses.push_back(response);
    }
    else
    {
        // Check if we still have the initial value (which we handle as invalid)
        if(my_value == g_invalid_value)
        {
            // We handle the initial value as invalid, so we can not deliver some value at the moment
            value_response response(wdx::status_codes::parameter_value_unavailable);
            responses.push_back(response);
        }
        else
        {
            // We are asked for the expected ID and we have a valid value, so deliver our copy
            value_response response(parameter_value::create_uint16(my_value));
            responses.push_back(std::move(response));
        }
    }
}
} // Namespace (anonymous)

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
example_parameter_provider::example_parameter_provider()
: m_my_value(g_invalid_value)
{ }

example_parameter_provider::~example_parameter_provider() noexcept = default;

parameter_selector_response example_parameter_provider::get_provided_parameters()
{
    return parameter_selector_response({
        parameter_selector::all_with_definition(g_sandbox_id_example_readonly)
    });
}

wago::future<std::vector<value_response>> example_parameter_provider::get_parameter_values(std::vector<parameter_instance_id> const parameter_ids)
{
    // Create a promise
    promise<std::vector<value_response>> parameter_promise;
    auto result = parameter_promise.get_future();

    // Handle all requests
    // From here we may do our work asynchronous,
    // but due to the value being stored in a member variable, this provider is so
    // fast that we decided to stay synchronous for this example.
    std::vector<value_response> responses;
    for(parameter_instance_id id : parameter_ids)
    {
        handle_request(m_my_value, id, responses);
    }

    // Satisfy our promise
    parameter_promise.set_value(std::move(responses));

    // Return future (not necessarily resolved/satisfied, yet)
    return result;
}

void example_parameter_provider::set_my_value(uint16_t const my_new_value)
{
    m_my_value = my_new_value;
}


} // Namespace examples
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
