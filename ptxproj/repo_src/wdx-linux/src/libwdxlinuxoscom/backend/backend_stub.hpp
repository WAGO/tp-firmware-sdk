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
///  \brief    Service backend stub for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_STUB_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "remote_backend_i.hpp"
#include "common/abstract_stub.hpp"
#include "backend_methods.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class backend_stub final : public abstract_stub<remote_backend_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(backend_stub)

public:
    using abstract_stub::abstract_stub;
    ~backend_stub() noexcept override = default;

private:
    void handle_call(method_id_type  method_id,
                     data_istream   &method_args) override
    {
        switch (method_id)
        {
            // Device management
            case backend_method_id::backend_register_devices:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: register devices");
                call(&remote_backend_i::remote_register_devices, method_id, method_args);
                break;
            case backend_method_id::backend_unregister_devices:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: unregister devices");
                call(&remote_backend_i::remote_unregister_devices, method_id, method_args);
                break;
            case backend_method_id::backend_unregister_all_devices:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: unregister all devices");
                call(&remote_backend_i::remote_unregister_all_devices, method_id, method_args);
                break;

            // Proxy creation
            case backend_method_id::backend_create_parameter_provider_proxies:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: create parameter provider proxies");
                call(&remote_backend_i::remote_create_parameter_provider_proxies, method_id, method_args);
                break;
            case backend_method_id::backend_create_model_provider_proxies:
                break;
            case backend_method_id::backend_create_device_description_provider_proxies:
                break;
            case backend_method_id::backend_create_device_extension_provider_proxies:
                break;
            case backend_method_id::backend_create_file_provider_proxies:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: create file provider proxies");
                call(&remote_backend_i::remote_create_file_provider_proxies, method_id, method_args);
                break;

            // Provider (proxy) management
            case backend_method_id::backend_register_parameter_providers:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: register parameter providers");
                call(&remote_backend_i::remote_register_parameter_providers, method_id, method_args);
                break;
            case backend_method_id::backend_unregister_parameter_providers:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: unregister parameter providers");
                call(&remote_backend_i::remote_unregister_parameter_providers, method_id, method_args);
                break;
            case backend_method_id::backend_update_parameter_provider_proxies:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: update parameter provider proxies");
                call(&remote_backend_i::remote_update_parameter_provider_proxies, method_id, method_args);
                break;
            case backend_method_id::backend_register_model_providers:
                break;
            case backend_method_id::backend_unregister_model_providers:
                break;
            case backend_method_id::backend_register_device_description_providers:
                break;
            case backend_method_id::backend_unregister_device_description_providers:
                break;
            case backend_method_id::backend_register_device_extension_providers:
                break;
            case backend_method_id::backend_unregister_device_extension_providers:
                break;
            case backend_method_id::backend_register_file_providers:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: register file providers");
                call(&remote_backend_i::remote_register_file_providers, method_id, method_args);
                break;
            case backend_method_id::backend_unregister_file_providers:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: unregister file providers");
                call(&remote_backend_i::remote_unregister_file_providers, method_id, method_args);
                break;
            case backend_method_id::backend_reregister_file_providers:
                WC_TRACE_SET_MARKER(channel_m, "Com backend stub: reregister file providers");
                call(&remote_backend_i::remote_reregister_file_providers, method_id, method_args);
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


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_STUB_HPP_
//---- End of source file ------------------------------------------------------
