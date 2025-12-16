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
///  \brief    This is an example provider to show how to provide a
///            simple method within a parameter provider.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_EXAMPLES_PROVIDERS_METHOD_PROVIDER_EXAMPLE_METHOD_PROVIDER_HPP_
#define SRC_EXAMPLES_PROVIDERS_METHOD_PROVIDER_EXAMPLE_METHOD_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/base_parameter_provider.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace examples {

using wdx::base_parameter_provider;
using wdx::parameter_selector_response;
using wdx::parameter_instance_id;
using wdx::parameter_value;
using wdx::method_invocation_response;

//------------------------------------------------------------------------------
// class definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \brief Example parameter provider with method for WAGO Parameter Service.
///
/// This example provider shows the basics how to write an own parameter provider, providing a method.
/// Therefore this provider realizes one simple method calculating the sum of two 8 bit unsigned integers.
//------------------------------------------------------------------------------
class example_method_provider final : public base_parameter_provider
{
public:
    //------------------------------------------------------------------------------
    /// Constructs a method (example) provider.
    //------------------------------------------------------------------------------
    example_method_provider();
    ~example_method_provider() noexcept override;

    /// \copydoc ::wago::wdx::parameter_provider_i::get_provided_parameters()
    parameter_selector_response get_provided_parameters() override;

    /// \copydoc ::wago::wdx::parameter_provider_i::get_parameter_values()
    wago::future<method_invocation_response> invoke_method(parameter_instance_id                         method_id,
                                                           std::vector<std::shared_ptr<parameter_value>> in_args) override;
};


} // Namespace examples
} // Namespace wdx
} // Namespace wago


#endif // SRC_EXAMPLES_PROVIDERS_METHOD_PROVIDER_EXAMPLE_METHOD_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
