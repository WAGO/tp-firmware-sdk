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
///  \brief    Authenticator implementation for WAGO Parameter Service
///            on Linux controllers using PAM.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "authenticator.hpp"
#include "definitions.hpp"
#include "wago/authserv/http/request_i.hpp"
#include "wago/authserv/exception.hpp"
#include "http/http_exception.hpp"
#include "http/head_response.hpp"
#include "group_management.hpp"

#include <wc/log.h>

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
namespace authserv {

using std::string;
using std::vector;
using std::regex;

namespace {
// "admin" is the only user who may have an empty password during commissioning. 
// To keep implementation both simple and optimized for that case, the username is 
// hard-coded right here.
constexpr char const admin_username[]                       = "admin";
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
authenticator::authenticator(std::shared_ptr<password_backend_i>               password_backend,
                             std::shared_ptr<password_change_backend_i>        password_change_backend,
                             std::string                                const &service_base)
: password_backend_m(std::move(password_backend))
, password_change_backend_m(std::move(password_change_backend))
, service_base_m(service_base)
, realm_m(service_base.substr(1))
, admin_login_without_password(true) // true until detected otherwise, as this is only checked until once false
{
    WC_ASSERT(password_backend_m != nullptr);
    WC_ASSERT(password_change_backend_m != nullptr);
}

authenticator::~authenticator() noexcept = default;

bool authenticator::admin_has_no_password()
{
#ifdef ENFORCE_SET_ADMIN_PASSWORD
    // return immediately if determined to be already set to false
    if (!admin_login_without_password) return false; // assuming that the password can't be removed

    admin_login_without_password = password_backend_m->authenticate(admin_username, "").success;
    return admin_login_without_password;
#else
    return false;
#endif
}

bool authenticator::has_form_auth_data(http::request_i const &request)
{
    return (     request.has_form_parameter(form_param_username)
             &&  request.has_form_parameter(form_param_password));
}

bool authenticator::has_form_password_change_data(http::request_i const &request)
{
    return (    has_form_auth_data(request)
            &&  (request.has_form_parameter(form_param_new_password) || request.has_form_parameter("new-password")) // FIXME remove legacy parameter
            && !request.get_form_parameter(form_param_username).empty()); // username should be set by server
}

auth_result authenticator::authenticate(http::request_i          const &request,
                                        std::vector<std::string> const &scopes)
{
    auth_result authenticated = { false, false, false, "" };

    try
    {
        // authentication information from form data
        if(has_form_auth_data(request))
        {
            // TODO: Handle when user denies access for requested scopes - for future implementations
            std::string const user     = request.get_form_parameter(form_param_username);
            std::string const password = request.get_form_parameter(form_param_password);
            if (user.empty()) {
                authenticated.success = false;
            }
            else {
                // Authenticate using PAM
                authenticated = password_backend_m->authenticate(user, password);
            }

            // Do not remember user name for failed autentication attempts
            // to avoid exposed passwords typed into field for user name
            if (authenticated.success)
            {
                authenticated.user_name = user;
            }

            if(authenticated.success && !is_authorized_for_scope(user, scopes))
            {
                authenticated.unauthorized = true;
            }
        }
        else
        {
            // No known authentication request
            wc_log(log_level_t::warning, "Not all required parameter for authentication provided");
            throw http::http_exception("Invalid authentication data");
        }
    }
    catch(http::http_exception &e)
    {
        throw;
    }
    catch(std::exception &e)
    {
        std::string const message = std::string("Internal server error on authentication request handling: ") + e.what();
        wc_log(log_level_t::error, message);
        throw http::http_exception(message);
    }
    catch(...)
    {
        std::string const message = "Internal server error on authentication request handling (invalid exception caught)";
        wc_log(log_level_t::error, message);
        throw http::http_exception(message);
    }

    return authenticated;
}

wago::future<void> authenticator::password_change(http::request_i const &request)
{
    try
    {
        // authentication information from form data
        if(has_form_password_change_data(request))
        {
            // TODO: Handle when user denies access for requested scopes - for future implementations
            std::string const user                 = request.get_form_parameter(form_param_username);
            std::string const password             = request.get_form_parameter(form_param_password);
            std::string       new_password;
            if (request.has_form_parameter(form_param_new_password))
            {
                new_password = request.get_form_parameter(form_param_new_password);
            }
            else
            {
                new_password = request.get_form_parameter("new-password"); // FIXME remove legacy parameter
            }
            return password_change_backend_m->change_password(user, password, new_password);
        }
        else
        {
            // No known password change request
            wc_log(log_level_t::warning, "Not all required parameter for password change provided");
            throw http::http_exception("Invalid password change data");
        }
    }
    catch (password_change_error &e)
    {
        throw;
    }
    catch(http::http_exception &e)
    {
        throw;
    }
    catch(std::exception &e)
    {
        std::string const message = std::string("Internal server error on password change request handling: ") + e.what();
        wc_log(log_level_t::error, message);
        throw http::http_exception(message);
    }
    catch(...)
    {
        std::string const message = "Internal server error on password change request handling (invalid exception caught)";
        wc_log(log_level_t::error, message);
        throw http::http_exception(message);
    }
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
