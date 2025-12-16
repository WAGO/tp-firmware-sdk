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

#ifndef INC_WAGO_WDX_FILE_TRANSFER_FILE_PROVIDER_I_HPP_
#define INC_WAGO_WDX_FILE_TRANSFER_FILE_PROVIDER_I_HPP_

#include <vector>
#include <wago/future.hpp>
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"
#include "wago/wdx/responses.hpp"

namespace wago {
namespace wdx {
inline namespace file_transfer {

/**
    \interface file_provider_i
    Provides the transfer of a file from and/or to the device.
    One `file_provider_i` instance is responsible for exactly one file, and is registered at the `parameter_service_backend_i`.
    At registering, a unique `file_id` will be generated and associated with the registered `file_provider_i`.
    All subsequent calls on the `parameter_service_file_api_i` will use the `file_id` to reference the file,
    and the `parameter_service` will dispatch the `file_id` to the corresponding `file_provider_i`.
*/
class file_provider_i
{
    public:
    
    /**
    Reads `length` bytes of the file, starting at `offset`.
    Problems should be reported with the following `status_codes`:
    `file_not_accessible`, if resource does not exist or cannot be accessed
    `file_size_exceeded`, regarding `offset`/`length`
    `internal_error`
     */
    virtual wago::future<file_read_response> read(uint64_t offset, size_t length) = 0;
    
    /**
    Writes the given `data` to the file, starting at `offset`.
    Problems should be reported with the following `status_codes`:
    `file_not_accessible`, if resource does not exist or cannot be accessed
    `file_size_exceeded`, regarding `offset`/`data` length
    `invalid_value`, if file validation is implemented
    `internal_error`
     */
    virtual wago::future<response> write(uint64_t offset, std::vector<uint8_t> data) = 0;
    
    /**
    Provides information for the file.
    Problems should be reported with the following `status_codes`:
    `file_not_accessible`, if resource does not exist or cannot be accessed
    `internal_error`
     */
    virtual wago::future<file_info_response> get_file_info() = 0;
    
    /**
    Signals that a file should be prepared for writing.
    \arg capacity The maximal file position that will be written to (typically the final file size).
    Problems should be reported with the following `status_codes`:
    `file_not_accessible`, if resource cannot be created
    `file_size_exceeded`, regarding `capacity`
    `internal_error`
     */
    virtual wago::future<response> create(uint64_t capacity) = 0;
    
    virtual ~file_provider_i() noexcept = default;
};

}}}
#endif // INC_WAGO_WDX_FILE_TRANSFER_FILE_PROVIDER_I_HPP_
