//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------

#ifndef INC_WAGO_WDX_FILE_TRANSFER_BASE_FILE_PROVIDER_HPP_
#define INC_WAGO_WDX_FILE_TRANSFER_BASE_FILE_PROVIDER_HPP_

#include "wago/wdx/file_transfer/file_provider_i.hpp"

namespace wago {
namespace wdx {
inline namespace file_transfer {
 
/**
Simple implementation of `file_provider_i` that returns `status_codes::not_implemented_yet` in all methods
 */
class base_file_provider : public file_provider_i
{
    public:
    wago::future<file_read_response> read(uint64_t offset, size_t length) override;
    wago::future<response> write(uint64_t offset, std::vector<uint8_t> data) override;
    wago::future<file_info_response> get_file_info() override;
    wago::future<response> create(uint64_t capacity) override;
};

}}}
#endif // INC_WAGO_WDX_FILE_TRANSFER_BASE_FILE_PROVIDER_HPP_
