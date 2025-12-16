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
///  \brief    File provider proxy for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_FILE_PROVIDER_PROXY_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_FILE_PROVIDER_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_proxy.hpp"
#include "backend_methods.hpp"

#include <wago/wdx/file_transfer/file_provider_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class file_provider_proxy final : public abstract_proxy<wdx::file_provider_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_provider_proxy)

public:
    using abstract_proxy::abstract_proxy;
    ~file_provider_proxy() noexcept override = default;

    future<wdx::file_read_response> read(uint64_t offset, size_t length) override
    {
        return call<future<wdx::file_read_response>>(file_provider_method_id::read, offset, length);
    }

    future<wdx::response> write(uint64_t offset, std::vector<uint8_t> data) override
    {
        return call<future<wdx::response>>(file_provider_method_id::write, offset, data);
    }
    
    future<wdx::file_info_response> get_file_info() override
    {
        return call<future<wdx::file_info_response>>(file_provider_method_id::get_file_info);
    }
    
    future<wdx::response> create(uint64_t capacity) override
    {
        return call<future<wdx::response>>(file_provider_method_id::create, capacity);
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_FILE_PROVIDER_PROXY_HPP_
//---- End of source file ------------------------------------------------------
