//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of the token endpoint.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "token.hpp"
#include "definitions.hpp"
#include "authenticator_i.hpp"
#include "wago/authserv/oauth_token_handler_i.hpp"
#include "token_document.hpp"
#include "http/head_response.hpp"
#include "http/http_exception.hpp"
#include "oauth_error.hpp"
#include "log_authentication.hpp"
#include "utils/vector_operations.hpp"
#include "wago/authserv/settings_store_i.hpp"
#include "wago/authserv/http/request_i.hpp"

#include <wc/log.h>

// #include <algorithm>
// #include <sstream>
#include <stdexcept>

#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------

namespace {
void process_token_request_by_auth_code(settings_store_i      const &settings_store,
                                        http::request_i             &request,
                                        oauth_token_handler_i       &token_handler);
void process_token_request_by_refresh_token(settings_store_i      const &settings_store,
                                            http::request_i             &request,
                                            oauth_token_handler_i       &token_handler);
void process_token_request_by_password(settings_store_i      const &settings_store,
                                       http::request_i             &request,
                                       oauth_token_handler_i       &token_handler,
                                       authenticator_i             &auth_handler);
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void process_token_request(settings_store_i      const &settings_store,
                           http::request_i             &request,
                           oauth_token_handler_i       &token_handler,
                           authenticator_i             &auth_handler)
{
    try
    {
        if(!request.has_form_parameter(form_param_grant_type))
        {
            wc_log(log_level_t::warning, "Got token request without grant type parameter");
            throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"grant_type\" parameter");
        }

        // Switch handling by grant type
        std::string const grant_type = request.get_form_parameter(form_param_grant_type);
        if(grant_type == grant_type_code)
        {
            process_token_request_by_auth_code(settings_store, request, token_handler);
        }
        else if(grant_type == grant_type_refresh)
        {
            process_token_request_by_refresh_token(settings_store, request, token_handler);
        }
        else if(grant_type == grant_type_password)
        {
            process_token_request_by_password(settings_store, request, token_handler, auth_handler);
        }
        else
        {
            wc_log(log_level_t::warning, "Got token request with unsupported grant type\"" + grant_type + "\"");
            throw oauth_error(oauth_error::error_type::unsupported_grant_type);
        }
    }
    catch(oauth_error const &e)
    {
        request.respond(e.get_as_document());
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, "Caught unexpected exception on token request: " + std::string(e.what()));
        request.respond(http::head_response(http::http_status_code::internal_server_error));
    }
    catch(...)
    {
        wc_log(log_level_t::error, "Caught unexpected exception on token request");
        request.respond(http::head_response(http::http_status_code::internal_server_error));
    }
}

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {

void process_token_request_by_auth_code(settings_store_i      const &settings_store,
                                        http::request_i             &request,
                                        oauth_token_handler_i       &token_handler)
{
    WC_ASSERT(request.get_form_parameter(form_param_grant_type) == grant_type_code);

    // Check preconditions
    if(!request.has_form_parameter(form_param_code))
    {
        wc_log(log_level_t::warning, "Got token request without code");
        throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"code\" parameter");
    }
    if(!request.has_form_parameter(form_param_client_id))
    {
        wc_log(log_level_t::warning, "Got token request without client ID");
        throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"client_id\" parameter");
    }
    auto const client_id = request.get_form_parameter(form_param_client_id);
    if(!settings_store.client_exists(client_id))
    {
        std::string const log_client_id = client_id.length() > max_client_id_log_length ?
                                          client_id.substr(0, max_client_id_log_length) + "[...]" :
                                          client_id;
        wc_log(log_level_t::warning, "Got token request with an invalid client ID \"" + log_client_id + "\"");
        throw oauth_error(oauth_error::error_type::invalid_client, "Invalid \"client_id\" parameter");
    }

    if(!settings_store.get_client(client_id).supports_grant_type(grant_type_code))
    {
        wc_log(log_level_t::warning, "Client '" + client_id + "' is not authorized to use \"code\" grant type");
        throw oauth_error(oauth_error::error_type::unauthorized_client, "Client not authorized to use \"code\" grant type");
    }

    // Check auth code
    auto const result = token_handler.validate_auth_code(request.get_form_parameter(form_param_code),
                                                         request.get_form_parameter(form_param_code_verifier));
    if(!result.valid || result.expired)
    {
        wc_log(log_level_t::warning, "Got token request with invalid auth code" +
                                      std::string(result.expired ? " (expired)" : ""));
        throw oauth_error(oauth_error::error_type::invalid_grant, std::string(result.expired ? "Expired" : "Invalid") + " authorization code");
    }
    if(result.client_id != client_id)
    {
        wc_log(log_level_t::warning, "Got token request with wrong client ID");
        throw oauth_error(oauth_error::error_type::invalid_client, "Invalid \"client_id\" parameter");
    }

    // Generate and respond tokens
    auto const access_token_lifetime_s = static_cast<uint32_t>(std::stoul(settings_store.get_global_setting(settings_store_i::access_token_lifetime)));
    std::string const access_token     = token_handler.generate_access_token(access_token_lifetime_s, client_id, result.scopes, result.user_name);
    if(request.is_localhost())
    {
        auto const refresh_token_lifetime_s = static_cast<uint32_t>(std::stoul(settings_store.get_global_setting(settings_store_i::refresh_token_lifetime)));
        std::string const refresh_token     = token_handler.generate_refresh_token(refresh_token_lifetime_s, client_id, result.scopes, result.user_name);
        request.respond(token_document(access_token, refresh_token, access_token_lifetime_s));
    }
    else
    {
        // A refresh token should not be given into an application frontend (running e.g. in a webbrowser).
        // All known clients placing a backend running on the same device, so limitation to localhost improves security.
        // The token backend does not support client authentication, with limitation to localhost we avoid an external
        // point of attack for testing (refresh) tokens. (see process_token_request_by_refresh_token)
        request.respond(token_document(access_token, access_token_lifetime_s));
    }
}

