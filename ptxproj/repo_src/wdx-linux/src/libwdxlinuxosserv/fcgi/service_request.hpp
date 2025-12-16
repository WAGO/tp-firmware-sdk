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
///  \brief    FCGI service request.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_FCGI_SERVICE_REQUEST_HPP_
#define SRC_LIBWDXLINUXOSSERV_FCGI_SERVICE_REQUEST_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "content_stream.hpp"

#include <wago/wdx/wda/http/request_i.hpp>
#include <wago/wdx/wda/trace_routes.hpp>
#include <wc/structuring.h>
#include <memory>

struct FCGX_Request;

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace fcgi {

using wdx::wda::http::request_i;
using wdx::wda::http::response_i;
using wdx::wda::http::http_method;
using wdx::wda::trace_route;

typedef int dsocket_t;
using parametermap = std::map<std::string, std::string>;

class service_request : public request_i
{
  private:
    trace_route                   const route_m;
    std::unique_ptr<FCGX_Request>       request_m;
    http_method                         method_m;
    std::string                         remote_host_m;
    wdx::wda::http::uri                 uri_m;
    parametermap                        query_parameters_m;
    content_stream                      content_m;
    bool                                content_touched_m;
    parametermap                        response_header_m;
    enum request_states
    {
      initial,
      accepted,
      sending_status_code,
      sending_header,
      sending_body,
      finished
    };
    request_states                      request_state_m;
  private:
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(service_request)

  public:
    explicit service_request(dsocket_t         fcgi_socket,
                             trace_route const route);
    ~service_request() noexcept override;

    bool accept();

    bool                 is_https() const override;
    bool                 is_localhost() const override;
    http_method          get_method() const override;
    wdx::wda::http::uri  get_request_uri() const override;
    bool                 has_query_parameter(std::string const & name) const override;
    std::string          get_query_parameter(std::string const & name, bool decode = true) const override;
    bool                 has_http_header(std::string const &header_name) const override;
    std::string          get_http_header(std::string const &header_name) const override;
    std::string          get_accepted_types() const override;
    std::string          get_content_type() const override;
    std::string          get_content_length() const override;
    std::string          get_content() override;
    std::istream        &get_content_stream() override;
    void                 add_response_header(std::string const &header_name,
                                             std::string const &header_value) override;
    void                 respond(response_i const &response) noexcept override;
    bool                 is_responded() const override;
    void                 send_data(char const *bytes, size_t size) override;
    void                 finish() override;

  private:
    char const * get_fcgi_param(char const * const param_name) const;
    void         finish_unchecked() noexcept;
};


} // Namespace fcgi
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_FCGI_SERVICE_REQUEST_HPP_
//---- End of source file ------------------------------------------------------
