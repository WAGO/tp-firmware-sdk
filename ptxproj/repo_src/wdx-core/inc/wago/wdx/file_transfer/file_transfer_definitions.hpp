//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------

#ifndef INC_WAGO_WDX_FILE_TRANSFER_FILE_TRANSFER_DEFINITIONS_HPP_
#define INC_WAGO_WDX_FILE_TRANSFER_FILE_TRANSFER_DEFINITIONS_HPP_

#include <cstdint>
#include "wago/wdx/requests.hpp"
#include "wago/wdx/responses.hpp"

namespace wago {
namespace wdx {
inline namespace file_transfer {

/**
This response contains the contents of a file, if successful.
Problems are to be reported with the following `status_codes`:
- FILE_NOT_ACCESSIBLE
- FILE_SIZE_EXCEEDED
- NOT_IMPLEMENTED
- INTERNAL_ERROR
*/
struct file_read_response : public response
{
    std::vector<uint8_t> data;
    file_read_response() = default;
    using response::response;
    explicit file_read_response(std::vector<uint8_t> data_);
};

/**
This response contains information for a file, if successful.
Problems are to be reported with the following `status_codes`:
- FILE_NOT_ACCESSIBLE
- NOT_IMPLEMENTED
- INTERNAL_ERROR
*/
struct file_info_response : public response
{
    /**
    The highest offset of the file that exists, following the definition in Linux file system.
    Should be usable for a partial, sequential file upload to check which segment to upload next.
    */
    uint64_t file_size = 0;
    file_info_response() = default;
    using response::response;
    explicit file_info_response(uint64_t file_size_);
};

struct file_id_response : public response
{
    file_id registered_file_id;
    file_id_response() = default;
    using response::response;
    explicit file_id_response(file_id id_);
};

/**
Response from registering a `file_provider_i`.
Contains the `file_id` that will be associated with the `file_provider_i`.
Returns `status_codes::provider_not_operational` if the `file_provider_i` already was registered.
*/
using register_file_provider_response = file_id_response;

}}}
#endif // INC_WAGO_WDX_FILE_TRANSFER_FILE_TRANSFER_DEFINITIONS_HPP_
