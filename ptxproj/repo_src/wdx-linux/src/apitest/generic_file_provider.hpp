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
///  \brief    API-Test generic file provider
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_APITEST_GENERIC_FILE_PROVIDER_HPP_
#define SRC_APITEST_GENERIC_FILE_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/linuxos/file/file_provider_extended_i.hpp>

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

using wdx::file_provider_i;
using wdx::response;
using wdx::file_info_response;
using wdx::file_read_response;


class generic_file_provider final : public file::file_provider_extended_i
{
WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(generic_file_provider)
private:
    std::vector<uint8_t> file_content_m;
    size_t               capacity_m;
    bool                 writable_m;

public:
    explicit generic_file_provider(std::vector<uint8_t> initial_content,
                                   bool                 writable = false);
    ~generic_file_provider() noexcept override;

    // Interface file_provider_i
    future<response> create(uint64_t capacity) override;
    future<file_info_response> get_file_info() override;
    future<file_read_response> read(uint64_t offset,
                                    size_t   length) override;
    future<response> write(uint64_t             offset,
                           std::vector<uint8_t> data) override;

    // Interface extended_file_provider_i
    future<bool> is_complete() const override;
    future<void> validate(file::file_validator validator) const override;
    future<void> finish() override;
};

} // Namespace apitest
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_APITEST_GENERIC_FILE_PROVIDER_HPP_

//---- End of source file ------------------------------------------------------
