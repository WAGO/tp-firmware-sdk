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
///  \brief    Authenticator implementation for WAGO Parameter Service
///            on Linux controllers using PAM.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticator.hpp"
#include "auth_settings_i.hpp"
#include "wago/wdx/wda/http/request_i.hpp"
#include "wago/wdx/wda/trace_routes.hpp"
#include "wago/wdx/wda/exception.hpp"
#include "http/head_response.hpp"
#include "http/cors_handler.hpp"
#include "http/url_utils.hpp"
#include <wc/base64.h>

#include <wc/log.h>
#include <wc/trace.h>

#include <string>
#include <map>
#include <vector>
#include <regex>
#include <cstring>
#include <cctype>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace auth {

using std::string;
using std::to_string;
using std::map;
using std::vector;
using std::regex;
using std::regex_match;


static constexpr char   const authorization_header_name[]     = "Authorization";
static constexpr size_t const max_authorization_header_length = 4096;
static constexpr size_t const max_basic_authorization_length  = 512;
static constexpr char   const auth_methods_header_name[]      = "WAGO-WDX-Auth-Methods";
static constexpr char   const auth_method_password[]          = "Password";
static constexpr char   const auth_method_token[]             = "WDXToken";
static constexpr char   const auth_method_oauth2[]            = "OAuth2";
static constexpr char   const authenticate_header_name[]      = "WWW-Authenticate";
static constexpr char   const no_auth_popup_name[]            = "WAGO-WDX-No-Auth-Popup";
static constexpr char   const no_auth_popup_value_on[]        = "true";
static constexpr char   const no_auth_popup_value_off[]       = "false";
static constexpr char   const basic_auth[]                    = "Basic";
static constexpr size_t const basic_auth_length               = sizeof(basic_auth) - 1;
static constexpr char   const token_auth[]                    = "Bearer";
static constexpr size_t const token_auth_length               = sizeof(token_auth) - 1;
static constexpr char   const ac_expose_header_name[]         = "Access-Control-Expose-Headers";
static constexpr char   const ac_expose_header_value[]        = "WWW-Authenticate, WAGO-WDX-Auth-Methods";
static constexpr char   const token_type_header_name[]        = "WAGO-WDX-Auth-Token-Type";
static constexpr char   const token_header_name[]             = "WAGO-WDX-Auth-Token";
static constexpr char   const token_expiration_header_name[]  = "WAGO-WDX-Auth-Token-Expiration";
static constexpr char   const password_expired_header_name[]  = "WAGO-WDX-Auth-Password-Expired";
static constexpr char   const password_expired_header_value[] = "true";
//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static bool has_auth_information(std::string const &authorisation_header);
static bool is_basic_scheme(std::string const &authorisation_header);
static bool is_token_scheme(std::string const &authorisation_header);
static void extract_user_and_password(std::string const &authorisation_header,
                                      std::string       &user,
                                      std::string       &password);
static void extract_token(std::string const &authorisation_header,
                          std::string       &token);
static bool allow_unauthenticated(std::string      const &service_base_url,
                                  auth_settings_i  const *settings,
                                  request_i        const &request);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
authenticator::authenticator(shared_ptr<auth_settings_i>                        auth_settings,
                             shared_ptr<password_backend_i>                     basic_auth_backend,
                             shared_ptr<token_backend_i>                        token_auth_backend,
                             string                                      const &service_base,
                             trace_route                                 const  route,
                             shared_ptr<authenticated_request_handler_i>        next_handler)
: auth_settings_m(auth_settings)
, basic_auth_m(basic_auth_backend)
, token_auth_m(token_auth_backend)
, service_base_m(service_base)
, realm_m(service_base.substr(1))
, route_m(route)
, handler_m(next_handler)
{
    WC_ASSERT(basic_auth_backend != nullptr);
}

authenticator::~authenticator() noexcept = default;

