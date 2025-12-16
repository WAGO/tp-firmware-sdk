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
///
///  \brief    File operations for WAGO Parameter Service file API.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_FILES_FILE_OPERATION_HPP_
#define SRC_LIBWDXWDA_FILES_FILE_OPERATION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "http/head_response.hpp"
#include "wago/wdx/wda/http/request_handler_i.hpp"

#include "wago/wdx/file_transfer/parameter_service_file_api_i.hpp"
#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

namespace http {

class request_i;

} // Namespace http

namespace files {

static constexpr uint64_t const download_chunk_size       = 1024 * 1024;      //  1 MB
static constexpr uint64_t const upload_chunk_size         = 1024 * 1024;      //  1 MB
static constexpr uint64_t const max_download_data_length  = 1024 * 1024 * 16; // 16 MB
static constexpr uint64_t const max_upload_request_length = 1024 * 1024 * 16; // 16 MB

using http::request_i;
using http::request_handler_i;

class file_operation
{
private:
    std::shared_ptr<wdx::parameter_service_file_api_i> const core_file_api_m;

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(file_operation)

public:
    explicit file_operation(std::shared_ptr<wdx::parameter_service_file_api_i> const  &core_frontend);
    ~file_operation() noexcept;

    /// Does an async core file api call to retrieve the file info
    void get_file_info(wdx::wdmm::file_id         file_id,
                       std::unique_ptr<request_i> request);

    /// Will perform one or more async core file api calls to retrieve 
    /// the whole or partial file (depends on whether the "Range" header 
    /// is set or not)
    void get_file_content(wdx::wdmm::file_id         file_id,
                          std::unique_ptr<request_i> request);

    /// Retrieve a new File-ID URL for a file upload
    void create_upload_id(std::unique_ptr<request_i> request,
                          uint16_t                   timeout);

    /// Will set file data with one or multiple core file api calls.
    void set_file_content(wdx::wdmm::file_id         file_id, 
                          std::unique_ptr<request_i> request);
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace files
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_FILES_FILE_OPERATION_HPP_
//---- End of source file ------------------------------------------------------
