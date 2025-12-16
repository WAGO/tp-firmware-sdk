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
///  \brief    This is an example parameter provider to show how to provide a
///            simple provider with one readonly parameter.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_EXAMPLES_PROVIDERS_PARAMETER_PROVIDER_EXAMPLE_PARAMETER_PROVIDER_HPP_
#define SRC_EXAMPLES_PROVIDERS_PARAMETER_PROVIDER_EXAMPLE_PARAMETER_PROVIDER_HPP_

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
using wdx::value_response;

//------------------------------------------------------------------------------
// class definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/// \brief Example parameter provider for WAGO Parameter Service.
///
/// This example provider shows the basics how to write an own parameter provider.
/// Therefore this provider realizes one readonly integer value with 16 bits.
/// An additional Method allows to set the integer value from an external class (not via parameter service).
//------------------------------------------------------------------------------
class example_parameter_provider final : public base_parameter_provider
{
private:
    uint16_t m_my_value;

public:
    //------------------------------------------------------------------------------
    /// Constructs a parameter (example) provider.
    //------------------------------------------------------------------------------
    example_parameter_provider();
    ~example_parameter_provider() noexcept override;

    /// \copydoc ::wago::wdx::parameter_provider_i::get_provided_parameters()
    parameter_selector_response get_provided_parameters() override;

    /// \copydoc ::wago::wdx::parameter_provider_i::get_parameter_values()
    wago::future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override;

    //------------------------------------------------------------------------------
    /// Method to set provided value.
    ///
    /// \param my_new_value
    ///   New value for provided parameter.
    //------------------------------------------------------------------------------
    void set_my_value(uint16_t const my_new_value);
};


} // Namespace examples
} // Namespace wdx
} // Namespace wago


#endif // SRC_EXAMPLES_PROVIDERS_PARAMETER_PROVIDER_EXAMPLE_PARAMETER_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