void authenticator::handle(std::unique_ptr<request_i> request) noexcept
{
    try
    {
        auth_result authenticated;

        // Store some information for future use
        string   token;
        uint32_t remaining_time = 0;

        // Get authentication information from header fields
        auto const authorisation_header = request->get_http_header(authorization_header_name);
        if(!has_auth_information(authorisation_header))
        {
            wc_log(log_level_t::info, "Request for path '" + request->get_request_uri().get_path() + "' does not contain auth information");
        }
        else
        {
            WC_DEBUG_LOG(string("Found header \"") + authorization_header_name + "\" for authentication");

            // Limit header length for security reasons
            if(authorisation_header.length() > max_authorization_header_length)
            {
                string const fail_text = string("Header \"") + authorization_header_name + "\" to long, exceeds "
                                       + to_string(max_authorization_header_length) + " bytes";
                wc_log(log_level_t::warning, fail_text);
                throw exception(fail_text);
            }

            // Try to authenticate
            if(is_basic_scheme(authorisation_header))
            {
                // Limit basic authorization length for security reasons
                if(authorisation_header.length() > max_basic_authorization_length)
                {
                    string const fail_text = string("Basic Authorization header to long, exceeds ")
                                           + to_string(max_basic_authorization_length) + " bytes";
                    wc_log(log_level_t::warning, fail_text);
                    throw exception(fail_text);
                }

                WC_TRACE_SET_MARKER(for_route(route_m), "Authenticator: handle basic authentication");
                string user;
                string password;
                extract_user_and_password(authorisation_header, user, password);

                // Authenticate using password backend
                wc_log(log_level_t::info, "Tried authentication for user \"" + user + "\"");
                authenticated = basic_auth_m->authenticate(user, password);

                if(authenticated.success && authenticated.expired)
                {
                    request->add_response_header(password_expired_header_name, password_expired_header_value);
                }
            }
            else if(token_auth_m != nullptr && is_token_scheme(authorisation_header))
            {
                WC_TRACE_SET_MARKER(for_route(route_m), "Authenticator: handle token authentication");
                extract_token(authorisation_header, token);
                WC_ASSERT(!token.empty());

                // Authenticate using given token
                wc_log(log_level_t::info, "Tried authentication with token");
                authenticated = token_auth_m->authenticate(token, &remaining_time);
            }
            else
            {
                WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Authenticator: handle unknown authentication");
                // No known authentication request
                wc_log(log_level_t::warning, "Scheme for authentication unknown");
            }
            WC_TRACE_SET_MARKER(for_route(trace_route::rest_api_call), "Authenticator: handled authentication");
        }

        // Check authentication
        if(!authenticated.success)
        {
            if(    has_auth_information(authorisation_header)
                || !allow_unauthenticated(service_base_m, auth_settings_m.get(), *request))
            {
                wc_log(log_level_t::warning, "Necessary authentication failed");

                // Create client information about authentication possibilities
                map<string, string> response_header;
                response_header[auth_methods_header_name] = string(auth_method_password) + ", "
                                                          + string(auth_method_token)    + ", "
                                                          + string(auth_method_oauth2);

                // WWW-Authenticate Response Header (see RFC 7617)
                if (!request->has_http_header(no_auth_popup_name) || request->get_http_header(no_auth_popup_name) != no_auth_popup_value_on)
                {
                    response_header[authenticate_header_name] = string(basic_auth) + " realm=\"" + realm_m + "\", "
                                                              + string(token_auth) + " realm=\"" + realm_m + "\"";
                }
                else if (request->get_http_header(no_auth_popup_name) != no_auth_popup_value_off)
                {
                    wc_log(notice, "unknown value for header '" + std::string(no_auth_popup_name) + "': '"
                        + request->get_http_header(no_auth_popup_name) + "', using default value '" + no_auth_popup_value_off + "'");
                }

                // Do not spoiler possible methods/routes, therefore mirror request method as allowed method
                http::cors_handler::add_cors_response_headers(*request, { request->get_method() }, "", "");
                // Expose authenticate header configured above independently from CORS request and authentication methods header
                request->add_response_header(ac_expose_header_name, ac_expose_header_value);

                request->respond(http::head_response(http::http_status_code::unauthorized, response_header));
            }
            else
            {
                // Call next handler for allowed unauthenticated request
                wc_log(log_level_t::notice, "Let unauthenticated request pass for path '" + request->get_request_uri().get_path() + "'");
                handler_m->handle(std::move(request), { "unknown" });
            }
        }
        else
        {
            // If WDx token is available, add token to the response
            if(!authenticated.token.empty())
            {
                // Determine remaining time
                remaining_time = authenticated.token_expires_in;

                // Add token for next API use
                request->add_response_header(token_type_header_name,       token_auth);
                request->add_response_header(token_header_name,            authenticated.token);
                request->add_response_header(token_expiration_header_name, to_string(remaining_time));

                // Add headers to avoid caching of token answers for security reasons
                // This is taken from RFC 6749 (OAuth v2.0): https://www.rfc-editor.org/rfc/rfc6749#section-5.1
                request->add_response_header("Cache-Control", "no-store");
                request->add_response_header("Pragma",        "no-cache");
            }

            // Call next handler
            wc_log(log_level_t::info, "Let authenticated request pass for path '" + request->get_request_uri().get_path() + "'");
            handler_m->handle(std::move(request), { authenticated.user_name });
        }
    }
    catch(std::exception &e)
    {
        wc_log(log_level_t::error, std::string("Internal server error on authentication request handling: ") + e.what());
        // Do not spoiler possible methods/routes, therefore mirror request method as allowed method
        http::cors_handler::add_cors_response_headers(*request, { request->get_method() }, "", "");
        request->respond(http::head_response(http::http_status_code::internal_server_error));
    }
    catch(...)
    {
        wc_log(log_level_t::error, "Internal server error on authentication request handling (invalid exception caught)");
        // Do not spoiler possible methods/routes, therefore mirror request method as allowed method
        http::cors_handler::add_cors_response_headers(*request, { request->get_method() }, "", "");
        request->respond(http::head_response(http::http_status_code::internal_server_error));
    }

    return;
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
bool has_auth_information(std::string const &authorisation_header)
{
    return !authorisation_header.empty();
}

bool is_basic_scheme(std::string const &authorisation_header)
{
    return    (authorisation_header.find_first_of(basic_auth) != std::string::npos)
           && (authorisation_header.at(basic_auth_length) == ' ');
}

bool is_token_scheme(std::string const &authorisation_header)
{
    return    (authorisation_header.find_first_of(token_auth) != std::string::npos)
           && (authorisation_header.at(token_auth_length) == ' ');
}

void extract_user_and_password(std::string const &authorisation_header,
                               std::string       &user,
                               std::string       &password)
{
    // Extract username and password from Base64
    std::string encoded_combination = authorisation_header.substr(authorisation_header.find(basic_auth) + basic_auth_length);
    auto first_non_whitespace = encoded_combination.find_first_not_of(" \n\r\t");
    if (first_non_whitespace == std::string::npos)
    {
        constexpr char const fail_text[] = "Empty basic authentication";
        wc_log(log_level_t::warning, fail_text);
        throw exception(fail_text);
    }
    auto last_non_whitespace  = encoded_combination.find_last_not_of(" \n\r\t");
    encoded_combination = encoded_combination.substr(
        first_non_whitespace, (last_non_whitespace - first_non_whitespace) + 1
    );

    uint8_t decoded_combination[wc_base64_calc_decoded_length(encoded_combination.size())];
    size_t  decoded_length = 0;
    wc_base64_decode(encoded_combination.data(), encoded_combination.size(), 
                     decoded_combination, &decoded_length);
    std::string user_password_combination(reinterpret_cast<char const *>(decoded_combination), decoded_length);

    size_t divider_position = user_password_combination.find(':');
    if(divider_position != string::npos)
    {
        user     = user_password_combination.substr(0, divider_position);
        password = user_password_combination.substr(divider_position + 1);
    }
    else
    {
        constexpr char const fail_text[] = "Invalid basic authentication";
        wc_log(log_level_t::warning, fail_text);
        throw exception(fail_text);
    }
}

void extract_token(std::string const &authorisation_header,
                   string            &token)
{
    // Extract token
    std::string token_part = authorisation_header.substr(authorisation_header.find(token_auth) + token_auth_length);
    auto first_non_whitespace = token_part.find_first_not_of(" \n\r\t");
    if (first_non_whitespace == std::string::npos)
    {
        constexpr char const fail_text[] = "Empty token authentication";
        wc_log(log_level_t::warning, fail_text);
        throw exception(fail_text);
    }
    auto last_non_whitespace  = token_part.find_last_not_of(" \n\r\t");
    token_part = token_part.substr(
        first_non_whitespace, (last_non_whitespace - first_non_whitespace) + 1
    );
    token = token_part;
}

bool allow_unauthenticated(string           const &service_base_url,
                           auth_settings_i  const *settings,
                           request_i        const &request)
{
    bool allow = false;

    // OPTIONS requests must always pass to support CORS preflight requests
    if (request.get_method() == http::http_method::options)
    {
        return true;
    }

    // Specific URL patterns may be allowed unauthenticated when configured
    if(settings != nullptr)
    {
        string const urls = settings->get_unauthenticated_urls();

        vector<string> unauthenticated_urls;
        string const delimiter = ";";
        size_t begin = 0;
        size_t delimiter_pos;
        while((delimiter_pos = urls.find(delimiter, begin + 1)) != string::npos) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional Assignment. Parasoft has a bug here."
        {
            unauthenticated_urls.push_back(urls.substr(begin, delimiter_pos - begin));
            begin = delimiter_pos + 1;
        }
        if(!urls.empty())
        {
            unauthenticated_urls.push_back(urls.substr(begin, urls.length() - begin));
        }

        string request_uri = request.get_request_uri().as_string();
        size_t start_of_service_uri = service_base_url.length();
        bool check_uri_length = request_uri.length() > start_of_service_uri;

        WC_ASSERT(check_uri_length);
        if(check_uri_length)
        {
            string service_uri = &request_uri[start_of_service_uri];
            auto query_seperator = service_uri.find('?');
            if(query_seperator != string::npos)
            {
                service_uri = service_uri.substr(0, query_seperator);
            }
            for(auto url_template : unauthenticated_urls)
            {
                if(regex_match(service_uri, regex(http::build_url_regex_pattern(url_template, true, false))))
                {
                    allow = true;
                    break;
                }
            }
        }
    }
    return allow;
}


} // Namespace auth
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
