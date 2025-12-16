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
///  \brief    OAuth 2.0 resource provider implementation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "oauth2_resource_provider.hpp"
#include "system_abstraction_serv.hpp"

#include <wc/base64.h>
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
oauth2_resource_provider::verify_result evaluate_verify_result(sal::curl::request_result const &request_result);
oauth2_resource_provider::verify_result extract_verify_result_data(nlohmann::json const &result_document);
std::string base64_encode(uint8_t const * raw_data,
                          size_t          raw_data_length);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
oauth2_resource_provider::oauth2_resource_provider(std::string const &client_id,
                                                   std::string const &provider_scope,
                                                   std::string const &server_origin,
                                                   std::string const &verify_path,
                                                   std::string const &client_secret)
: client_id_m(client_id)
, provider_scope_m(provider_scope)
, server_origin_m(server_origin)
, verify_path_m(verify_path)
, client_secret_m(client_secret)
{}

oauth2_resource_provider::~oauth2_resource_provider() noexcept = default;

oauth2_resource_provider::verify_result oauth2_resource_provider::access_token_verification(std::string const &access_token) noexcept
{
    oauth2_resource_provider::verify_result result;
    std::string const fail_prefix = "Failed to verify access token on OAuth 2 server";

    try
    {
        // Build headers
        std::map<std::string, std::string> additional_header_map;
        if (client_secret_m != "")
        {
            std::string basic_token(client_id_m + ":" + client_secret_m);
            std::string basic_token_b64 = base64_encode(reinterpret_cast<uint8_t const *>(basic_token.c_str()), // parasoft-suppress CERT_C-EXP39-b-3 "Base64 works with uint8_t while string works on char."
                                                        basic_token.size());

            additional_header_map.emplace("Authorization", "Basic " + basic_token_b64);
        }

        std::string const post_data = "token_type_hint=access_token"
                                      "&token=" + access_token;
        std::string const url = server_origin_m + verify_path_m;
        wc_log(log_level_t::info, ("Send POST with password credentials to OAuth 2 server (" + url + ")").c_str());
        auto post_result = sal::curl::get_instance().post_data(url,
                                                               "",
                                                               form_data_content_type,
                                                               post_data,
                                                               additional_header_map);
        result = evaluate_verify_result(post_result);
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

oauth2_resource_provider::verify_result evaluate_verify_result(sal::curl::request_result const &request_result)
{
    WC_DEBUG_LOG(("Result data: " + request_result.data).c_str());
    if(request_result.http_code >= 400)
    {
        throw std::runtime_error("(HTTP " + std::to_string(request_result.http_code) + ")");
    }
    if(!is_expected_content_type(request_result.content_type))
    {
        throw std::runtime_error("Unexpected content type delivered");
    }

    return extract_verify_result_data(nlohmann::json::parse(request_result.data));
}

oauth2_resource_provider::verify_result extract_verify_result_data(nlohmann::json const &result_document)
{
    oauth2_resource_provider::verify_result result;

    // Validate document
    if(!result_document.contains("active"))
    {
        throw std::runtime_error("Unexpected result document: No \"active\" included");
    }

    // All necessary preconditions matched, get primary member
    result.active = result_document.at("active").get<bool>();

    // Extract optional result data
    if(result_document.contains("scope"))
    {
        result.scope = result_document.at("scope").get<std::string>();
    }
    if(result_document.contains("client_id"))
    {
        result.client_id = result_document.at("client_id").get<std::string>();
    }
    if(result_document.contains("username"))
    {
        result.username = result_document.at("username").get<std::string>();
    }
    if(result.username.empty())
    {
        throw std::runtime_error("No username provided");
    }

    result.expires_in = 1;
    if(result_document.contains("expires_in"))
    {
        auto raw_value = result_document.at("expires_in").get<uint64_t>();
        if(raw_value >= std::numeric_limits<uint32_t>::max())
        {
            wc_log(log_level_t::warning, "Received value for \"expires_in\" to large, skip processing");
            result.expires_in = std::numeric_limits<uint32_t>::max() - 1;
        }
        else
        {
            result.expires_in = static_cast<uint32_t>(raw_value);
        }
    }
    else if(result_document.contains("exp"))
    {
        uint64_t raw_value = result_document.at("exp").get<uint64_t>();

        timespec current_time;
        sal::system_clock::get_instance().gettime(CLOCK_MONOTONIC, &current_time);
        uint64_t current_timestamp_seconds = (uint64_t) current_time.tv_sec;
        if(raw_value >= std::numeric_limits<uint32_t>::max())
        {
            wc_log(log_level_t::warning, "Received value for \"exp\" to large, skip processing");
            result.expires_in = std::numeric_limits<uint32_t>::max() - 1;
        }
        else if (raw_value < current_timestamp_seconds )
        {
            wc_log(log_level_t::warning, "Received value for \"exp\" which is in the past");
            result.expires_in = 0;
        }
        else
        {
            result.expires_in = static_cast<uint32_t>(raw_value - current_timestamp_seconds);
        }
    }

    return result;
}

std::string base64_encode(uint8_t const * raw_data,
                          size_t          raw_data_length)
{
    auto const expected_encoded_length = wc_base64_calc_encoded_length(raw_data_length);
    if((expected_encoded_length == 0) && (raw_data_length != 0))
    {
        throw std::runtime_error("Failed to encode to Base64 string: payload too large");
    }
    char   encoded[expected_encoded_length];
    size_t encoded_length = 0;
    if(!wc_base64_encode(raw_data, raw_data_length, encoded, &encoded_length))
    {
        throw std::runtime_error("Failed to encode to Base64 string");
    }

    return encoded;
}

} // Anonymous namespace
} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
