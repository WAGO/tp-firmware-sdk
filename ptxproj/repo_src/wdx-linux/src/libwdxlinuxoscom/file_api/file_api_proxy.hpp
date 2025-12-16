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
///  \brief    File API frontend proxy for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_PROXY_HPP_
#define SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_proxy.hpp"
#include "file_api_methods.hpp"

#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class file_api_proxy final : public abstract_proxy<wdx::parameter_service_file_api_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_api_proxy)

public:
    using abstract_proxy::abstract_proxy;
    ~file_api_proxy() noexcept override = default;

    wago::future<wdx::file_read_response> file_read(wdx::file_id id, uint64_t offset, size_t length) override
    {
        return call<future<wdx::file_read_response>>(file_api_method_id::file_read, id, offset, length);
    }

    wago::future<wdx::response> file_write(wdx::file_id id, uint64_t offset, std::vector<uint8_t> data) override
    {
        return call<future<wdx::response>>(file_api_method_id::file_write, id, offset, data);
    }

    wago::future<wdx::file_info_response> file_get_info(wdx::file_id id) override
    {
        return call<future<wdx::file_info_response>>(file_api_method_id::file_get_info, id);
    }

    wago::future<wdx::response> file_create(wdx::file_id id, uint64_t capacity) override
    {
        return call<future<wdx::response>>(file_api_method_id::file_create, id, capacity);
    }

    wago::future<wdx::register_file_provider_response> create_parameter_upload_id(wdx::parameter_instance_path context, uint16_t timeout_seconds) override
    {
        return call<future<wdx::register_file_provider_response>>(file_api_method_id::file_create_parameter_upload_id, context, timeout_seconds);
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_PROXY_HPP_
//---- End of source file ------------------------------------------------------
