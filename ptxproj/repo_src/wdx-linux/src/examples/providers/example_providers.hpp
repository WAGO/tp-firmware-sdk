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
///  \brief    Combination module to add example providers to parameter service.
///
///  \author   PEn : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_EXAMPLES_PROVIDERS_EXAMPLE_PROVIDERS_HPP_
#define SRC_EXAMPLES_PROVIDERS_EXAMPLE_PROVIDERS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_service_backend_i.hpp>

#include <memory>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace examples {

using std::unique_ptr;
using std::vector;

class example_parameter_provider;
class example_method_provider;

/// \brief Home for all available example providers
///
/// This class contains all available example providers and register them at the given backend interface.
//------------------------------------------------------------------------------
class example_providers final
{
private:
    wdx::parameter_service_backend_i             &m_backend;
    vector<wdx::parameter_provider_i*>            m_parameter_providers;
    unique_ptr<example_parameter_provider>        m_parameter_provider;
    unique_ptr<example_method_provider>           m_method_provider;

public:
    //------------------------------------------------------------------------------
    /// Constructs and registers all contained example providers.
    ///
    /// \param backend
    ///   Parameter Service backend reference, use to register and unregister providers.
    //------------------------------------------------------------------------------
    example_providers(wdx::parameter_service_backend_i &backend);
    ~example_providers() noexcept;
};


} // Namespace examples
} // Namespace wdx
} // Namespace wago


#endif // SRC_EXAMPLES_PROVIDERS_EXAMPLE_PROVIDERS_HPP_
//---- End of source file ------------------------------------------------------
