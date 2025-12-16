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
///  \brief    Response creater for requests to the WAGO Auth Service.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "response_helper.hpp"
#include "definitions.hpp"
#include "http/head_response.hpp"
#include "http/http_exception.hpp"
#include "system_abstraction.hpp"
#include "utils/html_escape.hpp"
#include "wago/authserv/http/http_status_code.hpp"
#include "wago/authserv/http/request_i.hpp"
#include "wago/authserv/settings_store_i.hpp"

#include <wc/log.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {
    void preload_template(std::string const &file_path, std::vector<std::string> &template_cache);
    bool json_response_requested(http::request_i const &request);
    std::string get_enum_error_representation(response_helper_i::error_type const &error_type);
    std::string get_error_message(response_helper_i::error_type const &error_type);
    void apply_and_send_template(http::request_i                          &request,
                                 http::http_status_code                    code,
                                 std::vector<std::string>           const &web_template,
                                 std::map<std::string, std::string> const &variables);
    void replace_html_template_variables(std::string                              &source, 
                                         std::map<std::string, std::string> const &variables);
    void send_json_confirmation(http::request_i                     &request,
                                http::http_status_code        const &code,
                                response_helper_i::error_type const &error_type = response_helper_i::no_error,
                                std::string                   const &additional_error_message = "");
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
response_helper::response_helper(std::string                       const &service_name,
                                 std::shared_ptr<settings_store_i>        settings_store)
: escaped_service_name_m(html_escape(service_name))
, settings_store_m(std::move(settings_store))
{
    preload_template(password_setup_page_file_path,  password_setup_template_m);
    preload_template(login_page_file_path,           login_template_m);
    preload_template(password_change_page_file_path, password_change_template_m);
    preload_template(confirmation_page_file_path, confirmation_template_m);
}

void response_helper::send_password_setup_page(http::request_i                    &request,
                                               std::string                  const &cancel_uri,
                                               http::http_status_code              code,
                                               response_helper_i::error_type const &error_type,
                                               std::string                  const &additional_error_message) const
{
    if (json_response_requested(request)) {
        send_json_confirmation(request, code, error_type, additional_error_message);
        return;
    }

    char hostname[1024];
    if (0 != sal::hostname::get_instance().gethostname(hostname, 1024))
    {
        hostname[0] = '\0';
    }
    bool silent_mode = settings_store_m->get_global_setting(settings_store_i::silent_mode_enabled) == "true";
    std::map<std::string, std::string> variables = {
        {"HOSTNAME",                html_escape(hostname)},
        {"SERVICE_NAME",            escaped_service_name_m},
        {"SERVICE_VERSION",         WC_SUBST_STR(AUTHSERV_VERSION)},
        {"API_VERSION",             WC_SUBST_STR(AUTH_API_VERSION)},
        {"USER_NAME",               "admin"},
        {"CANCEL_URI",              cancel_uri}, // used in attribute string (escaping not applicable)
        {"ERROR_MESSAGE",           html_escape(get_error_message(error_type) + additional_error_message)},
        {"SILENT_MODE_CLASS",       silent_mode ? "silent-mode" : ""},
        {"PAGE_TITLE",              (silent_mode ? "" : "WAGO ") + std::string("Password setup")},
        {"FAVICON_HREF",            silent_mode ? "data:," : "images/favicon.ico"}
    };
    apply_and_send_template(request, code, password_setup_template_m, variables);
}

