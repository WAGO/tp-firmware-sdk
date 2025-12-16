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
///  \brief    Implementation of example client using libwdxlinuxosclient.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/client/client.hpp"
#include <wago/wdx/base_parameter_provider.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \brief Minimal provider without any useful implementation.
//------------------------------------------------------------------------------
class my_provider : public wdx::base_parameter_provider
{
private:
    wdx::parameter_id_t const ordernumber = 1; // Identity/OrderNumber

public:
    wdx::parameter_selector_response get_provided_parameters() override
    {
        
        return wdx::parameter_selector_response({ wdx::parameter_selector::all_with_definition(ordernumber, wdx::device_selector::headstation()) });
    }

    wago::future<std::vector<wdx::value_response>> get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids) override
    {
        std::vector<wdx::value_response> responses(parameter_ids.size());
        if(    (parameter_ids.size() == 1)
            && (parameter_ids.at(0).id == ordernumber)
            && (parameter_ids.at(0).device == wdx::device_id::headstation))
        {
            responses[0] = wdx::value_response(wdx::parameter_value::create_string("1234-5678/0101-0202"));
        }

        return resolved_future(std::move(responses));
    }
};

//------------------------------------------------------------------------------
/// \brief Example client derived from abstract_client.
///
/// This example client creates a provider instance of my_provider and registers
/// this instance at the backend interface (both in init() method).
//------------------------------------------------------------------------------
class my_client : public abstract_client
{
public:
    //------------------------------------------------------------------------------
    /// The example my_client is simple and does not need a custom constructor.
    //------------------------------------------------------------------------------
    my_client() = default;
    ~my_client() noexcept override = default;

    std::string get_name() override
    {
        return "My Parameter Service Client App";
    }

    void init(init_args args) override
    {
        // Alternatives:
        // A default constructible provider may be instantiated as member
        // A provider with custom constructor may be constructed in client's constructor
        a_provider = std::make_unique<my_provider>();
        args.backend.register_parameter_provider(a_provider.get());
    }

private:
    std::unique_ptr<wdx::parameter_provider_i> a_provider = nullptr;
};

std::unique_ptr<abstract_client> create_client()
{
    return std::make_unique<my_client>();
}

} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
