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
///  \brief    APITest generic method provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_GENERIC_METHOD_PROVIDER_HPP_
#define SRC_APITEST_GENERIC_METHOD_PROVIDER_HPP_

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

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
class generic_method_provider final : public wdx::base_parameter_provider
{
WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(generic_method_provider)
public:
    using method_function = std::function<std::vector<std::shared_ptr<wdx::parameter_value>>(std::vector<std::shared_ptr<wdx::parameter_value>> in_args)>;
private:
    wdx::device_selector device_selector_m;
    wdx::parameter_id_t  method_id_m;
    method_function      method_m;
    bool                 using_extra_thread_m;

public:
    /// Generate a noop method provider (no in and out args)
    generic_method_provider(wdx::device_selector device_selector,
                            wdx::parameter_id_t  method_id,
                            method_function      method = nullptr,
                            bool                 using_extra_thread = false);
    ~generic_method_provider() noexcept override;

    wdx::parameter_selector_response get_provided_parameters() override;
    future<wdx::method_invocation_response> invoke_method(wdx::parameter_instance_id method_id, std::vector<std::shared_ptr<wdx::parameter_value>> in_args) override;
};

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_GENERIC_METHOD_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
