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
///  \brief    Implementation for file factory used in unit tests.
///
///  \author   MaHe:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_factory.hpp"
#include "file_factory_override.hpp"

#include <wc/assertion.h>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace {

std::unique_ptr<mock_file>   next_file_mock = nullptr;

}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

file_factory_override::file_factory_override()
{
    WC_ASSERT(next_file_mock == nullptr);
    next_file_mock = std::make_unique<mock_file>();
}

file_factory_override::~file_factory_override()
{
    next_file_mock = nullptr;
}

mock_file *file_factory_override::get_next_to_be_created_mock()
{
    return next_file_mock.get();
}

namespace wago {
namespace wdx {
namespace linuxos {
namespace file {
namespace sal {

std::unique_ptr<file_i> file_factory::create_for_read_access(std::string const &WC_UNUSED_PARAM(file_path),
                                                             bool               WC_UNUSED_PARAM(no_empty_file_on_disk),
                                                             bool               WC_UNUSED_PARAM(keep_file_open))
{
    auto file_mock = std::move(next_file_mock);
    file_mock->set_default_expectations();
    next_file_mock = std::make_unique<mock_file>();
    return file_mock;
}

std::unique_ptr<file_i> file_factory::create_for_write_access(std::string const &WC_UNUSED_PARAM(file_path),
                                                              bool               WC_UNUSED_PARAM(no_empty_file_on_disk),
                                                              uint64_t           WC_UNUSED_PARAM(file_capacity),
                                                              mode_t      const  WC_UNUSED_PARAM(file_mode),
                                                              std::string const &WC_UNUSED_PARAM(file_owner))
{
    auto file_mock = std::move(next_file_mock);
    file_mock->set_default_expectations();
    next_file_mock = std::make_unique<mock_file>();
    return file_mock;
}

std::unique_ptr<file_i> file_factory::create_for_write_access(std::string const &WC_UNUSED_PARAM(file_path),
                                                              bool               WC_UNUSED_PARAM(no_empty_file_on_disk),
                                                              uint64_t           WC_UNUSED_PARAM(file_capacity),
                                                              mode_t      const  WC_UNUSED_PARAM(file_mode),
                                                              uid_t       const  WC_UNUSED_PARAM(file_owner_id),
                                                              gid_t       const  WC_UNUSED_PARAM(file_group_id))
{
    auto file_mock = std::move(next_file_mock);
    file_mock->set_default_expectations();
    next_file_mock = std::make_unique<mock_file>();
    return file_mock;
}

} // Namespace sal
} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
