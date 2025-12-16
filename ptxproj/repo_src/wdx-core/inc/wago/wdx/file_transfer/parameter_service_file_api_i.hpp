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

#ifndef INC_WAGO_WDX_FILE_TRANSFER_PARAMETER_SERVICE_FILE_API_I_HPP_
#define INC_WAGO_WDX_FILE_TRANSFER_PARAMETER_SERVICE_FILE_API_I_HPP_

#include <vector>
#include <wago/future.hpp>
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"

namespace wago {
namespace wdx {
inline namespace file_transfer {

/**
This interface is used by clients that want to transfer files.
 */

class parameter_service_file_api_i
{
public:
    /**
    Reads `length` bytes of the file associated with `file_id`, starting at `offset`.
    Problems are reported with the following status_codes:
    `unknown_file_id`
    `file_not_accessible`, if resource does not exist or cannot be accessed
    `file_size_exceeded`, regarding offset/length
    `not_implemented`
    `internal_error`
     */
    virtual wago::future<file_read_response> file_read(file_id id, uint64_t offset, size_t length) = 0;
    
    /**
    Writes the given `data` to the file associated with `file_id`, starting at `offset`.
    Problems are reported with the following status_codes:
    unknown_file_id
    `file_not_accessible`, if resource does not exist or cannot be accessed
    `file_size_exceeded`, regarding offset/data length
    `invalid_value`, if file validation is implemented
    `not_implemented`
    `internal_error`
     */
    virtual wago::future<response> file_write(file_id id, uint64_t offset, std::vector<uint8_t> data) = 0;
    
    /**
    Provides information for the file associated with `file_id`.
    Problems are reported with the following status_codes:
    `unknown_file_id`
    `file_not_accessible`, if resource does not exist or cannot be accessed
    `not_implemented`
    `internal_error`
     */
    virtual wago::future<file_info_response> file_get_info(file_id id) = 0;
    
    /**
    Signals that a file should be prepared for writing.
    \arg capacity The maximal file position that will be written to (typically the final file size).
    Problems are reported with the following status_codes:
    `unknown_file_id`
    `file_not_accessible`, if resource cannot be created
    `file_size_exceeded`, regarding capacity
    `not_implemented`
    `internal_error`
    */
    virtual wago::future<response> file_create(file_id id, uint64_t capacity) = 0;

    /**
    Requests the creation of a `file_id` for an upload associated with a parameter `context` of type `file_id`.
    The `file_id` for the upload will be kept accessible for at least `timeout_seconds`.
    Problems are reported with the following status_codes:
    `unknown_device`
    `unknown_parameter_path`
    `internal_error`
    `parameter_not_provided`
    `not_a_file_id`, if parameter is not of type `file_id`,
    `upload_id_max_exceeded`, if the limit of 100 uploads is reached
    */
    virtual wago::future<file_id_response> create_parameter_upload_id(wago::wdx::parameter_instance_path context, uint16_t timeout_seconds = 3600) = 0;
    
    virtual ~parameter_service_file_api_i() noexcept = default;
};

}}}
#endif // INC_WAGO_WDX_FILE_TRANSFER_PARAMETER_SERVICE_FILE_API_I_HPP_