void process_token_request_by_refresh_token(settings_store_i      const &settings_store,
                                            http::request_i             &request,
                                            oauth_token_handler_i       &token_handler)
{
    WC_ASSERT(request.get_form_parameter(form_param_grant_type) == grant_type_refresh);

    // ONLY localhost is allowed to retrieve token by a refresh token.
    // A refresh token should not be given into an application frontend (running e.g. in a webbrowser).
    // All known clients placing a backend running on the same device, so limitation to localhost improves security.
    // The token backend does not support client authentication, with limitation to localhost we avoid an external
    // point of attack for testing (refresh) tokens.
    if(!request.is_localhost())
    {
        wc_log(log_level_t::warning, "Got token request with refresh token from a non-localhost remote");
        throw oauth_error(oauth_error::error_type::invalid_grant, "Not allowed from non-localhost");
    }

    // Check preconditions
    if(!request.has_form_parameter(form_param_refresh_token))
    {
        wc_log(log_level_t::warning, "Got token request without refresh token");
        throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"refresh_token\" parameter");
    }

    // Check used refresh token
    auto const result = token_handler.validate_refresh_token(request.get_form_parameter(form_param_refresh_token));
    if(!result.valid || result.expired)
    {
        wc_log(log_level_t::warning, "Got token request with invalid refresh token" +
                                      std::string(result.expired ? " (expired)" : ""));
        throw oauth_error(oauth_error::error_type::invalid_grant, std::string(result.expired ? "Expired" : "Invalid") + " refresh token");
    }
    if(    request.has_form_parameter(form_param_scope)
        && (result.scopes != request.get_form_parameter(form_param_scope)))
    {
        log_authentication_warning("Given scopes in access token request does not match scopes the refresh token was issued for, "
                                   "invalid request for", result.user_name, result.client_id, request.get_form_parameter(form_param_scope), "refresh_token");
        throw oauth_error(oauth_error::error_type::invalid_scope, "Invalid \"scope\" parameter for provided refresh token");
    }

    // Generate and respond
    log_authentication_info("Successfully authenticated", result.user_name, result.client_id, result.scopes, "refresh_token");
    auto const access_token_lifetime_s = static_cast<uint32_t>(std::stoul(settings_store.get_global_setting(settings_store_i::access_token_lifetime)));
    std::string const access_token     = token_handler.generate_access_token(access_token_lifetime_s, result.client_id, result.scopes, result.user_name);
    request.respond(token_document(access_token, access_token_lifetime_s));
}

