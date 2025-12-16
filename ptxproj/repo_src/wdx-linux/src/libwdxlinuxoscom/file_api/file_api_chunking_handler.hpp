//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    File API handler to limit read/write call chunk sizes.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_CHUNKING_HANDLER_HPP_
#define SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_CHUNKING_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>

#include <stack>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class file_api_chunking_handler final : public wdx::parameter_service_file_api_i
{
private:
    parameter_service_file_api_i       &original_interface_m;
    ssize_t                      const  max_read_chunk_size_m;
    ssize_t                      const  max_write_chunk_size_m;

public:
    file_api_chunking_handler(parameter_service_file_api_i &original_interface,
                              ssize_t                       max_read_chunk_size,
                              ssize_t                       max_write_chunk_size);

    future<wdx::file_read_response> file_read(wdx::file_id id,
                                              uint64_t     offset,
                                              size_t       length) override;
    future<wdx::response> file_write(wdx::file_id         id,
                                     uint64_t             offset,
                                     std::vector<uint8_t> data) override;
    future<wdx::file_info_response> file_get_info(wdx::file_id id) override;
    future<wdx::response> file_create(wdx::file_id id,
                                      uint64_t     capacity) override;
    future<wdx::register_file_provider_response> create_parameter_upload_id(wdx::parameter_instance_path context,
                                                                            uint16_t                     timeout_seconds) override;

private:
    using data_collector = std::shared_ptr<std::vector<uint8_t>>;
    future<wdx::response> read_more(wdx::file_id   id,
                                    uint64_t       offset,
                                    size_t         length,
                                    data_collector data);
    using data_stack = std::stack<std::vector<uint8_t>>;
    future<wdx::response> write_more(wdx::file_id id,
                                     uint64_t     offset,
                                     data_stack   data);
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_CHUNKING_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