void response_helper::send_login_page(http::request_i                     &request,
                                      std::string                   const &cancel_uri,
                                      http::http_status_code               code,
                                      std::string                   const &accessing_client_label,
                                      response_helper_i::error_type const &error_type,
                                      std::string                   const &additional_error_message,
                                      std::string                   const &success_message) const
{
    // Message for invalid username and / or password is more important than failure loading "system use notification"
    if (json_response_requested(request)) {
        send_json_confirmation(request, code, error_type, additional_error_message);
        return;
    }
    
    char hostname[1024];
    if (0 != sal::hostname::get_instance().gethostname(hostname, 1024))
    {
        hostname[0] = '\0';
    }

    std::string system_use_notification = settings_store_m->get_global_setting(settings_store_i::system_use_notification);
    std::string system_use_notification_error = "";
    try
    {
        if(system_use_notification.empty() && sal::filesystem::get_instance().is_regular_file(system_use_notification_file))
        {
            auto stream = sal::filesystem::get_instance().open_stream(system_use_notification_file, std::ios_base::in);
            system_use_notification.resize(system_use_notification_max);
            stream->read(&system_use_notification[0], system_use_notification_max);
            system_use_notification.resize(stream->gcount());
            if(!stream->eof())
            {
                wc_log(log_level_t::warning, "System use notification exceeds " +
                                             std::to_string(system_use_notification_max) + " bytes, "
                                             "cutting notification.");
            }
        }
    }
    catch(std::exception const &e)
    {
        system_use_notification_error = "Failed to load system use notification";
        wc_log(log_level_t::error, "Failed to load system use notification: " + std::string(e.what()));
    }


    // Trim "system use notification" from begin
    system_use_notification.erase(system_use_notification.begin(),
                                  std::find_if(system_use_notification.begin(), system_use_notification.end(), [](unsigned char ch) {
                                      return !std::isspace(ch);
                                  }));

    // Trim "system use notification" from end
    system_use_notification.erase(std::find_if(system_use_notification.rbegin(), system_use_notification.rend(), [](unsigned char ch) {
                                      return !std::isspace(ch);
                                  }).base(),
                                  system_use_notification.end());

    bool silent_mode = settings_store_m->get_global_setting(settings_store_i::silent_mode_enabled) == "true";
    std::map<std::string, std::string> variables = {
        {"HOSTNAME",                html_escape(hostname)},
        {"SERVICE_NAME",            escaped_service_name_m},
        {"SERVICE_VERSION",         WC_SUBST_STR(AUTHSERV_VERSION)},
        {"API_VERSION",             WC_SUBST_STR(AUTH_API_VERSION)},
        {"CLIENT_NAME",             html_escape(accessing_client_label)},
        {"CANCEL_URI",              cancel_uri}, // used in attribute string (escaping not applicable)
        {"ERROR_MESSAGE",           html_escape(
            get_error_message(error_type) + 
            (additional_error_message=="" ? system_use_notification_error : additional_error_message)
        )},
        {"SUCCESS_MESSAGE",         html_escape(success_message)},
        {"SYSTEM_USE_NOTIFICATION", html_escape(system_use_notification)},
        {"SILENT_MODE_CLASS",       silent_mode ? "silent-mode" : ""},
        {"PAGE_TITLE",              (silent_mode ? "" : "WAGO ") + std::string("Login")},
        {"FAVICON_HREF",            silent_mode ? "data:," : "images/favicon.ico"}
    };
    apply_and_send_template(request, code, login_template_m, variables);
}

void response_helper::send_password_change_page(http::request_i                     &request,
                                                std::string                   const &cancel_uri,
                                                http::http_status_code               code,
                                                std::string                   const &user_name,
                                                response_helper_i::error_type const &error_type,
                                                std::string                   const &additional_error_message,
                                                std::string                   const &info_message) const
{
    if (json_response_requested(request)) {
        send_json_confirmation(request, code, error_type, additional_error_message);
        return;
    }

    char hostname[1024];
    if (0 != sal::hostname::get_instance().gethostname(hostname, 1024))
    {
        hostname[0] = '\0';
    }

    bool silent_mode = settings_store_m->get_global_setting(settings_store_i::silent_mode_enabled) == "true";
    std::map<std::string, std::string> variables = {
        {"HOSTNAME",          html_escape(hostname)},
        {"SERVICE_NAME",      escaped_service_name_m},
        {"SERVICE_VERSION",   WC_SUBST_STR(AUTHSERV_VERSION)},
        {"API_VERSION",       WC_SUBST_STR(AUTH_API_VERSION)},
        {"USER_NAME",         html_escape(user_name)},
        {"CANCEL_URI",        cancel_uri}, // used in attribute string (escaping not applicable)
        {"ERROR_MESSAGE",     html_escape(get_error_message(error_type) + additional_error_message)},
        {"INFO_MESSAGE",      html_escape(info_message)},
        {"SILENT_MODE_CLASS", silent_mode ? "silent-mode" : ""},
        {"PAGE_TITLE",        (silent_mode ? "" : "WAGO ") + std::string("Password change")},
        {"FAVICON_HREF",      silent_mode ? "data:," : "images/favicon.ico"}
    };
    apply_and_send_template(request, code, password_change_template_m, variables);
}

