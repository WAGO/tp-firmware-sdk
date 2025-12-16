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
///  \brief    File-API frontend for WAGO Parameter Service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_FILES_FILES_FRONTEND_HPP_
#define SRC_LIBWDXWDA_FILES_FILES_FRONTEND_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "auth/authenticated_request_handler_i.hpp"
#include "files_settings_i.hpp"
#include "wago/wdx/file_transfer/parameter_service_file_api_i.hpp"
#include "wago/wdx/unauthorized.hpp"

#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

class settings_store_i;

namespace http {

class request_i;

} // Namespace http

namespace files {

using http::request_i;
using std::shared_ptr;

class files_frontend : public auth::authenticated_request_handler_i
                     , public files_settings_i
{
private:
    std::string                                     const service_base_m;
    std::shared_ptr<settings_store_i>               const settings_store_m;
    unauthorized<wdx::parameter_service_file_api_i>       frontend_m;

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(files_frontend)

public:
    files_frontend(std::string                                     const &service_base,
                   shared_ptr<settings_store_i>                    const  settings_store,
                   unauthorized<wdx::parameter_service_file_api_i> const  frontend);
    ~files_frontend() noexcept override;

    void handle(std::unique_ptr<request_i> request, auth::authentication_info auth_info) noexcept override;

    uint16_t get_upload_id_timeout() const override;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace files
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_FILES_FILES_FRONTEND_HPP_
//---- End of source file ------------------------------------------------------
