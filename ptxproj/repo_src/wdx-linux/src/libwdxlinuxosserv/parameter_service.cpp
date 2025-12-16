//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of WAGO Parameter Service on Linux controllers.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/serv/parameter_service.hpp"
#include "wago/wdx/wda/service_identity_i.hpp"
#include "wago/wdx/wda/parameter_service.hpp"
#include "settings_store.hpp"
#include "fcgi/server.hpp"
#include "auth/oauth2_backend.hpp"
#include "auth/permissions.hpp"
#include "model_loader.hpp"
#include "device_description_loader.hpp"
#include "device_extension_loader.hpp"
#include "system_abstraction.hpp"

#include <wago/wdx/wda/trace_routes.hpp>
#include <wc/preprocessing.h>
#include <wc/log.h>

#include <memory>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

using fcgi::server;
using wdx::wda::http::request_handler_i;
using wdx::parameter_service_i;
using wdx::parameter_provider_i;
using wdx::device_description_provider_i;
using wdx::device_selector;
using wdx::register_device_request;
using wdx::device_collections;
using wdx::response;
using std::vector;
using std::make_unique;
using std::make_shared;
using std::string;

class identity_provider : public wda::service_identity_i
{
public:
    identity_provider()                noexcept       = default;
    char const * get_id()              const override { return "wda"; };
    char const * get_name()            const override { return "WAGO Parameter Service Linux Daemon"; };
    char const * get_version_string()  const override { return WC_SUBST_STR(WDXSERV_VERSION); };
    uint16_t     get_major_version()   const override { return              WDXSERV_MAJOR; };
    uint16_t     get_minor_version()   const override { return              WDXSERV_MINOR; };
    uint16_t     get_bugfix_version()  const override { return              WDXSERV_BUGFIX; };
    char const * get_revision_string() const override { return WC_SUBST_STR(WDXSERV_REVISION); };
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char    const default_model_path[]        = WDM_DIR "/WAGO.bundle.wdm.json";
static constexpr char    const default_repository_path[]   = WDD_DIR;
static constexpr char    const additional_wdm_pattern[]    = WDE_DIR "/*.wdm.json";
static constexpr char    const additional_wdd_pattern[]    = WDE_DIR "/*.wdd.json";
static constexpr size_t  const broken_token_slowdown       = 5;

static identity_provider const identity;
//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
static void register_headstation_device(parameter_service_backend_i &backend,
                                        std::string                  ordernumber,
                                        std::string                  firmware_version)
{
    register_device_request headstation_device_request = { {  0, device_collections::root }, ordernumber, firmware_version };
    auto future_responses = backend.register_devices({headstation_device_request});
    future_responses.set_notifier([headstation_device_request](std::vector<response> responses){
        if(responses.at(0).has_error())
        {
            wc_log(error, ("Failed to register headstation device "
                            + wago::wdx::to_description(headstation_device_request.device_id)
                            + " (" + headstation_device_request.order_number + ": "
                            + to_string(responses.at(0).status) + ")").c_str());
        }
    });
    // Core promised not to use exceptions, thus an exception notifier is not necessary
}

parameter_service::parameter_service(std::string headstation_ordernumber,
                                     std::string headstation_firmware_version,
                                     std::string rest_api_base_path,
                                     std::string rest_api_socket,
                                     std::string file_api_base_path,
                                     std::string file_api_socket,
                                     std::string doc_base_path)
: headstation_ordernumber_m(std::move(headstation_ordernumber))
, headstation_firmware_version_m(std::move(headstation_firmware_version))
, settings_store_m(make_shared<settings_store>())
, combined_backend_m(make_shared<auth::oauth2_backend>(settings_store_m, broken_token_slowdown))
//, model_provider_m(make_unique<model_loader>(default_model_path)) // FIXME: Re-add model provider
//, device_description_providers_m()
, parameter_service_base_m(std::move(rest_api_base_path),
                           std::move(file_api_base_path),
                           std::move(doc_base_path),
                           identity,
                           settings_store_m,
                           std::make_unique<auth::permissions>(),
                           [](){
                               model_loader model_provider(default_model_path);
                               return model_provider.get_model_information().get().wdm_content;
                           },
                           [](std::string ordernumber, std::string firmware_version){
                               device_description_loader device_provider(default_repository_path, device_selection({ device_selector::headstation() }));
                               return device_provider.get_device_information(std::move(ordernumber), std::move(firmware_version)).get().content;
                           },
                           combined_backend_m,
                           combined_backend_m) // parasoft-suppress CERT_CPP-MEM56-a-1 "Bug in Parasoft? No unrelated smart pointers created at this point"
, rest_api_server_m(std::make_unique<server>(std::move(rest_api_socket),
                                             wdx::wda::trace_route::rest_api_call,
                                             parameter_service_base_m.get_rest_api_handler()))
, file_api_server_m(std::make_unique<server>(std::move(file_api_socket),
                                             wdx::wda::trace_route::file_api_call,
                                             parameter_service_base_m.get_file_api_handler()))
{
    //parameter_service_base_m.get_backend().register_model_providers({model_provider_m.get()});

    //vector<device_description_provider_i*> providers;

    //device_description_providers_m.push_back(make_unique<device_description_loader>(default_repository_path, device_selection({ device_selector::headstation() })));
    //providers.push_back(device_description_providers_m.back().get());
    //parameter_service_base_m.get_backend().register_device_description_providers(providers);

    register_headstation_device(parameter_service_base_m.get_backend(), headstation_ordernumber_m, headstation_firmware_version_m);

    try
    {
        auto additional_wdm_paths = sal::filesystem::get_instance().glob(additional_wdm_pattern);
        for (auto const &model_path : additional_wdm_paths)
        {
            parameter_service_base_m.load_additional_wdm(
                [model_path](){
                    model_loader model_provider(model_path);
                    return model_provider.get_model_information().get().wdm_content;
                }
            );
        }
    }
    catch(std::exception const &e)
    {
        wc_log_format(error, "error while loading additional WDM artifacts: %s", e.what());
    }

    try
    {
        auto additional_wdd_paths = sal::filesystem::get_instance().glob(additional_wdd_pattern);
        for (auto const &descr_path : additional_wdd_paths)
        {
            parameter_service_base_m.load_additional_wdd(
                [descr_path](std::string WC_UNUSED_PARAM(ordernumber), std::string WC_UNUSED_PARAM(firmware_version)){
                    device_extension_loader extension_provider(descr_path);
                    return extension_provider.get_device_information().get().content;
                }
            );
        }
    }
    catch(std::exception const &e)
    {
        wc_log_format(error, "error while loading additional WDD artifacts: %s", e.what());
    }
}

parameter_service::~parameter_service() noexcept = default;

server_i & parameter_service::get_rest_api_server()
{
    return *rest_api_server_m;
}

server_i & parameter_service::get_file_api_server()
{
    return *file_api_server_m;
}


wdx::wda::service_identity_i const & parameter_service::get_service_identity()
{
    return identity;
}

wdx::unauthorized<wdx::parameter_service_frontend_i> & parameter_service::get_unauthorized_frontend()
{
    return parameter_service_base_m.get_unauthorized_frontend();
}

wdx::unauthorized<wdx::parameter_service_file_api_i> & parameter_service::get_unauthorized_file_api()
{
    return parameter_service_base_m.get_unauthorized_file_api();
}

wdx::parameter_service_backend_extended_i & parameter_service::get_backend()
{
   return parameter_service_base_m.get_backend();
}

void parameter_service::cleanup(uint32_t timeout_ms)
{
    parameter_service_base_m.cleanup(timeout_ms);
}


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
