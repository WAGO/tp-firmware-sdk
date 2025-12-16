//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
#include "wago/wdx/wda/parameter_service.hpp"
#include "identity.hpp"
#include "http/secure_handler.hpp"
#include "auth/authenticated_request_handler_i.hpp"
#include "auth/authenticator.hpp"
#include "rest/rest_frontend.hpp"
#include "files/files_frontend.hpp"
#include "parameter_service_core.hpp"
#include "model_loader.hpp"
#include "device_description_loader.hpp"
#include "settings_provider.hpp"

#include <wc/trace.h>
#include <wc/log.h>
#include <wc/assertion.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

class core_janitor final : public auth::authenticated_request_handler_i
{
private:
    std::shared_ptr<wdx::parameter_service_i>        core_m;
    std::shared_ptr<authenticated_request_handler_i> handler_m;

public:
    core_janitor(std::shared_ptr<wdx::parameter_service_i>        core,
                 std::shared_ptr<authenticated_request_handler_i> next_handler)
    : core_m(core)
    , handler_m(next_handler)
    { }
    ~core_janitor() noexcept override = default;

    void handle(std::unique_ptr<http::request_i> request, auth::authentication_info auth_info) noexcept override
    {
        // cleanup any outstanding file ids or monitoring list cleanups
        // before handling the next request.
        core_m->trigger_lapse_checks();
        handler_m->handle(std::move(request), std::move(auth_info));
    }
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

static std::unique_ptr<http::request_handler_i> create_rest_api_handler(std::shared_ptr<wdx::parameter_service_i>        core,
                                                                        service_identity_i                        const &service_identity,
                                                                        std::string                               const &base_path,
                                                                        std::string                               const &doc_base,
                                                                        std::shared_ptr<settings_store_i>                settings_store,
                                                                        std::shared_ptr<auth::password_backend_i>        password_backend,
                                                                        std::shared_ptr<auth::token_backend_i>           token_backend);

static std::unique_ptr<http::request_handler_i> create_file_api_handler(std::shared_ptr<wdx::parameter_service_i>        core,
                                                                        std::string                               const &base_path,
                                                                        std::shared_ptr<settings_store_i>                settings_store,
                                                                        std::shared_ptr<auth::password_backend_i>        password_backend,
                                                                        std::shared_ptr<auth::token_backend_i>           token_backend);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
parameter_service::parameter_service(std::string                               const &rest_api_base_path,
                                     std::string                               const &file_api_base_path,
                                     std::string                               const &doc_base_path,
                                     service_identity_i                        const &service_identity,
                                     std::shared_ptr<settings_store_i>                settings_store,
                                     std::unique_ptr<permissions_i>                   permissions,
                                     wdm_getter                                       get_wdm,
                                     wdd_getter                                       get_wdd,
                                     std::shared_ptr<auth::password_backend_i>        password_backend,
                                     std::shared_ptr<auth::token_backend_i>           token_backend)
: core_m(std::make_shared<wdx::parameter_service_core>(std::move(permissions)))
, unauthorized_frontend_m(core_m)
, unauthorized_file_api_m(core_m)
, service_identity_m(service_identity)
, rest_api_handler_m(create_rest_api_handler(core_m, service_identity_m, rest_api_base_path, doc_base_path, settings_store, password_backend, token_backend))
, file_api_handler_m(create_file_api_handler(core_m, file_api_base_path, settings_store, password_backend, token_backend))
, model_providers_m({std::make_shared<model_loader>(get_wdm)})
, device_description_providers_m({std::make_shared<device_description_loader>(device_selector::headstation(), get_wdd)})
#if PARAMETER_PROVIDER
, settings_provider_m(std::make_unique<settings_provider>(settings_store))
#endif
{
    core_m->register_model_provider(model_providers_m.at(0).get());
    core_m->register_device_description_provider(device_description_providers_m.at(0).get());
#if PARAMETER_PROVIDER
    core_m->register_parameter_provider(settings_provider_m.get()).get();
#endif
}

parameter_service::~parameter_service() noexcept = default;

void parameter_service::load_additional_wdm(wdm_getter get_wdm)
{
    auto model_provider = std::make_shared<model_loader>(get_wdm);
    core_m->register_model_provider(model_provider.get());
    model_providers_m.push_back(model_provider);
}

void parameter_service::load_additional_wdd(wdd_getter get_wdd)
{
    auto device_description_provider = std::make_shared<device_description_loader>(device_selector::headstation(), get_wdd);
    core_m->register_device_description_provider(device_description_provider.get());
    device_description_providers_m.push_back(device_description_provider);
}

http::request_handler_i & parameter_service::get_rest_api_handler()
{
    return *rest_api_handler_m;
}

http::request_handler_i & parameter_service::get_file_api_handler()
{
    return *file_api_handler_m;
}

wdx::unauthorized<wdx::parameter_service_frontend_i> & parameter_service::get_unauthorized_frontend()
{
    return unauthorized_frontend_m;
}

wdx::unauthorized<wdx::parameter_service_file_api_i> & parameter_service::get_unauthorized_file_api()
{
    return unauthorized_file_api_m;
}

wdx::parameter_service_backend_extended_i & parameter_service::get_backend()
{
    return *core_m;
}

void parameter_service::cleanup(uint32_t timeout_ms)
{
    try
    {
        auto pending_cleanup = core_m->cleanup();
        pending_cleanup.wait_for(std::chrono::milliseconds(timeout_ms));

        // check for readiness and log result (or log that there is no result)
        if(pending_cleanup.ready())
        {
            if (pending_cleanup.has_exception())
            {
                std::rethrow_exception(pending_cleanup.get_exception());
            }
            else
            {
                wc_log(info, "Core cleanup done successfully.");
            }
        }
        else
        {
            wc_log(error, "Core cleanup took too long. Didn't wait for success.");
        }
    }
    catch(std::exception const &e)
    {
        wc_log(error, std::string("Core cleanup failed: ") + e.what());
    }
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

static std::unique_ptr<http::request_handler_i> create_rest_api_handler(std::shared_ptr<wdx::parameter_service_i>        core,
                                                                        service_identity_i                        const &service_identity,
                                                                        std::string                               const &base_path,
                                                                        std::string                               const &doc_base,
                                                                        std::shared_ptr<settings_store_i>                settings_store,
                                                                        std::shared_ptr<auth::password_backend_i>        password_backend,
                                                                        std::shared_ptr<auth::token_backend_i>           token_backend)
{
    auto shared_rest_frontend = std::make_shared<rest::rest_frontend>(base_path,
                                                                      doc_base,
                                                                      service_identity,
                                                                      settings_store,
                                                                      unauthorized<parameter_service_frontend_extended_i>(core));
    return
#if !ALLOW_HTTP
        std::make_unique<http::secure_handler>(
#endif
#if !DISABLE_AUTH
            std::make_unique<auth::authenticator>(shared_rest_frontend,
                                                  password_backend,
                                                  token_backend,
                                                  base_path,
                                                  trace_route::rest_api_call,
#endif
                                                  std::make_unique<core_janitor>(
                                                      core,
                                                      shared_rest_frontend
                                                  )
#if !DISABLE_AUTH
            )
#endif
#if !ALLOW_HTTP
        )
#endif
  ;
}

static std::unique_ptr<http::request_handler_i> create_file_api_handler(std::shared_ptr<wdx::parameter_service_i>        core,
                                                                        std::string                               const &base_path,
                                                                        std::shared_ptr<settings_store_i>                settings_store,
                                                                        std::shared_ptr<auth::password_backend_i>        password_backend,
                                                                        std::shared_ptr<auth::token_backend_i>           token_backend)
{
    return
#if !ALLOW_HTTP
        std::make_unique<http::secure_handler>(
#endif
#if !DISABLE_AUTH
            std::make_unique<auth::authenticator>(nullptr,
                                                  password_backend,
                                                  token_backend,
                                                  base_path,
                                                  trace_route::file_api_call,
#endif
                                                  std::make_unique<core_janitor>(
                                                      core,
                                                      std::make_shared<files::files_frontend>(base_path,
                                                                                              settings_store,
                                                                                              unauthorized<parameter_service_file_api_i>(core))
                                                  )
#if !DISABLE_AUTH
            )
#endif
#if !ALLOW_HTTP
        )
#endif
    ;
}

} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
