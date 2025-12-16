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
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"
#include "wago/wdx/file_transfer/base_file_provider.hpp"

namespace wago {
namespace wdx {
namespace file_transfer {

file_read_response::file_read_response(std::vector<uint8_t> data_)
{
    status = status_codes::success;
    data = std::move(data_);
}

file_info_response::file_info_response(uint64_t file_size_)
{
    status = status_codes::success;
    file_size = file_size_;
}

file_id_response::file_id_response(file_id id_)
{
    status = status_codes::success;
    registered_file_id = std::move(id_);
}

wago::future<file_read_response> base_file_provider::read(uint64_t offset, size_t length)
{
    (void)(offset);
    (void)(length);
    return resolved_future(file_read_response(status_codes::not_implemented));
}

wago::future<response> base_file_provider::write(uint64_t offset, std::vector<uint8_t> data)
{
    (void)(offset);
    (void)(data);
    return resolved_future(response{status_codes::not_implemented});
}

wago::future<file_info_response> base_file_provider::get_file_info()
{
    return resolved_future(file_info_response(status_codes::not_implemented));
}

wago::future<response> base_file_provider::create(uint64_t capacity)
{
    (void)(capacity);
    return resolved_future(response{status_codes::not_implemented});
}

}}}
