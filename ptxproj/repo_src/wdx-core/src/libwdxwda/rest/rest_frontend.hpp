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
///  \brief    REST-API frontend for WAGO Parameter Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_REST_FRONTEND_HPP_
#define SRC_LIBWDXWDA_REST_REST_FRONTEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "router.hpp"
#include "auth_settings_i.hpp"
#include "run_object_manager.hpp"
#include "auth/authenticated_request_handler_i.hpp"
#include "wago/wdx/unauthorized.hpp"

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
class service_identity_i;
class settings_store_i;

namespace rest {

using http::request_i;

class rest_frontend : public auth::authenticated_request_handler_i
                    , public auth_settings_i
{
private:
    service_identity_i                                       const &service_identity_m;
    shared_ptr<settings_store_i>                             const  settings_store_m;
    // needed to hold smart pointer
    unauthorized<wdx::parameter_service_frontend_extended_i> const  core_frontend_m;
    router                                                          router_m;
    shared_ptr<run_object_manager>                                  run_manager_m;

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(rest_frontend)
    uint16_t read_run_result_timeout();

public:
    rest_frontend(string                                                   const &service_base,
                  string                                                   const &doc_link_base,
                  service_identity_i                                       const &service_identity,
                  shared_ptr<settings_store_i>                             const  settings_store,
                  unauthorized<wdx::parameter_service_frontend_extended_i> const  core_frontend);
    ~rest_frontend() noexcept override;

    // Interface request_handler_i
    void handle(std::unique_ptr<request_i> request, auth::authentication_info auth_info) noexcept override;

    // Interface auth_settings_i
    std::string get_unauthenticated_urls() const override;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_REST_FRONTEND_HPP_
//---- End of source file ------------------------------------------------------
