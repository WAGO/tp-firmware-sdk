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
///  \brief    Service backend proxy for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_PROXY_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "parameter_provider_stub.hpp"
#include "file_provider_stub.hpp"
#include "registered_device_store_i.hpp"
#include "common/abstract_proxy.hpp"
#include "common/managed_object_store_i.hpp"

#include <wago/wdx/parameter_service_backend_extended_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class backend_proxy final : public abstract_proxy<wdx::parameter_service_backend_extended_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(backend_proxy)

private:
    managed_object_store_i    &object_store_m;
    registered_device_store_i &device_store_m;
    wc_trace_channels           channel_m;

public:
    backend_proxy(managed_object_id                id,
                  std::string               const &connection_name,
                  sender_i                        &sender,
                  driver_i                        &driver,
                  managed_object_store_i          &object_store,
                  registered_device_store_i       &device_store,
                  wc_trace_channels          const  channel);
    ~backend_proxy() noexcept override = default;

public:
    void reregister_devices();
    void reregister_providers();

// wdx::parameter_service_backend_i
public:
    wago::future<std::vector<wdx::response>>                        register_devices(std::vector<wdx::register_device_request> requests) override;
    wago::future<wdx::response>                                     unregister_all_devices(wdx::device_collection_id_t device_collection) override;
    wago::future<std::vector<wdx::response>>                        unregister_devices(std::vector<wdx::device_id> device_ids) override;
    wago::future<std::vector<wdx::response>>                        register_parameter_providers(std::vector<wdx::parameter_provider_i*> providers, wdx::parameter_provider_call_mode mode) override;
    void                                                            unregister_parameter_providers(std::vector<wdx::parameter_provider_i*> providers) override;
    wago::future<std::vector<wdx::response>>                        register_model_providers(std::vector<wdx::model_provider_i*> providers) override;
    void                                                            unregister_model_providers(std::vector<wdx::model_provider_i*> providers) override;
    wago::future<std::vector<wdx::response>>                        register_device_description_providers(std::vector<wdx::device_description_provider_i*> providers) override;
    void                                                            unregister_device_description_providers(std::vector<wdx::device_description_provider_i*> providers) override;
    wago::future<std::vector<wdx::response>>                        register_device_extension_providers(std::vector<wdx::device_extension_provider_i*> providers) override;
    void                                                            unregister_device_extension_providers(std::vector<wdx::device_extension_provider_i*> providers) override;
    wago::future<std::vector<wdx::register_file_provider_response>> register_file_providers(std::vector<wdx::register_file_provider_request> requests, wdx::provider_call_mode mode) override;
    void                                                            unregister_file_providers(std::vector<wdx::file_provider_i*> providers) override;

// wdx::parameter_service_backend_extended_i
public:
    wago::future<std::vector<wdx::register_file_provider_response>> reregister_file_providers(std::vector<wdx::reregister_file_provider_request> requests, wdx::provider_call_mode mode) override;

// private utilities
private:

template <class Interface>
std::vector<managed_object_id> get_provider_stub_ids(std::vector<Interface*> const &providers);

template<typename ResponseType = wdx::response>
void create_providers_proxies(std::shared_ptr<promise<std::vector<ResponseType>>>       response_promise,
                              size_t                                              const providers_count,
                              method_id_type                                      const create_proxy_id,
                              future_notifier<std::vector<managed_object_id>>&&         notifier);

template<typename ProviderType,
         typename ResponseType = wdx::response>
void remote_update_providers(std::shared_ptr<promise<std::vector<ResponseType>>>        response_promise,
                             std::vector<managed_object_id>                      const &object_ids,
                             std::vector<ProviderType*>                          const  providers,
                             future_notifier<no_return>&&                               notifier);

template<typename ResponseType,
         typename ... ArgumentTypes>
void remote_register_providers(std::shared_ptr<promise<std::vector<ResponseType>>>        response_promise,
                               std::vector<managed_object_id>                      const &object_ids,
                               method_id_type                                      const  register_id,
                               wdx::provider_call_mode                             const  mode,
                               future_notifier<std::vector<ResponseType>>&&               notifier,
                               ArgumentTypes &...                                         args);

template<typename ResponseType,
         typename ProviderStubType,
         typename ProviderType>
future<std::vector<ResponseType>> register_providers(std::vector<ProviderType*> const providers,
                                                     method_id_type             const create_proxy_id,
                                                     method_id_type             const register_id,
                                                     wdx::provider_call_mode    const mode,
                                                     wc_trace_channels          const channel);

template<typename ProviderType>
void unregister_providers(std::vector<ProviderType*> const providers,
                          method_id_type             const unregister_id);

};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_BACKEND_PROXY_HPP_
//---- End of source file ------------------------------------------------------
