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
///  \brief    Implementation of REST-API router to route different API requests
///            by URI path.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "router.hpp"
#include "wago/wdx/wda/http/http_status_code.hpp"
#include "http/http_exception.hpp"
#include "http/url_utils.hpp"
#include "request.hpp"
#include "definitions.hpp"

#include <wc/assertion.h>
#include <wc/log.h>

#include <regex>
#include <stdexcept>
#include <cstring>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static std::regex const url_template_validation_regex { // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
    R"(^.*[^/]?$)", std::regex::optimize
};
static std::regex const url_is_lowercase_regex { // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
    R"(^[^A-Z]*$)", std::regex::optimize
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static bool match_base(std::string const &base_url,
                       std::string const &request_path);

static void extract_path_parameters(map<std::string, std::string>       &path_parameters,
                                    std::vector<std::string>      const &parameter_names,
                                    std::smatch                   const &parameter_result);

static bool is_type_accepted(char const * const content_type_part1,
                             char const * const content_type_part2,
                             request_i    const &fcgi_request);

static std::vector<std::string>extract_parameter_names(std::string const template_);

struct router::route_entry
{
    std::string                                 url_template;
    std::string                                 url_pattern;
    std::regex                                  url_match_regex;
    std::vector<std::string>                    parameter_names;
    map<http_method, operation_handler_t const> operation_handlers;
    map<http_method, std::string const>         doc_links;

    route_entry(std::string const template_,
                std::string const pattern)
    : url_template(template_)
    , url_pattern(pattern)
    , url_match_regex(std::regex(url_pattern, std::regex::optimize | std::regex::icase))
    , parameter_names(extract_parameter_names(template_))
    {
        WC_DEBUG_LOG(std::string("Add route for URL match regex: \"") + url_pattern + "\"");
    }

    std::vector<http_method> get_methods() const
    {
        std::vector<http_method> methods;
        for(auto handler : operation_handlers)
        {
            methods.push_back(handler.first);
        }
        return methods;
    }

    void add_handler(http_method                method,
                     operation_handler_t const  handler,
                     std::string         const &doc_link = "")
    {
        WC_ASSERT(has_handler(method) == false);
        operation_handlers.emplace(method, handler);
        if (!doc_link.empty())
        {
            doc_links.emplace(method, doc_link);
        }
    }

    bool has_handler(http_method method) const
    {
        return (operation_handlers.count(method) > 0);
    }
    operation_handler_t const& get_handler(http_method method) const
    {
        WC_ASSERT(has_handler(method) == true);
        return operation_handlers.at(method);
    }
    bool has_doc_link(http_method method) const
    {
        return (doc_links.count(method) > 0);
    }
    std::string const& get_doc_link(http_method method) const
    {
        WC_ASSERT(has_doc_link(method) == true);
        return doc_links.at(method);
    }
};

router::route_result::route_result(rest::request                  request, 
                                   operation_handler_t      const handler,
                                   std::vector<http_method> const allowed_methods)
: request_m(std::move(request))
, handler_m(handler)
, allowed_methods_m(allowed_methods)
{ }

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
router::router(service_identity_i     const &service_identity,
               std::string            const &service_base_url)
: service_identity_m(service_identity)
, service_base_url_m(service_base_url)
{ }

router::~router() noexcept = default;

void router::add_route(http_method         const  method,
                       std::string         const &url_template,
                       operation_handler_t const &handler,
                       std::string         const &doc_link)
{
    // MUST NOT end with a slash
    WC_ASSERT(url_template.back() != '/');

    wc_log_format(info, "Configure new route: \"%s\"", url_template.c_str());
    add_route(method, url_template, handler, doc_link, false, false);

    // implicitly add a redirect for the added route with a trailing slash
    add_route(method, url_template + "/", operation::redirect_trailing_slash, "", false, false);
}

void router::add_redirect(std::string         const &url_template,
                          operation_handler_t const &handler)
{
    wc_log_format(info, "Configure new redirect: \"%s\"", url_template.c_str());
    for (auto const &method : http::all_http_methods)
    {
        add_route(method, url_template, handler, "", true, true);
    } 
}

void router::add_route(http_method         const  method,
                       std::string         const &url_template,
                       operation_handler_t const &handler,
                       std::string         const &doc_link,
                       bool                const  allow_trailing_slash,
                       bool                const  match_following)
{
    WC_ASSERT(validate_url_template(url_template));
    route_entry *entry;
    std::string url_pattern = http::build_url_regex_pattern(url_template, allow_trailing_slash, match_following);
    auto found = std::find_if(routes_m.begin(), routes_m.end(), [&](route_entry &existing_entry) {
        if (existing_entry.url_pattern == url_pattern) {
            // if this assertion fails, two routes have the same resulting pattern and are 
            // going to be confused!
            WC_ASSERT(existing_entry.url_template == url_template);
            return true;
        }
        return false;
    });
    if(found == routes_m.end()) 
    {
        routes_m.emplace_back(route_entry(url_template, url_pattern));
        entry = &routes_m.back();
    }
    else 
    {
        entry = &(*found);
    }
    entry->add_handler(method, handler, doc_link);
}

bool router::validate_url_template(std::string const &url_template)
{
    return regex_match(url_template, url_template_validation_regex);
}

std::vector<http_method> router::find_route(http_method                          method,
                                            operation_handler_t                 &handler,
                                            std::string                   const &request_path,
                                            map<std::string, std::string>       &path_parameters,
                                            std::string                         &doc_link) const
{
    std::vector<http_method> route_methods;
    if(match_base(service_base_url_m, request_path))
    {
        WC_DEBUG_LOG(std::string("Service base: \"") + service_base_url_m + "\"");
        WC_DEBUG_LOG(std::string("Request path: \"") + request_path + "\"");
        std::string const service_uri = request_path.substr(service_base_url_m.length());
        for(auto const &route : routes_m)
        {
            std::smatch parameter_result;
            if(std::regex_match(service_uri, parameter_result, route.url_match_regex))
            {
                WC_DEBUG_LOG(std::string("URL template matched: \"") + route.url_template + "\"");
                // redirect UPPER or MiXeD case urls (letters only)
                if (!std::regex_match(service_uri, url_is_lowercase_regex))
                {
                    handler = &operation::redirect_to_lowercase;
                }
                else if(route.has_handler(method))
                {
                    handler = route.get_handler(method);
                    WC_DEBUG_LOG(std::string("Handler found for: \"") + route.url_template + "\"");
                    if (route.has_doc_link(method))
                    {
                        WC_DEBUG_LOG(std::string("Doc link found for: \"") + route.url_template + "\"");
                        doc_link =  route.get_doc_link(method);
                    }
                }
                else if(method == http_method::head)
                {   
                    // fallback to HEAD operation, if not configured
                    handler = &operation::get_head;
                }
                else
                {
                    handler = &operation::method_not_allowed;
                }
                route_methods = route.get_methods();
                route_methods.push_back(http_method::head);
                extract_path_parameters(path_parameters, route.parameter_names, parameter_result);
                break;
            }
        }
    }
    else
    {
        WC_FAIL(("Routed to an unserved path (misconfigured webserver?): " + request_path).c_str());
    }
    return route_methods;
}

router::route_result router::route(std::unique_ptr<request_i> request) noexcept
{
    operation_handler_t           operation_handler = nullptr;
    map<std::string, std::string> path_parameters;
    std::string                   doc_link;

    auto found_methods = find_route(request->get_method(), operation_handler, request->get_request_uri().get_path(), path_parameters, doc_link);
    bool route_found = !found_methods.empty();

    // always support 'OPTIONS'
    found_methods.push_back(http_method::options);

    serializer_i   const * serializer;
    deserializer_i const * deserializer;

    try {
        serializer = &select_serializer(*request);
    } catch(...) {
        operation_handler = &operation::not_acceptable;
        serializer = &json_api_serializer_m;
    }
    try {
        deserializer = &select_deserializer(*request);
    } catch(...) {
        operation_handler = &operation::not_supported_content_type;
        deserializer = &json_api_serializer_m;
    }

    if(!route_found)
    {
        operation_handler = &operation::not_found;
    }

    // OPTIONS requests should always pass
    if (request->get_method() == http::http_method::options)
    {
        operation_handler = &operation::options;
    }

    rest::request rest_request(std::move(request), *serializer, *deserializer, path_parameters, doc_link);
    return route_result(std::move(rest_request), operation_handler, found_methods);
}


serializer_i const &router::select_serializer(request_i const & request) const
{
    // TODO: select proper serializer based on Accept header
    // Example: Browser sends "" => means text is preferred over anything
    // See chapter 14.1: https://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html

    if(is_type_accepted(json_api_content_type_part1, json_api_content_type_part2, request))
    {
        return json_api_serializer_m;
    }

    std::string const message = std::string("Only \"") + json_api_content_type + "\" is supported.";
    throw http_exception(std::move(message), http_status_code::not_acceptable);
}

deserializer_i const & router::select_deserializer(request_i const &request) const
{
    // Select proper deserializer based on Content-Type header
    bool content_required = request.get_method() == http_method::patch 
                             || request.get_method() == http_method::post 
                             || request.get_method() == http_method::put;
    if(!content_required || request.get_content_type() == std::string(json_api_content_type))
    {
        return json_api_serializer_m;
    }

    std::string const message = std::string("Only \"") + json_api_content_type + "is supported.";
    throw http_exception(std::move(message), http_status_code::unsupported_media_type);
}

static bool is_type_accepted(char const * const content_type_part1,
                             char const * const content_type_part2,
                             request_i    const &request)
{
    bool const accepted = request.is_response_content_type_accepted(content_type_part1, content_type_part2);
    if(!accepted)
    {
        WC_DEBUG_LOG(std::string("Client does not accept content type \"") +
                                 content_type_part1 + "/" + content_type_part2 +
                                 "\"");
    }
    return accepted;
}

static bool match_base(std::string const &base_url,
                       std::string const &request_path)
{
    return (    (request_path.compare(0, base_url.length(), base_url) == 0)
             && (    (request_path.c_str()[base_url.length()] == '/')
                  || (request_path.c_str()[base_url.length()] == '?')
                  || (request_path.c_str()[base_url.length()] == '\0')));
}

static void extract_path_parameters(map<std::string, std::string>       &path_parameters,
                                    std::vector<std::string>      const &parameter_names,
                                    std::smatch                   const &parameter_result)
{
    auto parameter_iterator = parameter_result.begin();
    auto parameter_name_iterator = parameter_names.begin();
   
    ++parameter_iterator; // skip full match
    while(   (parameter_name_iterator != parameter_names.end())
          && (parameter_iterator      != parameter_result.end()))
    {
        WC_DEBUG_LOG("Got parameter for \"" + *parameter_name_iterator + "\" "
                     " from URL path: \"" + parameter_iterator->str() + "\"");
        path_parameters[*parameter_name_iterator] = parameter_iterator->str();
        ++parameter_name_iterator;
        ++parameter_iterator;
    }
}

static std::vector<std::string>extract_parameter_names(std::string const template_)
{
    std::vector<std::string> parameter_names;
    // Extract url parameters from template
    size_t marker_pos = 0;
    while((marker_pos = template_.find_first_of(http::parameter_markers, marker_pos)) != std::string::npos) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional assignment. Parasoft detects a false-positive here."
    {
        char const start_marker = template_.at(marker_pos);
        size_t const end_marker_pos = template_.find_first_of(start_marker, marker_pos + 1);
        if(end_marker_pos == std::string::npos)
        {
            throw std::runtime_error(std::string("Invalid URL template: Missing \'") + start_marker + "\' for parameter");
        }
        WC_ASSERT(end_marker_pos > marker_pos);
        std::string const parameter = template_.substr(marker_pos + 1, end_marker_pos - marker_pos - 1);
        WC_DEBUG_LOG(std::string("Found URL parameter: \"") + parameter + "\"");
        parameter_names.push_back(parameter);
        marker_pos = end_marker_pos + 1;
    }
    return parameter_names;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
