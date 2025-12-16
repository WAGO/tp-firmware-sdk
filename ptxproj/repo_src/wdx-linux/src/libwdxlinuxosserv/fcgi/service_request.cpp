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
///  \brief    Service FCGI request allowing text response.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "service_request.hpp"
#include "system_abstraction_serv.hpp"
#include "utils/url_encode.hpp"

#include <wago/wdx/wda/trace_routes.hpp>
#include <wc/assertion.h>
#include <wc/structuring.h>
#include <wc/log.h>

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <regex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace fcgi {

using wdx::wda::http::get_method_from_name;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

namespace {
void debug_log_request(FCGX_Request const * request);
void write_response_data(std::string const &str,
                         FCGX_Stream       *stream);
char const * empty_if_null(char const *str);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
service_request::service_request(dsocket_t         fcgi_socket,
                                 trace_route const route)
: route_m(route)
, request_m(std::unique_ptr<FCGX_Request>(new FCGX_Request))
, method_m(http_method::get) // Init with GET until we know more after accept
, content_m(request_m.get())
, content_touched_m(false)
, request_state_m(request_states::initial)
{
    int status = sal::fcgi::get_instance().InitRequest(request_m.get(), fcgi_socket, 0);
    if(0 != status)
    {
        throw std::runtime_error("Failed to initialize FCGI request");
    }
}

service_request::~service_request() noexcept
{
    if(request_state_m != request_states::finished)
    {
        finish_unchecked();
    }
    sal::fcgi::get_instance().Free(request_m.get(), 1);
}

bool service_request::accept()
{
    WC_ASSERT(request_state_m == request_states::initial);
    if(request_state_m == request_states::initial)
    {
        bool request_accepted = (sal::fcgi::get_instance().Accept_r(request_m.get()) == 0);
        if(request_accepted)
        {
            request_state_m = request_states::accepted;
        }
        remote_host_m      = empty_if_null(get_fcgi_param("REMOTE_ADDR")) + std::string(":") + empty_if_null(get_fcgi_param("REMOTE_PORT"));
        method_m           = get_method_from_name(get_fcgi_param("REQUEST_METHOD"));
        auto raw_uri       = get_fcgi_param("REQUEST_URI");
        wc_log_format(log_level_t::debug, "Accept request for URI '%s'", raw_uri);
        uri_m              = wdx::wda::http::uri(raw_uri);
        query_parameters_m = uri_m.parse_query();
        wc_log_format(log_level_t::debug, "Received request from '%s' with method '%s', path '%s' and query '%s': %s",
                      remote_host_m.c_str(), wdx::wda::http::get_method_name(method_m),
                      uri_m.get_path().c_str(), uri_m.get_query().c_str(), request_accepted ? "accepted" : "rejected");
        return request_accepted;
    }
    else
    {
        throw std::runtime_error("Request can only be accepted once.");
    }
}

bool service_request::is_https() const
{
    char const * const https = get_fcgi_param("HTTPS");
    return (https != nullptr) && (std::string(https) == "on");
}

bool service_request::is_localhost() const
{
    char const * const remote_host = get_fcgi_param("REMOTE_ADDR");
    // localhost: Every host part after "127.0.0." is a valid localhost address in IPv4
    return (remote_host != nullptr) && (    (std::string(remote_host).find("127.0.0.") == 0)
                                         || (std::string(remote_host) == "::1"));
}

http_method service_request::get_method() const
{
    return method_m;
}

wdx::wda::http::uri service_request::get_request_uri() const
{
    return uri_m;
}

bool service_request::has_query_parameter(std::string const & name) const
{
    return query_parameters_m.count(name) > 0;
}

std::string service_request::get_query_parameter(std::string const & name, bool decode) const
{
    return decode ? decode_url(query_parameters_m.at(name)) : query_parameters_m.at(name);
}

bool service_request::has_http_header(std::string const &header_name) const
{
    std::string upper_header_name(header_name);
    std::transform(upper_header_name.begin(), upper_header_name.end(),upper_header_name.begin(), [](unsigned char c) {
        return (c == '-') ? '_' : static_cast<unsigned char>(::toupper(c));
    });
    return nullptr != sal::fcgi::get_instance().GetParam(("HTTP_" + upper_header_name).c_str(), request_m->envp);
}

std::string service_request::get_http_header(std::string const &header_name) const
{
    std::string upper_header_name(header_name);
    std::transform(upper_header_name.begin(), upper_header_name.end(),upper_header_name.begin(), [](unsigned char c) {
        return (c == '-') ? '_' : static_cast<unsigned char>(::toupper(c));
    });
    return empty_if_null(sal::fcgi::get_instance().GetParam(("HTTP_" + upper_header_name).c_str(), request_m->envp));
}

std::string service_request::get_accepted_types() const
{
    return empty_if_null(get_fcgi_param("HTTP_ACCEPT"));
}

std::string service_request::get_content_type() const
{
    return empty_if_null(get_fcgi_param("CONTENT_TYPE"));
}

std::string service_request::get_content_length() const
{
    return empty_if_null(get_fcgi_param("CONTENT_LENGTH"));
}

std::string service_request::get_content()
{
    auto &content = get_content_stream();
    std::stringstream ss;
    ss << content.rdbuf();
    return ss.str();
}

std::istream &service_request::get_content_stream()
{
    WC_ASSERT(!content_touched_m);
    if(content_touched_m)
    {
        std::string message = "Cannot get content stream more than once.";
        throw std::runtime_error(message);
    }
    content_touched_m = true;
    if(!is_content_type_set())
    {
        wc_log(warning, "Tried to get body content from request but Content-Type header is not set");
    }
    return content_m;
}

void service_request::add_response_header(std::string const &header_name,
                                          std::string const &header_value)
{
    WC_ASSERT(request_state_m <= request_states::sending_header);
    if(request_state_m > request_states::sending_header)
    {
        throw std::runtime_error("Header already sent.");
    }
    else
    {
        response_header_m[header_name] = header_value;
    }
}

void service_request::respond(response_i const &response) noexcept
{
    WC_ASSERT(request_state_m == request_states::accepted);
    if(request_state_m != request_states::accepted)
    {
        wc_log(log_level_t::error, "Failed to respond FCGI request: Request in wrong state (not accepted).");
        return;
    }
    debug_log_request(request_m.get());

    wdx::wda::http::http_status_code const  http_status_code      = response.get_status_code();
    char                              const *http_status_code_text = wdx::wda::http::get_http_status_code_text(http_status_code);
    WC_ASSERT(http_status_code_text[0] != '\0');
    wc_log_format(log_level_t::debug, "Respond request from '%s' with method '%s', path '%s' and query '%s': HTTP status %u (%s)",
                  remote_host_m.c_str(), wdx::wda::http::get_method_name(method_m),
                  uri_m.get_path().c_str(), uri_m.get_query().c_str(), static_cast<unsigned>(http_status_code), http_status_code_text);

    try
    {
        WC_DEBUG_LOG("Write HTTP status");
        request_state_m = request_states::sending_status_code;
        write_response_data("Status: " + to_string(http_status_code) + " " + http_status_code_text + "\r\n", request_m->out);

        WC_DEBUG_LOG("Write header");
        request_state_m = request_states::sending_header;
        if(!response_header_m.empty())
        {
            // TODO: Use destructuring instead (available beginning with C++17)
            for(std::pair<std::string, std::string> const &header_pair : response_header_m)
            {
                write_response_data(header_pair.first + ": " + header_pair.second + "\r\n", request_m->out);
            }
        }
        parametermap const &response_header = response.get_response_header();
        if(!response_header.empty())
        {
            // TODO: Use destructuring instead (available beginning with C++17)
            for(auto const &header_pair : response_header)
            {
                write_response_data(header_pair.first + ": " + header_pair.second + "\r\n", request_m->out);
            }
        }
        WC_ASSERT(    (!response.has_content()
                           && ((response_header.find("Content-Length") == response_header.end()) || (response_header.find("Content-Length")->second == "0")))
                   || ( response.has_content()
                           && !response_header.find("Content-Type")->second.empty()
                           && !response_header.find("Content-Length")->second.empty()));
        write_response_data("\r\n", request_m->out);
        // can send body now

        request_state_m = request_states::sending_body;
        auto content = response.get_content();
        if(!content.empty())
        {
            WC_DEBUG_LOG("Write body (response content)");
            send_data(content.c_str(), content.size());
        }
    }
    catch(std::exception const &e)
    {
        std::string const error_message = std::string("Failed to respond FCGI request: ") + e.what();
        WC_FAIL(error_message.c_str());
        wc_log(log_level_t::error, error_message.c_str());

        // No way to rescue this request (what data was already sent?),
        // emergency finish to avoid further write operations
        finish_unchecked();
    }
}

bool service_request::is_responded() const
{
    // FIXME: rename is_status_sent ?
    return (request_state_m > request_states::accepted);
}

void service_request::send_data(char const *bytes, size_t size)
{
    WC_ASSERT(request_state_m == request_states::sending_body);

    WC_DEBUG_LOG("Write body (direct response data)");
    if(request_state_m == request_states::sending_body)
    {
        WC_ASSERT(size <= INT_MAX);
        if(sal::fcgi::get_instance().PutStr(bytes, static_cast<int>(size), request_m->out) < 0)
        {
            throw std::runtime_error("Failed to write FCGI response body data");
        }
    }
    else
    {
        throw std::runtime_error("Not in FCGI sending body state");
    }
}

void service_request::finish()
{
  WC_DEBUG_LOG("Finishing request");
  // Todo: initial < State  < finished
  WC_ASSERT(request_state_m == request_states::sending_body);
  if(request_state_m == request_states::sending_body)
  {
      finish_unchecked();
  }
  else if(request_state_m == request_states::finished)
  {
      throw std::runtime_error("Request already finished");
  }
  else
  {
      throw std::runtime_error("Not ready for finishing request");
  }
}

void service_request::finish_unchecked() noexcept
{
    request_state_m = request_states::finished;
    sal::fcgi::get_instance().Finish_r(request_m.get());
    WC_TRACE_SET_MARKER(for_route(route_m), "FCGI Request: handling done");
    wc_trace_stop_channel(for_route(route_m));
    wc_log_format(log_level_t::debug, "Finished request from '%s' with method '%s', path '%s' and query '%s'",
                  remote_host_m.c_str(), wago::wdx::wda::http::get_method_name(method_m),
                  uri_m.get_path().c_str(), uri_m.get_query().c_str());
}

char const * service_request::get_fcgi_param(char const * const param_name) const
{
    return sal::fcgi::get_instance().GetParam(param_name, request_m->envp);
}

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
void debug_log_request(FCGX_Request const *request)
{
#ifdef NDEBUG
    WC_UNUSED_PARAM(request);
#else
    if(wc_get_log_level() == log_level_t::debug)
    {
        // Log request parameters
        std::string params;
        char **p = request->envp;
        for(int i = 0; p[i] != nullptr; ++i)
        {
            params += std::to_string(i);
            params += ") ";
            params += p[i];
            wc_log(log_level_t::debug, params.c_str());
            params = "";
        }

        // Log request content
        if(sal::fcgi::get_instance().HasSeenEOF(request->in) == EOF)
        {
            wc_log(log_level_t::debug, "Content stream was already read from request");
        }
        else
        {
            char buffer[4096] = "";
            size_t const max_to_print  = 32 * 1024;
            size_t       left_to_print = max_to_print;
            while(sal::fcgi::get_instance().GetLine(buffer, 4096, request->in) != nullptr)
            {
                size_t const line_length = strlen(buffer);

                // Remove HTTP line endings
                if((line_length > 0) && (buffer[line_length - 1] == '\n'))
                {
                    buffer[line_length - 1] = '\0';
                    if((line_length > 1) && (buffer[line_length - 2] == '\r'))
                    {
                        buffer[line_length - 2] = '\0';
                    }
                }

                // Log content
                wc_log(log_level_t::debug, buffer);
                if(line_length >= left_to_print)
                {
                    wc_log(log_level_t::debug, "Cut of further request content");
                    break;
                }
                left_to_print -= line_length;
            }
            if(left_to_print == max_to_print)
            {
                wc_log(log_level_t::debug, "No content on request");
            }
        }
    }
#endif
}

void write_response_data(std::string const &str,
                         FCGX_Stream       *stream)
{
    if(sal::fcgi::get_instance().PutStr(str.c_str(), static_cast<int>(str.length()), stream) < 0)
    {
        throw std::runtime_error("Failed to write FCGI response data");
    }
}

char const * empty_if_null(char const *str)
{
    return str == nullptr ? "" : str;
}

}


} // Namespace fcgi
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
