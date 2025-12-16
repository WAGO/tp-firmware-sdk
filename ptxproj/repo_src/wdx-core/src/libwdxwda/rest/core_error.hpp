//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Error object to represent a core error which is deliverable via
///            REST-API.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_CORE_ERROR_HPP_
#define SRC_LIBWDXWDA_REST_CORE_ERROR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "status.hpp"
#include "wago/wdx/wda/http/http_status_code.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class core_error
{
private:
    core_status_code       core_status_m;
    uint16_t               domain_status_m;
    std::string            title_m;
    std::string            message_m;
    http::http_status_code http_status_m;
public:
    template <class WdaResponse>
    explicit core_error(WdaResponse const &response);
    template <class WdaResposne, class WdaRequest>
    core_error(WdaResposne const &response,
               WdaRequest  const &request);
    core_error(wdx::status_codes core_status,
               std::string       message,
               uint16_t          domain_status,
               std::string       resource_type,
               std::string       resource_id         = "",
               bool              is_request_error    = false,
               std::string       associated_type     = "",
               std::string       associated_id       = "");

    virtual ~core_error() noexcept = default;

    core_status_code       get_core_status_code()   const;
    uint16_t               get_domain_status_code() const;
    std::string            get_message()            const;
    std::string            get_title()              const;
    http::http_status_code get_http_status_code() const;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_CORE_ERROR_HPP_
//---- End of source file ------------------------------------------------------
