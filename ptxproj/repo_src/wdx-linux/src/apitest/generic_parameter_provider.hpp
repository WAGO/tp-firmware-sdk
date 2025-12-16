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
///  \brief    APITest generic parameter provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_GENERIC_PARAMETER_PROVIDER_HPP_
#define SRC_APITEST_GENERIC_PARAMETER_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/base_parameter_provider.hpp>

#include <map>
#include <string>
#include <memory>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

using wdx::base_parameter_provider;
using wdx::device_selector;
using wdx::parameter_id_t;
using wdx::parameter_value;
using wdx::parameter_selector_response;
using wdx::parameter_instance_id;
using wdx::value_response;
using wdx::value_request;
using wdx::set_parameter_response;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
class generic_parameter_provider final : public base_parameter_provider
{
WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(generic_parameter_provider)
private:
    device_selector                                                device_selector_m;
    std::map<parameter_id_t, std::shared_ptr<parameter_value>>     initial_values_m;
    std::map<parameter_instance_id, std::shared_ptr<parameter_value>> values_m;

public:
    generic_parameter_provider(device_selector                                            device_selector,
                               std::map<parameter_id_t, std::shared_ptr<parameter_value>> initial_values);
    ~generic_parameter_provider() noexcept override;

    parameter_selector_response get_provided_parameters() override;
    future<std::vector<value_response>> get_parameter_values(std::vector<parameter_instance_id> parameter_ids) override;
    future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;
};

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_GENERIC_PARAMETER_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
