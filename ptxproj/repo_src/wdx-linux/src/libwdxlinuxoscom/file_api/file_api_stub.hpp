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
///  \brief    File API frontend stub for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_STUB_HPP_
#define SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_stub.hpp"
#include "file_api_methods.hpp"

#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class file_api_stub final : public abstract_stub<wdx::parameter_service_file_api_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_api_stub)

    std::unique_ptr<wdx::parameter_service_file_api_i> owned_target_m;

public:
    file_api_stub(managed_object_id                                         id,
                  std::string                                        const &connection_name,
                  sender_i                                                 &sender,
                  driver_i                                                 &driver,
                  std::unique_ptr<wdx::parameter_service_file_api_i>        target,
                  wc_trace_channels                                  const  channel)
    : abstract_stub(id, connection_name, sender, driver, *target, channel)
    , owned_target_m(std::move(target))
    { }

    ~file_api_stub() noexcept override = default;

private:
    void handle_call(method_id_type  method_id,
                     data_istream   &method_args) override
    {
        switch (method_id)
        {
            case file_api_method_id::file_read:
                WC_TRACE_SET_MARKER(channel_m, "Com file API stub: file read");
                call(&wdx::parameter_service_file_api_i::file_read, method_id, method_args);
                break;
            case file_api_method_id::file_write:
                WC_TRACE_SET_MARKER(channel_m, "Com file API stub: file write");
                call(&wdx::parameter_service_file_api_i::file_write, method_id, method_args);
                break;
            case file_api_method_id::file_get_info:
                WC_TRACE_SET_MARKER(channel_m, "Com file API stub: file get info");
                call(&wdx::parameter_service_file_api_i::file_get_info, method_id, method_args);
                break;
            case file_api_method_id::file_create:
                WC_TRACE_SET_MARKER(channel_m, "Com file API stub: file create");
                call(&wdx::parameter_service_file_api_i::file_create, method_id, method_args);
                break;
            case file_api_method_id::file_create_parameter_upload_id:
                WC_TRACE_SET_MARKER(channel_m, "Com file API stub: create parameter upload id");
                call(&wdx::parameter_service_file_api_i::create_parameter_upload_id, method_id, method_args);
                break;
            default:
                break;
            }
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FILE_API_FILE_API_STUB_HPP_
//---- End of source file ------------------------------------------------------
