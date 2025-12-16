//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
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
///  \brief    APITest parameter provider for file parameters
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_FILE_PARAMETER_PROVIDER_HPP_
#define SRC_APITEST_FILE_PARAMETER_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/base_parameter_provider.hpp>

#include <string>
#include <vector>
#include <memory>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {

namespace wdx {
class parameter_service_backend_i;

namespace linuxos {

namespace file {
class file_parameter_handler;
}

namespace apitest {
class generic_file_provider;

class file_parameter_provider final : public wdx::base_parameter_provider
{
WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_parameter_provider)
private:
    wdx::device_selector                             const  device_selector_m;
    wdx::parameter_id_t                              const  file_parameter_id_m;
    bool                                             const  fail_on_write_m;
    wdx::parameter_service_backend_i                       &backend_m;

    std::unique_ptr<file::file_parameter_handler>        handler_m;

public:
    file_parameter_provider(wdx::device_selector             const &device_selector,
                            wdx::parameter_id_t              const &writable_file_parameter,
                            bool                                    fail_on_write_parameter,
                            wdx::parameter_service_backend_i       &backend);
    ~file_parameter_provider() noexcept override;

    wdx::parameter_selector_response get_provided_parameters() override;
    future<std::vector<wdx::value_response>> get_parameter_values(std::vector<wdx::parameter_instance_id> parameter_ids) override;
    future<wdx::file_id_response> create_parameter_upload_id(wdx::parameter_id_t context) override;
    future<wdx::response> remove_parameter_upload_id(wdx::file_id        id,
                                                     wdx::parameter_id_t context) override;
    future<std::vector<wdx::set_parameter_response>> set_parameter_values(std::vector<wdx::value_request> value_requests) override;
};


} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_FILE_PARAMETER_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
