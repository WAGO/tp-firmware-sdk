//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of URI helper class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

#include "wago/authserv/http/uri.hpp"
#include "utils/url_encode.hpp"

#include <wc/log.h>

#include <algorithm>
#include <regex>

namespace wago {
namespace authserv {
namespace http {
namespace {

// Regex to parse query part:
// Groups: 0) a whole key value pair
//         1) the key
//         2) the value
std::regex const match_query_params("[?]?([^&=]+)=?([^&]*)?"); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."

}// Namespace <anonymous>


uri::uri(std::string uri_str)
: uri_m(std::move(uri_str))
{
    //find and remove URI fragment if any exists
    auto fragment_start = uri_m.find('#');
    if(fragment_start != std::string::npos)
    {
        uri_m = uri_m.substr(0, fragment_start);
    }

    auto query_start = uri_m.find('?');
    if(query_start == std::string::npos)
    {
        path_m = uri_m;
    }
    else
    {
        path_m  = uri_m.substr(0, query_start);
        query_m = uri_m.substr(query_start);
    }
}

uri::uri(char const *uri_str)
: uri(std::string(uri_str))
{ }

std::string const &uri::as_string() const noexcept
{
    return uri_m;
}

bool uri::operator==(uri const &other_uri) const noexcept
{
    return uri_m == other_uri.uri_m;
}

std::string uri::get_path() const noexcept
{
    return path_m;
}

std::string uri::get_query() const noexcept
{
    return query_m;
}

std::map<std::string, std::string> uri::parse_query() const
{
    return parse_query(query_m);
}

std::map<std::string, std::string> uri::parse_query(std::string const &query)
{
    std::map<std::string, std::string> query_params;
    auto query_begin = std::sregex_iterator(query.begin(), query.end(), match_query_params);
    auto query_end   = std::sregex_iterator();
    for(std::sregex_iterator match_it = query_begin; match_it != query_end; ++match_it)
    {
        std::smatch match           = *match_it;
        std::string parameter_name  = match[1];
        std::string parameter_value = match[2];
        WC_DEBUG_LOG("Found parameter \"" + parameter_name + "\" value: \"" + parameter_value + "\"");
        query_params.emplace(std::move(parameter_name), std::move(parameter_value));
    }
    WC_DEBUG_LOG("Found " + std::to_string(query_params.size()) + " parameters");
    return query_params;
}

} // Namespace http
} // Namespace authserv
} // Namespace wago

//---- End of source file ------------------------------------------------------
