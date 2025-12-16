//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WEB-API frontend for the WAGO Auth Service password change functionality.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "password_change.hpp"
#include "definitions.hpp"
#include "response_helper.hpp"
#include "password_change_backend_i.hpp"
#include "http/http_exception.hpp"
#include "wago/authserv/http/request_i.hpp"
#include "system_abstraction.hpp"
#include "authenticator_i.hpp"

#include <wc/log.h>

#include <stdexcept>

#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

void change_user_password(std::shared_ptr<http::request_i>          request,
                          std::shared_ptr<response_helper_i> const  html_templater,
                          authenticator_i                          &auth_handler);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
void process_password_change_request(std::shared_ptr<response_helper_i> const  html_templater,
                                     std::shared_ptr<http::request_i>          request,
                                     authenticator_i                          &auth_handler)
{
    try
    {
        if (request->get_method() == http::http_method::get)
        {
            if(!request->has_query_parameter(query_param_user))
            {
                html_templater->send_error_confirmation(*request, response_helper_i::CLOSE_TAB, http::http_status_code::bad_request, response_helper_i::invalid_request, "Missing query parameter 'username'");
            }
            std::string username = request->get_query_parameter(query_param_user);
            html_templater->send_password_change_page(*request, response_helper_i::CLOSE_TAB, http::http_status_code::ok, username);
        }
        else if (request->get_method() == http::http_method::post)
        {
            if (auth_handler.has_form_password_change_data(*request))
            {
                change_user_password(request, html_templater, auth_handler);
            }
            else
            {
                html_templater->send_error_confirmation(*request, response_helper_i::CLOSE_TAB, http::http_status_code::bad_request, response_helper_i::invalid_request, "Invalid form data");
            }
        }
        else
        {
            html_templater->send_error_confirmation(*request, response_helper_i::CLOSE_TAB, http::http_status_code::method_not_allowed);
        }
    }
    catch(http::http_exception const &e)
    {
        html_templater->send_error_confirmation(*request, response_helper_i::CLOSE_TAB, e.get_http_status_code());
    }
    catch(...)
    {
        html_templater->send_error_confirmation(*request, response_helper_i::CLOSE_TAB, http::http_status_code::internal_server_error);
    }
}

#define NO_SCOPES {}
void change_user_password(std::shared_ptr<http::request_i>          request,
                          std::shared_ptr<response_helper_i> const  html_templater,
                          authenticator_i                          &auth_handler) {
    // authenticate bevore password change because the password change provider doesn't report wrong passwords
    auto auth_result = auth_handler.authenticate(*request, NO_SCOPES);
    if(!auth_result.success)
    {
        html_templater->send_password_change_page(*request, response_helper_i::CLOSE_TAB, http::http_status_code::bad_request, auth_result.user_name, response_helper_i::invalid_username_or_password);
    }
    else
    {
        try
        {
            // change password
            wago::future<void> pw_change_success = auth_handler.password_change(*request);

            pw_change_success.set_notifier([html_templater, request]()
            {
                std::string username = request->get_form_parameter(form_param_username);
                wc_log(log_level_t::info, "Successfully updated password for user \"" + username + "\"");
                html_templater->send_success_confirmation(*request, response_helper_i::CLOSE_TAB, http::http_status_code::ok, "Successfully updated password for user " + username + ".");
            });
            pw_change_success.set_exception_notifier([request, html_templater](std::exception_ptr &&e_ptr)
            {
                try
                {
                    std::rethrow_exception(e_ptr);
                }
                catch (password_change_error const &e)
                {
                    std::string username = request->get_form_parameter(form_param_username);
                    html_templater->send_password_change_page(*request, response_helper_i::CLOSE_TAB, http::http_status_code::bad_request, username, response_helper_i::internal_error, e.what());
                }
                catch (...)
                {
                    wc_log(log_level_t::error, "Got unexpected error while changing password");
                    html_templater->send_error_confirmation(*request, response_helper_i::CLOSE_TAB, http::http_status_code::internal_server_error);
                }
            });
        }
        catch (password_change_error const &e)
        {
            std::string username = request->get_form_parameter(form_param_username);
            html_templater->send_password_change_page(*request, response_helper_i::CLOSE_TAB, http::http_status_code::bad_request, username, response_helper_i::internal_error, e.what());
        }
    }
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
