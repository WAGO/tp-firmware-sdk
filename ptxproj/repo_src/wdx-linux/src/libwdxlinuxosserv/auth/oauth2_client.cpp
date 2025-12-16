//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    OAuth 2.0 client implementation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "oauth2_client.hpp"
#include "system_abstraction_serv.hpp"
#include "utils/url_encode.hpp"

#include <wc/log.h>
#include <wc/compiler.h>

#include <nlohmann/json.hpp>
#include <algorithm>
#include <limits>

#include <ctype.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace auth {

//------------------------------------------------------------------------------
// global variables and const expressions
//------------------------------------------------------------------------------
constexpr char const form_data_content_type[] = "application/x-www-form-urlencoded";

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------
namespace {
inline GNUC_ATTRIBUTE(const) std::string to_lower(std::string convert)
{
    std::transform(convert.begin(), convert.end(), convert.begin(), ::tolower);
    return convert;
}

bool is_expected_content_type(std::string const &candidate);
oauth2_client::token_result evaluate_token_result(sal::curl::request_result const &request_result);
oauth2_client::token_result extract_token_result_data(nlohmann::json const &result_document);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
oauth2_client::oauth2_client(std::string const &client_id,
                             std::string const &server_origin,
                             std::string const &token_path,
                             std::string const &verify_path,
                             std::string const &client_secret)
: client_id_m(client_id)
, server_origin_m(server_origin)
, token_path_m(token_path)
, verify_path_m(verify_path)
, client_secret_m(client_secret)
{}

oauth2_client::~oauth2_client() noexcept = default;

oauth2_client::token_result oauth2_client::resource_owner_password_credentials_grant(std::string const &scopes,
                                                                                     std::string const &user,
                                                                                     std::string const &password) noexcept
{
    oauth2_client::token_result result;
    std::string const fail_prefix = "Failed to authenticate with password credentials on OAuth 2 server";

    try
    {
        // OAuth 2 client credentials may part of the body as 'client_id' and 'client_secret'.
        // An empty string as secret may be omitted (https://datatracker.ietf.org/doc/html/rfc6749#section-2.3.1).
        auto scopes_escaped   = encode_url(scopes);
        auto user_escaped     = encode_url(user);
        auto password_escaped = encode_url(password);

        // add client secret if present
        std::string client_secret_arg = "";
        if(!client_secret_m.empty())
        {
            client_secret_arg = "&client_secret=" + client_secret_m;
        }

        std::string const post_data = "grant_type=password"
                                      "&client_id=" + client_id_m +
                                      client_secret_arg +
                                      "&scope="     + scopes_escaped +
                                      "&username="  + user_escaped +
                                      "&password="  + password_escaped;
        std::string const url = server_origin_m + token_path_m;
        wc_log(log_level_t::info, ("Send POST with password credentials to OAuth 2 server (" + url + ")").c_str());
        auto post_result = sal::curl::get_instance().post_data(url, "",
                                                               form_data_content_type,
                                                               post_data);
        result = evaluate_token_result(post_result);
    }
    catch(sal::curl::exception const &ce)
    {
        std::string const message = fail_prefix + ": libcurl: "
                                  + ce.what()
                                  + " (" + std::to_string(ce.get_curl_code()) + ")";
        wc_log(log_level_t::error, message.c_str());
    }
    catch(nlohmann::json::exception const &je)
    {
        std::string const base_message = fail_prefix + ": ";
        wc_log(log_level_t::debug, (base_message + je.what()).c_str());
        wc_log(log_level_t::error, (base_message + "Malformed or unexpected result document").c_str());
    }
    catch(std::exception const &e)
    {
        std::string const message = fail_prefix + ": " + e.what();
        wc_log(log_level_t::error, message.c_str());
    }

    return result;
}

oauth2_client::token_result oauth2_client::refresh_token_grant(std::string const refresh_token) noexcept
{
    oauth2_client::token_result result;
    std::string const fail_prefix = "Failed refresh token grant on OAuth 2 server";

    try
    {
        std::string const post_data = "grant_type=refresh_token"
                                      "&refresh_token=" + refresh_token;
        std::string const url = server_origin_m + token_path_m;
        wc_log(log_level_t::info, ("Send POST with password credentials to OAuth 2 server (" + url + ")").c_str());
        auto post_result = sal::curl::get_instance().post_data(url, "",
                                                               form_data_content_type,
                                                               post_data);
        result = evaluate_token_result(post_result);
    }
    catch(sal::curl::exception const &ce)
    {
        std::string const message = fail_prefix + ": libcurl: "
                                  + ce.what()
                                  + " (" + std::to_string(ce.get_curl_code()) + ")";
        wc_log(log_level_t::error, message.c_str());
    }
    catch(nlohmann::json::exception const &je)
    {
        std::string const base_message = fail_prefix + ": ";
        wc_log(log_level_t::debug, (base_message + je.what()).c_str());
        wc_log(log_level_t::error, (base_message + "Malformed or unexpected result document").c_str());
    }
    catch(std::exception const &e)
    {
        std::string const message = fail_prefix + ": " + e.what();
        wc_log(log_level_t::error, message.c_str());
    }

    // Re-use refresh token if no new one is given
    if(result.refresh_token.empty())
    {
        result.refresh_token = refresh_token;
        wc_log(log_level_t::info, "Found no new refresh token in result, use current one again");
    }

    return result;
}

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
bool is_expected_content_type(std::string const &candidate)
{
    auto const unified_candidate = to_lower(candidate);
    return (    (unified_candidate == "application/json;charset=utf-8")
             || (unified_candidate == "application/json"));
}

oauth2_client::token_result evaluate_token_result(sal::curl::request_result const &request_result)
{
    bool is_json = false;
    nlohmann::json result_document;
    WC_DEBUG_LOG(("Result data: " + request_result.data).c_str());
    if(is_expected_content_type(request_result.content_type))
    {
        is_json = true;
        if(request_result.data.length() > 0)
        {
            result_document = nlohmann::json::parse(request_result.data);
        }
    }
    if(request_result.http_code >= 400)
    {
        std::string const error_type        = result_document.contains("error")             ?
                                              result_document.at(      "error").get<std::string>()             : "";
        std::string const error_description = result_document.contains("error_description") ?
                                              result_document.at(      "error_description").get<std::string>() : "";
        std::string const message = ((request_result.http_code >= 500) ? "Internal Server Error" : "Bad Request")
                                  + (error_type.empty()        ? "" : (": " + error_type))
                                  + (error_description.empty() ? "" : (": " + error_description))
                                  + " (HTTP " + std::to_string(request_result.http_code) + ")";
        throw std::runtime_error(message);
    }
    if(!is_json)
    {
        throw std::runtime_error("Unexpected content type delivered");
    }
    if(request_result.data.empty())
    {
        throw std::runtime_error("Unexpected result document: No content included");
    }

    return extract_token_result_data(result_document);
}

oauth2_client::token_result extract_token_result_data(nlohmann::json const &result_document)
{
    oauth2_client::token_result result;

    // Validate document
    if(!result_document.contains("token_type"))
    {
        throw std::runtime_error("Unexpected result document: No \"token_type\" included");
    }
    if(to_lower(result_document.at("token_type").get<std::string>()) != "bearer")
    {
        throw std::runtime_error(("Unknown token type \"" + result_document.at("token_type").get<std::string>() + "\"").c_str());
    }
    if(!result_document.contains("access_token"))
    {
        throw std::runtime_error("Unexpected result document: No \"access_token\" included");
    }

    // All necessary preconditions matched => success
    result.success = true;
    result.expired = result_document.contains("password_expired") && result_document.at("password_expired").get<bool>();

    // Extract tokens and expiration from POST result data
    result.access_token = result_document.at("access_token").get<std::string>();
    if(result_document.contains("refresh_token"))
    {
        result.refresh_token = result_document.at("refresh_token").get<std::string>();
    }
    if(result_document.contains("expires_in"))
    {
        auto raw_value = result_document.at("expires_in").get<uint64_t>();
        if(raw_value >= std::numeric_limits<uint32_t>::max())
        {
            wc_log(log_level_t::warning, "Received value for \"expires_in\" too large, skip processing");
        }
        else
        {
            result.expires_in = static_cast<uint32_t>(raw_value);
        }
    }

    return result;
}

} // Anonymous namespace
} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------

