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
///  \brief    REST-API router to route different API requests by URI path.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_ROUTER_HPP_
#define SRC_LIBWDXWDA_REST_ROUTER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/request_i.hpp"
#include "wago/wdx/wda/http/http_method.hpp"
#include "json_api.hpp"
#include "operation.hpp"

#include "parameter_service_frontend_extended_i.hpp"

#include <string>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::string;
using std::vector;
using http::request_i;
using http::http_method;

using frontend_i = wdx::parameter_service_frontend_extended_i;

class router {
public:
struct route_result;

private:
    struct route_entry;

    service_identity_i   const &service_identity_m;
    string               const  service_base_url_m;
    vector<route_entry>         routes_m;
    json_api                    json_api_serializer_m;

public:
    router(service_identity_i     const &service_identity,
           string                 const &service_base_url);
    ~router() noexcept;

    void add_route(http_method         const  method,
                   string              const &url_template,
                   operation_handler_t const &handler,
                   string              const &doc_link = "");
    void add_redirect(string              const &url_template,
                      operation_handler_t const &handler);
    route_result route(std::unique_ptr<request_i> request) noexcept;

private:
    std::vector<http_method> find_route(http_method                method,
                                        operation_handler_t       &handler,
                                        string              const &request_path,
                                        map<string, string>       &path_parameters,
                                        string                    &doc_link) const;
    bool validate_url_template(string const &url_template);
    void add_route(http_method         const  method,
                   string              const &url_template,
                   operation_handler_t const &handler,
                   string              const &doc_link,
                   bool                const  allow_trailing_slash,
                   bool                const  match_following);

private:
    serializer_i   const & select_serializer(request_i const &request) const;
    deserializer_i const & select_deserializer(request_i const &request) const;
};

struct router::route_result
{
    rest::request                  request_m;
    operation_handler_t      const handler_m;
    std::vector<http_method> const allowed_methods_m;

    route_result(rest::request                  request,
                 operation_handler_t      const handler,
                 std::vector<http_method> const allowed_methods);
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


#endif // SRC_LIBWDXWDA_REST_ROUTER_HPP_
//---- End of source file ------------------------------------------------------
