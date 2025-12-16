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
///  \brief    Remote backend interface to manage remote provider proxies in IPC context.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_REMOTE_BACKEND_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_REMOTE_BACKEND_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_service_backend_extended_i.hpp>
#include "common/definitions.hpp"
#include "common/managed_object.hpp"

#include <wc/structuring.h>
#include <wago/future.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class remote_backend_i
{
    WC_INTERFACE_CLASS(remote_backend_i)

public:
    // Device management
    // "inherited" from original interface
    virtual future<std::vector<wdx::response>> remote_register_devices(std::vector<wdx::register_device_request> requests) = 0;
    virtual future<wdx::response>              remote_unregister_all_devices(wdx::device_collection_id_t device_collection) = 0;
    virtual future<std::vector<wdx::response>> remote_unregister_devices(std::vector<wdx::device_id> device_ids) = 0;

    // Proxy creation
    virtual future<std::vector<managed_object_id>> remote_create_parameter_provider_proxies(uint32_t count) = 0;
    virtual future<std::vector<managed_object_id>> remote_create_file_provider_proxies(uint32_t count) = 0;

    // Provider (proxy) management
    virtual future<no_return>                                         remote_update_parameter_provider_proxies(std::vector<managed_object_id>                object_ids,
                                                                                                               std::vector<std::string>                      names,
                                                                                                               std::vector<wdx::parameter_selector_response> selected_parameters) = 0;
    // inspired by original interface, but the original used to have pointers as args
    virtual future<std::vector<wdx::response>>                        remote_register_parameter_providers(std::vector<managed_object_id>    object_ids,
                                                                                                          wdx::parameter_provider_call_mode call_mode) = 0;
    virtual future<no_return>                                         remote_unregister_parameter_providers(std::vector<managed_object_id> object_ids) = 0;
    virtual future<std::vector<wdx::register_file_provider_response>> remote_register_file_providers(std::vector<managed_object_id>   object_ids,
                                                                                                     wdx::provider_call_mode          mode,
                                                                                                     std::vector<wdx::parameter_id_t> contexts) = 0;
    virtual future<no_return>                                         remote_unregister_file_providers(std::vector<managed_object_id> object_ids) = 0;
    virtual future<std::vector<wdx::register_file_provider_response>> remote_reregister_file_providers(std::vector<managed_object_id>   object_ids,
                                                                                                       wdx::provider_call_mode          mode,
                                                                                                       std::vector<wdx::parameter_id_t> contexts,
                                                                                                       std::vector<wdx::file_id>        file_ids) = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_REMOTE_BACKEND_I_HPP_
//---- End of source file ------------------------------------------------------
