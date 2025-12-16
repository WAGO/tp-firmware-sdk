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
///  \brief    Implementation of REST operation object wrapping FCGI request.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "operation.hpp"
#include "operations/utils.hpp"
#include "wago/wdx/wda/service_identity_i.hpp"
#include "wago/wdx/wda/http/request_i.hpp"
#include "wago/wdx/wda/http/response_i.hpp"
#include "wago/wdx/wda/http/http_method.hpp"
#include "core_exception.hpp"
#include "data_exception.hpp"
#include "basic_resource.hpp"
#include "method_invocation.hpp"
#include "http/cors_handler.hpp"

#include <wc/assertion.h>
#include <wc/log.h>
#include <wc/structuring.h>

#include <string>
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace {

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------
void handle_exception(std::shared_ptr<request> req,
                      std::exception_ptr       except);
} // Anonymous namespace for internal functions

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
operation::operation(service_identity_i               const  &identity,
                     std::unique_ptr<frontend_i>              core_frontend,
                     settings                               &&operation_settings,
                     shared_ptr<run_object_manager_i>         run_manager)
: service_identity_m(identity)
, service_frontend_m(std::move(core_frontend))
, operation_settings_m(operation_settings)
, run_manager_m(run_manager)
{
    run_manager_m->clean_runs();
}

service_identity_i const &operation::get_service_identity() const
{
    return service_identity_m;
}
frontend_i &operation::get_service_frontend() const
{
    return *service_frontend_m;
}

std::vector<http_method> const &operation::get_allowed_methods() const
{
    return operation_settings_m.allowed_methods;
}

uint16_t operation::get_run_result_timeout_span() const
{
    return operation_settings_m.run_result_timeout_span;
}

shared_ptr<run_object_manager_i> operation::get_run_manager()
{
    return run_manager_m;
}

void operation::handle(operation_handler_t const  &handler, 
                       request                   &&req) noexcept
{
    http::cors_handler::add_cors_response_headers(req, get_allowed_methods(), cors_allowed_headers, cors_exposed_headers);
    auto shared_operation = std::make_shared<operation>(*this);
    auto shared_request   = std::make_shared<request>(std::move(req));
    try {
        auto handler_response = handler(shared_operation.get(), shared_request);
        handler_response.set_notifier([shared_operation, shared_request](std::unique_ptr<response_i> &&resp) {
            shared_request->respond(*resp);
            shared_request->finish();
            if(shared_operation->deferred_handler_m != nullptr)
            {
                shared_operation->deferred_handler_m();
            }
        });
        handler_response.set_exception_notifier([shared_operation, shared_request](std::exception_ptr e) {
            handle_exception(shared_request, e);
            shared_request->finish();
            if(shared_operation->deferred_handler_m != nullptr)
            {
                shared_operation->deferred_handler_m();
            }
        });
    }
    catch(...)
    {
        WC_DEBUG_LOG("An unexpected internal server error on json frontend request handling (exception caught)");
        handle_exception(shared_request, std::current_exception());
    }
}

void operation::set_deferred_handler(std::function<void()> handler)
{
    deferred_handler_m = handler;
}

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {

void handle_exception(std::shared_ptr<request> req,
                      std::exception_ptr       except)
{
    try
    {
        try
        {
            std::rethrow_exception(std::move(except));
        }
        catch(http_exception const &e)
        {
            throw;
        }
        catch(wdx::exception const &e)
        {
            auto const wda_error_number = static_cast<unsigned>(e.get_error_code());
            WC_STATIC_ASSERT(sizeof(wda_error_number) >= sizeof(e.get_error_code()));
            string const error_text = string(wda_core_exception_text) + " " + std::to_string(wda_error_number);
            throw http_exception(error_text);
        }
        catch(std::exception const &e)
        {
            WC_DEBUG_LOG("Internal operation execution error occurred: " + std::string(e.what()));
            throw http_exception("Internal operation execution error occurred.");
        }
        catch(...)
        {
            throw http_exception("Internal operation execution error occurred.");
        }
    }
    catch(http_exception const &e)
    {
        shared_ptr<http_exception> error;
        auto const *data_e = dynamic_cast<data_exception const *>(&e); // parasoft-suppress CERT_C-EXP39-b-3 "data_exception is a derived type of a http_exception"
        if(data_e != nullptr)
        {
            error = std::make_shared<data_exception>(*data_e);
        }
        else
        {
            auto const *core_e = dynamic_cast<core_exception const *>(&e); // parasoft-suppress CERT_C-EXP39-b-3 "core_exception is a derived type of a http_exception"
            if(core_e != nullptr)
            {
                error = std::make_shared<core_exception>(*core_e);
            }
            else
            {
                error = std::make_shared<http_exception>(e);
            }
        }
        auto response = get_error_response(req, error);
        req->respond(*response);
    }
}

} // Anonymous namespace for internal functions

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