constexpr char default_success_message[] = "Operation succeeded.";
void response_helper::send_success_confirmation(http::request_i              &request,
                                                std::string            const &continue_uri,
                                                http::http_status_code        code,
                                                std::string            const &success_message) const
{
    if (json_response_requested(request)) {
        send_json_confirmation(request, code);
        return;
    }

    char hostname[1024];
    if (0 != sal::hostname::get_instance().gethostname(hostname, 1024))
    {
        hostname[0] = '\0';
    }

    bool silent_mode = settings_store_m->get_global_setting(settings_store_i::silent_mode_enabled) == "true";
    std::map<std::string, std::string> variables = {
        {"HOSTNAME",          html_escape(hostname)},
        {"SERVICE_NAME",      escaped_service_name_m},
        {"SERVICE_VERSION",   WC_SUBST_STR(AUTHSERV_VERSION)},
        {"API_VERSION",       WC_SUBST_STR(AUTH_API_VERSION)},
        {"CONTINUE_URI",      continue_uri}, // used in attribute string (escaping not applicable)
        {"SUCCESS_MESSAGE",   success_message=="" ? default_success_message : html_escape(success_message)},
        {"SILENT_MODE_CLASS", silent_mode ? "silent-mode" : ""},
        {"PAGE_TITLE",        (silent_mode ? "" : "WAGO ") + std::string("Success")},
        {"FAVICON_HREF",      silent_mode ? "data:," : "images/favicon.ico"}
    };
    apply_and_send_template(request, code, confirmation_template_m, variables);
}

void response_helper::send_error_confirmation(http::request_i                     &request,
                                              std::string                   const &continue_uri,
                                              http::http_status_code               code,
                                              response_helper_i::error_type const &error_type,
                                              std::string                   const &additional_error_message) const
{
    if (json_response_requested(request)) {
        send_json_confirmation(request, code, error_type, additional_error_message);
        return;
    }

    char hostname[1024];
    if (0 != sal::hostname::get_instance().gethostname(hostname, 1024))
    {
        hostname[0] = '\0';
    }

    bool silent_mode = settings_store_m->get_global_setting(settings_store_i::silent_mode_enabled) == "true";
    std::map<std::string, std::string> variables = {
        {"HOSTNAME",          html_escape(hostname)},
        {"SERVICE_NAME",      escaped_service_name_m},
        {"SERVICE_VERSION",   WC_SUBST_STR(AUTHSERV_VERSION)},
        {"API_VERSION",       WC_SUBST_STR(AUTH_API_VERSION)},
        {"CONTINUE_URI",      continue_uri}, // used in attribute string (escaping not applicable)
        {"ERROR_MESSAGE",     html_escape(get_error_message(error_type) + additional_error_message)},
        {"SILENT_MODE_CLASS", silent_mode ? "silent-mode" : ""},
        {"PAGE_TITLE",        (silent_mode ? "" : "WAGO ") + std::string("Error")},
        {"FAVICON_HREF",      silent_mode ? "data:," : "images/favicon.ico"}
    };
    apply_and_send_template(request, code, confirmation_template_m, variables);
}


