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
///  \brief    Remote backend managing remote provider proxies for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_REMOTE_BACKEND_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_REMOTE_BACKEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "remote_backend_i.hpp"
#include "common/definitions.hpp"
#include "common/receiver_i.hpp"

#include <wc/structuring.h>

#include <set>

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class manager;
class managed_object_store_i;
class remote_backend final : public remote_backend_i, public receiver_i
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(remote_backend)
    using parameter_provider_pointers          = std::set<wdx::parameter_provider_i*>;
    using file_provider_pointers               = std::set<wdx::file_provider_i*>;

private:
    wdx::parameter_service_backend_extended_i &backend_m;
    managed_object_store_i                    &store_m;
    std::unique_ptr<receiver_i>                receiver_m;
    parameter_provider_pointers                registered_parameter_providers_m;
    file_provider_pointers                     registered_file_providers_m;

public:
    remote_backend(wdx::parameter_service_backend_extended_i &original_backend, std::unique_ptr<manager> manager_);
    remote_backend(wdx::parameter_service_backend_extended_i &original_backend, std::unique_ptr<receiver_i> receiver, managed_object_store_i &store);

    ~remote_backend() noexcept override;

    // Device management
    future<std::vector<wdx::response>> remote_register_devices(std::vector<wdx::register_device_request> requests) override;
    future<std::vector<wdx::response>> remote_unregister_devices(std::vector<wdx::device_id> device_ids) override;
    future<wdx::response>              remote_unregister_all_devices(wdx::device_collection_id_t device_collection) override;

    // Proxy creation
    future<std::vector<managed_object_id>> remote_create_parameter_provider_proxies(uint32_t count) override;
    future<std::vector<managed_object_id>> remote_create_file_provider_proxies(uint32_t count) override;

    // Provider (proxy) management
    future<no_return>                                         remote_update_parameter_provider_proxies(std::vector<managed_object_id>                object_ids,
                                                                                                       std::vector<std::string>                      names,
                                                                                                       std::vector<wdx::parameter_selector_response> selected_parameters) override;
    future<std::vector<wdx::response>>                        remote_register_parameter_providers(std::vector<managed_object_id>    object_ids,
                                                                                                  wdx::parameter_provider_call_mode call_mode) override;
    future<no_return>                                         remote_unregister_parameter_providers(std::vector<managed_object_id> object_ids) override;
    future<std::vector<wdx::register_file_provider_response>> remote_register_file_providers(std::vector<managed_object_id>   object_ids,
                                                                                             wdx::provider_call_mode          mode,
                                                                                             std::vector<wdx::parameter_id_t> contexts) override;
    future<no_return>                                         remote_unregister_file_providers(std::vector<managed_object_id> object_ids) override;
    future<std::vector<wdx::register_file_provider_response>> remote_reregister_file_providers(std::vector<managed_object_id>   object_ids,
                                                                                               wdx::provider_call_mode          mode,
                                                                                               std::vector<wdx::parameter_id_t> contexts,
                                                                                               std::vector<wdx::file_id>        file_ids) override;

private:
    template<class Proxy,
             class Interface = typename Proxy::interface>
    std::vector<Interface*> get_provider_proxy_interfaces(std::vector<managed_object_id> object_ids);

    template<class Proxy>
    std::vector<Proxy*> get_provider_proxies(std::vector<managed_object_id> object_ids);

    template <class Proxy>
    std::vector<managed_object_id> create_provider_proxies(uint32_t count);

public:
    void receive(message_data message) override;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_REMOTE_BACKEND_HPP_
//---- End of source file ------------------------------------------------------
