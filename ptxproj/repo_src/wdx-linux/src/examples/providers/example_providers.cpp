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
///  \brief    Implementation to add example providers to parameter service.
///
///  \author   PEn : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "example_providers.hpp"
#include "parameter_provider/example_parameter_provider.hpp"
#include "method_provider/example_method_provider.hpp"

#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace examples {

using std::make_unique;
using wdx::parameter_service_backend_i;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
example_providers::example_providers(parameter_service_backend_i &backend)
: m_backend(backend)
{
    m_parameter_provider = make_unique<example_parameter_provider>();
    m_parameter_provider->set_my_value(42);
    m_parameter_providers.push_back(        m_parameter_provider.get() );

    m_method_provider = make_unique<example_method_provider>();
    m_parameter_providers.push_back(        m_method_provider.get() );

    auto register_parameter_provider_future_responses = m_backend.register_parameter_providers(m_parameter_providers);
    register_parameter_provider_future_responses.set_notifier([](std::vector<wdx::response> const &register_parameter_provider_responses) {
        for(auto const &register_response: register_parameter_provider_responses)
        {
            if(register_response.has_error())
            {
                wc_log(error, "Failed to register example parameter provider");
            }
        }
    });
}

example_providers::~example_providers() noexcept
{
    m_backend.unregister_parameter_providers(m_parameter_providers);
}


} // Namespace examples
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
