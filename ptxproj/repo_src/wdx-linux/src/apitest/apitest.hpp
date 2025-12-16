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
///  \brief    APITest class to enable APITest model and providers
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_APITEST_HPP_
#define SRC_APITEST_APITEST_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_service_backend_i.hpp>
#include <wago/wdx/parameter_provider_i.hpp>
#include <wago/wdx/file_transfer/file_provider_i.hpp>

#include <vector>
#include <memory>

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace apitest {

class apitest
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(apitest)
private:
    std::vector<std::unique_ptr<wdx::parameter_provider_i>>          concurrent_mode_parameter_providers;
    std::vector<std::unique_ptr<wdx::parameter_provider_i>>          serialized_mode_parameter_providers;
    std::vector<std::unique_ptr<wdx::file_provider_i>>               file_providers;
public:
    explicit apitest(wdx::parameter_service_backend_i &backend,
                     bool                              support_error_parameters);
    ~apitest() noexcept;
};

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_APITEST_HPP_

//---- End of source file ------------------------------------------------------