void process_token_request_by_password(settings_store_i      const &settings_store,
                                       http::request_i             &request,
                                       oauth_token_handler_i       &token_handler,
                                       authenticator_i             &auth_handler)
{
    WC_ASSERT(request.get_form_parameter(form_param_grant_type) == grant_type_password);

    // ONLY localhost is allowed to retrieve token by password grant.
    // This grant is more risky than the grant by auth code.
    // OAuth 2 discourage the use of this grant type (see https://www.rfc-editor.org/rfc/rfc6749#section-4.3).
    // OAuth 2.1 removes this grant type.
    // Taking this facts into security consideration, the password grant is limited to special clients from
    // this device (localhost) only.
    if(!request.is_localhost())
    {
        wc_log(log_level_t::warning, "Got token request with refresh token from a non-localhost remote");
        throw oauth_error(oauth_error::error_type::invalid_grant, "Not allowed from non-localhost");
    }

    // Check client
    if(!request.has_form_parameter(form_param_client_id))
    {
        wc_log(log_level_t::warning, "Got password grant token request without client id");
        throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"client_id\" parameter");
    }
    auto const client_id = request.get_form_parameter(form_param_client_id);
    if(!settings_store.client_exists(client_id))
    {
        std::string const log_client_id = client_id.length() > max_client_id_log_length ?
                                          client_id.substr(0, max_client_id_log_length) + "[...]" :
                                          client_id;
        wc_log(log_level_t::warning, "Got token request with an invalid client ID \"" + log_client_id + "\"");
        throw oauth_error(oauth_error::error_type::invalid_client, "Invalid \"client_id\" parameter");
    }
    if(!settings_store.get_client(client_id).supports_grant_type(grant_type_password))
    {
        wc_log(log_level_t::warning, "Client '" + client_id + "' is not authorized to use grant type \"password\"");
        throw oauth_error(oauth_error::error_type::unauthorized_client, "Client not authorized to use grant type \"password\"");
    }

    // Check scopes
    if(!request.has_form_parameter(form_param_scope) || request.get_form_parameter(form_param_scope).empty())
    {
        wc_log(log_level_t::warning, "Got password grant token request without scopes");
        throw oauth_error(oauth_error::error_type::invalid_scope, "Missing \"scope\" parameter");
    }
    auto const scopes_plain = request.get_form_parameter(form_param_scope);
    auto const scopes       = split_string(scopes_plain, ' ');
    if(!is_subset(scopes, settings_store.get_all_scopes()))
    {
        wc_log(log_level_t::warning, "Got password grant token request with at least one invalid scope");
        throw oauth_error(oauth_error::error_type::invalid_scope, "Invalid \"scope\" parameter");
    }

    // Check username and password
    if(!request.has_form_parameter(form_param_username))
    {
        wc_log(log_level_t::warning, "Got password grant token request without username");
        throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"username\" parameter");
    }
    if(!request.has_form_parameter(form_param_password))
    {
        wc_log(log_level_t::warning, "Got password grant token request without password");
        throw oauth_error(oauth_error::error_type::invalid_request, "Missing \"password\" parameter");
    }
    auto const auth_result = auth_handler.authenticate(request, scopes);
    if(auth_result.unauthorized)
    {
        log_authentication_warning("Requested scopes not authorized for", auth_result.user_name, client_id, scopes_plain, "password");
        throw oauth_error(oauth_error::error_type::invalid_scope, "Unauthorized for requested scopes");
    }
    // Handling for first use case "admin without password" is handled the same
    // way as an invalid password to avoid exposing any information about the
    // fact admin has no password at the moment.
    else if(!auth_result.success || auth_handler.admin_has_no_password())
    {
        // User name was not remembered in this case to avoid exposed passwords
        log_authentication_warning("User authentication not successful", "", client_id, scopes_plain, "password");
        throw oauth_error(oauth_error::error_type::invalid_grant, "Authentication failed");
    }
    else if(auth_result.expired)
    {
        // Log exactly as unsuccessful authentication to avoid exposing state of password to logs
        log_authentication_warning("User authentication not successful", "", client_id, scopes_plain, "password");
        throw oauth_error(oauth_error::error_type::invalid_grant, "Password expired");
    }

    // Generate tokens and respond
    log_authentication_info("Successfully authenticated", auth_result.user_name, client_id, scopes_plain, "password");

    auto const access_token_lifetime_s  = static_cast<uint32_t>(std::stoul(settings_store.get_global_setting(settings_store_i::access_token_lifetime)));
    auto const refresh_token_lifetime_s = static_cast<uint32_t>(std::stoul(settings_store.get_global_setting(settings_store_i::refresh_token_lifetime)));
    std::string const access_token      = token_handler.generate_access_token(access_token_lifetime_s, client_id, scopes_plain, auth_result.user_name);
    std::string const refresh_token     = token_handler.generate_refresh_token(refresh_token_lifetime_s, client_id, scopes_plain, auth_result.user_name);
    request.respond(token_document(access_token, refresh_token, access_token_lifetime_s, auth_result.expired));
}

} // Anonymous namespace

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
