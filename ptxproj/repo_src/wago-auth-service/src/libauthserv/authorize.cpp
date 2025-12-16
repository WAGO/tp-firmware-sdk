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
///  \brief    WEB-API frontend for WAGO Auth Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authorize.hpp"
#include "definitions.hpp"
#include "authenticator_i.hpp"
#include "response_helper.hpp"
#include "http/head_response.hpp"
#include "http/http_exception.hpp"
#include "oauth_error.hpp"
#include "system_abstraction.hpp"
#include "password_change_backend_i.hpp"
#include "log_authentication.hpp"
#include "utils/vector_operations.hpp"
#include "wago/authserv/http/http_status_code.hpp"
#include "wago/authserv/http/request_i.hpp"
#include "wago/authserv/oauth_token_handler_i.hpp"
#include "wago/authserv/settings_store_i.hpp"

#include <wc/log.h>

#include <stdexcept>

#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
constexpr char   const password_expired_message[] = "Your password is expired. A password change is required.";

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------
namespace {

struct oauth_basic_info {
    std::string cancel_uri;
    std::string client_id;
    std::string client_label;
};

struct oauth_return_info {
    std::string              scopes_plain;
    std::vector<std::string> scopes;
    std::string              code_challenge;
    std::string              redirect_location;
    std::string              state_query;
};

void authenticate_user(http::request_i             &request,
                       response_helper_i     const &response_helper,
                       oauth_basic_info      const &basic_info,
                       settings_store_i      const &settings_store,
                       oauth_token_handler_i       &token_handler,
                       authenticator_i             &auth_handler,
                       oauth_return_info     const &return_info);

void handle_authentication_errors(http::request_i &request, oauth_return_info const &return_info, std::exception_ptr const &e_ptr);

void change_user_password(std::shared_ptr<http::request_i>              request,
                          std::shared_ptr<response_helper_i>     const  response_helper,
                          oauth_basic_info                       const &basic_info,
                          std::shared_ptr<settings_store_i>             settings_store,
                          std::shared_ptr<oauth_token_handler_i>        token_handler,
                          authenticator_i                              &auth_handler,
                          oauth_return_info                      const &return_info,
                          bool                                          is_setup = false);

void redirect_successful_authentication(http::request_i             &request,
                                        oauth_basic_info      const &basic_info,
                                        settings_store_i      const &settings_store,
                                        oauth_token_handler_i       &token_handler,
                                        oauth_return_info     const &return_info,
                                        auth_result           const &auth_result);
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void process_authorize_request(std::shared_ptr<response_helper_i>     const  response_helper,
                               std::shared_ptr<settings_store_i>      const  settings_store,
                               std::shared_ptr<http::request_i>              request,
                               std::shared_ptr<oauth_token_handler_i>        token_handler,
                               authenticator_i                              &auth_handler)
{
    // structure collected information
    oauth_basic_info basic_info = {};
    oauth_return_info return_info = {};

    // Extract client ID & state from request and determine redirect location
    if(!request->has_query_parameter(query_param_client_id))
    {
        // FIXME: Send back error information: https://www.rfc-editor.org/rfc/rfc6749#section-4.1.2.1
        // Idea: Send an HTML page with error information (will be displayed instead of login page)
        wc_log(log_level_t::warning, "Got authorize request without client ID");
        throw http::http_exception("Reject authorize request with missing client ID.",
                                   http::http_status_code::bad_request);
    }
    basic_info.client_id = request->get_query_parameter(query_param_client_id);
    if(!settings_store->client_exists(basic_info.client_id))
    {
        throw http::http_exception("Reject authorize request by unknown client ID.",
                                   http::http_status_code::bad_request);
    }
    auto const &client = settings_store->get_client(basic_info.client_id);
    basic_info.client_label = client.label;

    // HTTPS (TLS) should be required for redirect locations:
    // https://www.rfc-editor.org/rfc/rfc6749#section-3.1.2.1
    // Allow HTTP exceptional only for localhost
    // Get correct location for client from config.
    return_info.redirect_location = client.redirect_uri;

    if(return_info.redirect_location.empty())
    {
        throw http::http_exception("Corrupt settings: Missing redirect URI for client.",
                                   http::http_status_code::internal_server_error);
    }
    return_info.state_query = request->has_query_parameter(query_param_state) ?
                                    "&state=" + request->get_query_parameter(query_param_state, false) :
                                    "";
    try
    {
        // Check request
        if(!settings_store->get_client(basic_info.client_id).supports_grant_type(grant_type_code))
        {
            wc_log(log_level_t::warning, "Reject authorize request by client '" + basic_info.client_id +
                                         "' not supporting authorization code flow.");
            throw oauth_error(oauth_error::error_type::unauthorized_client);
        }
        if(!request->has_query_parameter(query_param_response_type))
        {
            wc_log(log_level_t::warning, "Got authorize request without response type parameter");
            throw oauth_error(oauth_error::error_type::unsupported_response_type, "No response_type parameter");
        }
        std::string const response_type = request->get_query_parameter(query_param_response_type);
        if(response_type != response_type_code)
        {
            wc_log(log_level_t::warning, "Got authorize request with unsupported response type"
                                         " \"" + std::string(response_type) + "\"");
            throw oauth_error(oauth_error::error_type::unsupported_response_type);
        }
        if(!request->has_query_parameter(query_param_code_challenge_method))
        {
            wc_log(log_level_t::warning, "Got authorize request without \"code_challenge_method\" parameter");
            throw oauth_error(oauth_error::error_type::invalid_request, "No \"code_challenge_method\" parameter");
        }
        if(request->get_query_parameter(query_param_code_challenge_method) != code_challenge_method_s256)
        {
            wc_log(log_level_t::warning, "Got authorize request with invalid \"code_challenge_method\" parameter");
            throw oauth_error(oauth_error::error_type::invalid_request, "Invalid \"code_challenge_method\" parameter");
        }
        if(!request->has_query_parameter(query_param_code_challenge))
        {
            wc_log(log_level_t::warning, "Got authorize request without \"code_challenge\" parameter");
            throw oauth_error(oauth_error::error_type::invalid_request, "No \"code_challenge\" parameter");
        }
        if(!request->has_query_parameter(query_param_scope) || request->get_query_parameter(query_param_scope).empty())
        {
            wc_log(log_level_t::warning, "Got authorize request without \"scope\" parameter");
            throw oauth_error(oauth_error::error_type::invalid_scope, "No \"scope\" parameter");
        }
        return_info.scopes_plain = request->get_query_parameter(query_param_scope);
        return_info.scopes       = split_string(return_info.scopes_plain, ' ');
        if(!is_subset(return_info.scopes, settings_store->get_all_scopes()))
        {
            wc_log(log_level_t::warning, "Got authorize request with invalid scopes");
            throw oauth_error(oauth_error::error_type::invalid_scope, "Found at least one invalid scope");
        }

        // Process request information
        return_info.code_challenge = request->get_query_parameter(query_param_code_challenge);
        basic_info.cancel_uri      = return_info.redirect_location + "?" "error=access_denied"
                                           "&" "error_description=Cancel%20requested" + return_info.state_query;

        if (auth_handler.admin_has_no_password())
        {
            if (request->get_method() == http::http_method::get)
            {
                response_helper->send_password_setup_page(*request, basic_info.cancel_uri, http::http_status_code::ok);
            }
            else if (request->get_method() == http::http_method::post)
            {
                if (auth_handler.has_form_password_change_data(*request))
                {
                    //User will be redirected to the client after successfully changing their password
                    change_user_password(request, response_helper, basic_info,
                                 settings_store, token_handler, auth_handler, return_info, true);
                }
                else
                {
                    wc_log(log_level_t::warning, "Got post request without needed form data.");
                    throw oauth_error(oauth_error::error_type::server_error, "Invalid form data");
                }
            }
            else
            {
                request->respond(http::head_response(http::http_status_code::method_not_allowed));
            }
        }
        else if (request->get_method() == http::http_method::get)
        {
            response_helper->send_login_page(*request, basic_info.cancel_uri, http::http_status_code::ok, basic_info.client_label);
            //TODO send password change page when requested (maybe before checking for OAuth params)
        }
        else if(request->get_method() == http::http_method::post)
        {
            if (auth_handler.has_form_password_change_data(*request))
            {
                change_user_password(request, response_helper, basic_info,
                                     settings_store, token_handler, auth_handler, return_info);
            }
            else if (auth_handler.has_form_auth_data(*request))
            {
                authenticate_user(*request, *response_helper, basic_info,
                                  *settings_store, *token_handler, auth_handler, return_info);
            }
            else
            {
                wc_log(log_level_t::warning, "Got post request without form data.");
                throw oauth_error(oauth_error::error_type::server_error, "Invalid form data");
            }
        }
        else
        {
            request->respond(http::head_response(http::http_status_code::method_not_allowed));
        }
    }
    catch(...)
    {
        handle_authentication_errors(*request, return_info, std::current_exception());
    }
}


//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------

namespace {

void authenticate_user(http::request_i                &request,
                       response_helper_i        const &response_helper,
                       oauth_basic_info         const &basic_info,
                       settings_store_i         const &settings_store,
                       oauth_token_handler_i          &token_handler,
                       authenticator_i                &auth_handler,
                       oauth_return_info        const &return_info) {

    auto auth_result = auth_handler.authenticate(request, return_info.scopes);
    if(auth_result.expired)
    {
        // Expired passwords (currently) logged/reported by PAM
        // We do not expose something related to passwords at this point

        // force change of expired password
        response_helper.send_password_change_page(request, basic_info.cancel_uri, http::http_status_code::ok, auth_result.user_name, response_helper_i::no_error, "", password_expired_message);
    }
    else if(auth_result.unauthorized)
    {
        log_authentication_warning("Requested scopes not authorized for", auth_result.user_name, basic_info.client_id, return_info.scopes_plain, "authorization_code");
        throw oauth_error(oauth_error::error_type::access_denied);
    }
    else if(!auth_result.success)
    {
        // User name was not remembered in this case to avoid exposed passwords
        log_authentication_warning("User authentication not successful", "", basic_info.client_id, return_info.scopes_plain, "authorization_code");
        response_helper.send_login_page(request, basic_info.cancel_uri, http::http_status_code::bad_request, basic_info.client_label, response_helper_i::invalid_username_or_password);
    }
    else
    {
        log_authentication_info("Successfully authenticated", auth_result.user_name, basic_info.client_id, return_info.scopes_plain, "authorization_code");
        redirect_successful_authentication(request, basic_info, settings_store, token_handler, return_info, auth_result);
    }
}

void handle_authentication_errors(http::request_i &request, oauth_return_info const &return_info, std::exception_ptr const &e_ptr) {
    try
    {
        try
        {
            std::rethrow_exception(e_ptr);
        }
        catch(http::http_exception &e)
        {
            throw oauth_error(oauth_error::error_type::server_error, e.what());
        }
    }
    // Redirect error information: https://www.rfc-editor.org/rfc/rfc6749#section-4.1.2.1
    catch(oauth_error const &e)
    {
        request.add_response_header("Location", return_info.redirect_location + "?" + e.get_as_query() + return_info.state_query);
        request.respond(http::head_response(http::http_status_code::found));
    }
    catch(std::exception const &e)
    {
        wc_log(log_level_t::error, "Caught unexpected exception on authorize request: " + std::string(e.what()));
        request.add_response_header("Location", return_info.redirect_location + "?error=server_error" + return_info.state_query);
        request.respond(http::head_response(http::http_status_code::found));
    }
    catch(...)
    {
        wc_log(log_level_t::error, "Caught unexpected exception on authorize request.");
        request.add_response_header("Location", return_info.redirect_location + "?error=server_error" + return_info.state_query);
        request.respond(http::head_response(http::http_status_code::found));
    }
}

void change_user_password(std::shared_ptr<http::request_i>              request,
                          std::shared_ptr<response_helper_i>     const  response_helper,
                          oauth_basic_info                       const &basic_info,
                          std::shared_ptr<settings_store_i>             settings_store,
                          std::shared_ptr<oauth_token_handler_i>        token_handler,
                          authenticator_i                              &auth_handler,
                          oauth_return_info                      const &return_info,
                          bool                                          is_setup) {
    auto auth_result = auth_handler.authenticate(*request, return_info.scopes);
    if(!auth_result.success)
    {
        if (is_setup)
        {
            response_helper->send_password_setup_page(*request, basic_info.cancel_uri, http::http_status_code::bad_request, response_helper_i::internal_error);
        }
        else
        {
            response_helper->send_password_change_page(*request, basic_info.cancel_uri, http::http_status_code::bad_request, auth_result.user_name, response_helper_i::invalid_username_or_password);
        }
    }
    else
    {
        try
        {
            // change password
            wago::future<void> pw_change_success = auth_handler.password_change(*request);

            pw_change_success.set_notifier([response_helper, settings_store, token_handler, basic_info, return_info, auth_result, request]()
            {
                wc_log(log_level_t::info, "Successfully updated password for user \"" + auth_result.user_name + "\"");
                response_helper->send_login_page(*request, basic_info.cancel_uri, http::http_status_code::ok, basic_info.client_label, response_helper_i::no_error, "", "Successfully updated password.");
            });
            pw_change_success.set_exception_notifier([auth_result, request, response_helper, basic_info, return_info, is_setup](std::exception_ptr &&e_ptr)
            {
                try
                {
                    std::rethrow_exception(e_ptr);
                }
                catch (password_change_error const &e)
                {
                    if (is_setup)
                    {
                        response_helper->send_password_setup_page(*request, basic_info.cancel_uri, http::http_status_code::bad_request, response_helper_i::internal_error, e.what());
                    }
                    else
                    {
                        std::string &&info_msg = auth_result.expired ? password_expired_message : ""; // add expired warning if applicable
                        response_helper->send_password_change_page(*request, basic_info.cancel_uri, http::http_status_code::bad_request, auth_result.user_name, response_helper_i::internal_error, e.what(), info_msg);
                    }
                }
                catch (...)
                {
                    std::string error_msg = is_setup
                        ? "Got unexpected error while setting password"
                        : "Got unexpected error while changing password";
                    wc_log(log_level_t::error, error_msg);
                    handle_authentication_errors(*request, return_info, e_ptr);
                }
            });
        }
        catch (password_change_error const &e)
        {
            std::string &&info_msg = auth_result.expired ? password_expired_message : ""; // add expired warning if applicable
            response_helper->send_password_change_page(*request, basic_info.cancel_uri, http::http_status_code::bad_request, auth_result.user_name, response_helper_i::internal_error, e.what(), info_msg);
        }
    }
}

void redirect_successful_authentication(http::request_i             &request,
                                        oauth_basic_info      const &basic_info,
                                        settings_store_i      const &settings_store,
                                        oauth_token_handler_i       &token_handler,
                                        oauth_return_info     const &return_info,
                                        auth_result           const &auth_result)
{
    // Redirect with auth code
    auto        const auth_code_lifetime_s = static_cast<uint32_t>(std::stoul(settings_store.get_global_setting(settings_store_i::auth_code_lifetime)));
    std::string const authorization_code   = token_handler.generate_auth_code(auth_code_lifetime_s, basic_info.client_id, return_info.scopes_plain, auth_result.user_name, return_info.code_challenge);
    request.add_response_header("Location", return_info.redirect_location + "?code=" + authorization_code + return_info.state_query);
    request.respond(http::head_response(http::http_status_code::found));
}

} // Anonymous namespace

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