namespace {

void preload_template(std::string const &file_path, std::vector<std::string> &template_cache)
{
    auto file_stream = sal::filesystem::get_instance().open_stream(file_path, std::ios_base::in);
    for(std::string line; std::getline(*file_stream, line); )
    {
        template_cache.push_back(line);
    }
    WC_DEBUG_LOG("Preloaded " + std::to_string(template_cache.size()) + " "
                 "lines for template \"" + file_path + "\"");
}

bool json_response_requested(http::request_i const &request)
{
    return request.get_accepted_types().find("application/json") != std::string::npos;
}

std::string get_enum_error_representation(response_helper_i::error_type const &error_type)
{
    switch(error_type) {
        case response_helper_i::no_error:
            return "";
        case response_helper_i::internal_error:
            return "internal_error";
        case response_helper_i::invalid_request:
            return "invalid_request";
        case response_helper_i::invalid_username_or_password:
            return "invalid_username_or_password";
        case response_helper_i::invalid_new_password:
            return "invalid_new_password";
        default:
            return "internal_error";
    };
}

std::string get_error_message(response_helper_i::error_type const &error_type)
{
    switch(error_type) {
        case response_helper_i::no_error:
            return "";
        case response_helper_i::internal_error:
            return "Internal server error. ";
        case response_helper_i::invalid_request:
            return "Invalid request. ";
        case response_helper_i::invalid_username_or_password:
            return "Invalid username and / or password. ";
        case response_helper_i::invalid_new_password:
            return "New password doesn't meet criteria. ";
        default:
            return "An unexpected error occured. ";
    };
}

void apply_and_send_template(http::request_i                          &request,
                             http::http_status_code                    code,
                             std::vector<std::string>           const &web_template,
                             std::map<std::string, std::string> const &variables)
{
    http::head_response response(code);
    response.set_content_type("text/html");
    request.respond(response);

    for(size_t i = 0; i < web_template.size(); ++i)
    {
        std::string line = web_template.at(i);

        // check for placeholders to replace with variable values
        replace_html_template_variables(line, variables);
        request.send_data(line.c_str(), line.size());
    }
    request.finish();
}

void replace_html_template_variables(std::string                              &source, 
                                     std::map<std::string, std::string> const &variables)
{
    size_t offset   = 0;
    size_t placeholder_start = 0;
    size_t placeholder_end   = 0;
    while(    ((placeholder_start = source.find("$$", offset))       != std::string::npos) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional assignment. Parasoft detects a false-positive here."
           && ((placeholder_end   = source.find("$$", placeholder_start + 2)) != std::string::npos)) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional assignment. Parasoft detects a false-positive here."
    {
        auto        placeholder_len   = placeholder_end - placeholder_start + 2;
        std::string placeholder       = source.substr(placeholder_start, placeholder_len);
        auto        variable_name_len = placeholder_end - placeholder_start - 2;
        std::string variable_name     = source.substr(placeholder_start + 2, variable_name_len);
        std::string variable_value    = variables.count(variable_name) > 0 ? variables.at(variable_name) : "";
        source.replace(placeholder_start, placeholder_len, variable_value);
        offset = placeholder_start + variable_value.length();
    }
}

void send_json_confirmation(http::request_i                     &request,
                            http::http_status_code        const &code,
                            response_helper_i::error_type const &error_type,
                            std::string                   const &additional_error_message)
{
    http::head_response response(code);
    response.set_content_type("application/json");
    request.respond(response);

    // add json body
    auto json_content = nlohmann::json({
        {"success", error_type == response_helper_i::no_error}
    });

    if (error_type != response_helper_i::no_error)
    {
        json_content.emplace("error", get_enum_error_representation(error_type));
        if (additional_error_message != "")
        {
            json_content.emplace("error_description", additional_error_message);
        }
    }

    std::string body = json_content.dump();
    request.send_data(body.c_str(), body.size());
    request.finish();
}
} // Namespace anonym

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
