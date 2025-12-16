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
///  \brief    Implementation of requests to the rest frontend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "request.hpp"
#include "json_api.hpp"
#include "http/http_exception.hpp"

#include "wda_ipc/representation.hpp"
#include <wc/log.h>

#include <sstream>
#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::stringstream;
using http::http_exception;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

request::request(std::unique_ptr<request_i>        original_request,
                 serializer_i               const &serializer,
                 deserializer_i             const &deserializer,
                 map<string, string>        const  path_parameters,
                 string                     const &doc_link)
: original_request_m(std::move(original_request))
, serializer_m(serializer)
, deserializer_m(deserializer)
, path_parameters_m(path_parameters)
, doc_link_m(doc_link)
{ }


bool request::is_https() const
{
    return original_request_m->is_https();
}

bool request::is_localhost() const
{
    return original_request_m->is_localhost();
}

http_method request::get_method() const
{
    return original_request_m->get_method();
}

http::uri request::get_request_uri() const
{
    return original_request_m->get_request_uri();
}

bool request::has_query_parameter(std::string const & name) const
{
    return original_request_m->has_query_parameter(name);
}

std::string request::get_query_parameter(std::string const & name, bool decode) const
{
    return original_request_m->get_query_parameter(name, decode);
}

bool request::has_http_header(std::string const &header_name) const
{
    return original_request_m->has_http_header(header_name);
}

std::string request::get_http_header(std::string const &header_name) const
{
    return original_request_m->get_http_header(header_name);
}

std::string request::get_accepted_types() const
{
    return original_request_m->get_accepted_types();
}

std::string request::get_content_type() const
{
    return original_request_m->get_content_type();
}

std::string request::get_content_length() const
{
    return original_request_m->get_content_length();
}

std::string request::get_content()
{
    return original_request_m->get_content();
}

std::istream &request::get_content_stream()
{
    return original_request_m->get_content_stream();
}

void request::add_response_header(std::string const &header_name,
                                  std::string const &header_value)
{
    return original_request_m->add_response_header(header_name, header_value);
}

void request::respond(response_i const &response) noexcept
{
    return original_request_m->respond(response);
}

bool request::is_responded() const
{
    return original_request_m->is_responded();
}

void request::send_data(char const *bytes, size_t size)
{
    return original_request_m->send_data(bytes, size);
}

void request::finish()
{
    original_request_m->finish();
}

string const &request::get_doc_link() const
{
    return doc_link_m;
}

bool request::has_path_parameter(string const &parameter_name) const
{
    return path_parameters_m.count(parameter_name) > 0;
}

std::string request::get_path_parameter(std::string const &parameter_name) const
{
    return path_parameters_m.at(parameter_name);
}

void request::get_include_parameters(vector<vector<string>> &relationship_paths) const
{
    
    if(has_query_parameter("include"))
    {
        auto parameter_value = get_query_parameter("include");
        stringstream value_ss(parameter_value);
        string path;
        while (std::getline(value_ss, path, ','))
        {
            string related;
            stringstream path_ss(path);
            vector<string> related_path;
            while (std::getline(path_ss, related, '.'))
            {
                related_path.push_back(related);
            }
            relationship_paths.push_back(related_path);
        }
    }
}

// ToDo: review data types of page_limit and page_offset: use size_t like in core frontend?
void request::get_pagination_parameters(unsigned &page_limit,
                                        unsigned &page_offset) const
{
    // Get optional pagination parameters, else leave the defaults
    page_limit  = page_limit_default;
    page_offset = page_offset_default;

    constexpr unsigned const page_max = std::numeric_limits<unsigned>::max() / 2;

    // Note: since ssize_t is used for std iterators differences, page_max must not exceed this limit 
    WC_STATIC_ASSERT( page_max <= std::numeric_limits<ssize_t>::max() );

    try
    {
        if(has_query_parameter(page_limit_query_key))
        {
            page_limit = stoul(get_query_parameter(page_limit_query_key));
        }
        if(has_query_parameter(page_offset_query_key))
        {
            page_offset = stoul(get_query_parameter(page_offset_query_key));
        }

        if(page_max < page_limit)
        {
            throw std::out_of_range("Value of page_limit too big");
        }
        if(page_max < page_offset)
        {
            throw std::out_of_range("Value of page_offset too big");
        }

        // Ńo overflow can happen, if page_max is half the value of its datatype or less
        WC_STATIC_ASSERT( (std::numeric_limits<decltype(page_max)>::max() / 2) >= page_max );
        if(page_max < (page_limit + page_offset))
        {
            page_limit = page_max - page_offset;
            wc_log(log_level_t::warning, std::string("Reduced query parameter \"") + page_limit_query_key + "\" to " +
                                         std::to_string(page_limit) + ", due to internal server limitations");
        }
    }
    catch(std::invalid_argument const &)
    {
        http_status_code const http_code = http_status_code::bad_request;
        throw http_exception("Invalid pagination parameter in query: value not valid", http_code);
    }
    catch(std::out_of_range const &)
    {
        http_status_code const http_code = http_status_code::bad_request;
        throw http_exception("Invalid pagination parameter in query: value out of range", http_code);
    }
}

bool request::get_errors_as_data_attributes_parameter() const
{
    return has_query_parameter(parameter_errors_as_data_attributes) &&
          (get_query_parameter(parameter_errors_as_data_attributes) == "true");
}

bool request::get_deferred_parameters_as_errors() const
{
    return has_query_parameter(deferred_parameters_as_errors) &&
          (get_query_parameter(deferred_parameters_as_errors) == "true");
}

bool request::get_adjusted_parameters_as_errors() const
{
    return has_query_parameter(adjusted_parameters_as_errors) &&
          (get_query_parameter(adjusted_parameters_as_errors) == "true");
}

result_behavior_types request::get_result_behavior() const
{
    auto chosen_result_behavior = result_behavior_types::any;
    if(has_query_parameter(result_behavior))
    {
        auto result_behavior_query = get_query_parameter(result_behavior);
        if(result_behavior_query == "sync")
        {
            chosen_result_behavior = result_behavior_types::sync;
        }
        else if(result_behavior_query == "async")
        {
            chosen_result_behavior = result_behavior_types::async;
        }
        else if(result_behavior_query == "auto")
        {
            chosen_result_behavior = result_behavior_types::automatic;
        }
        else
        {
            throw http_exception("Invalid value for query parameter \"result-behavior\". Allowed values are \"sync\", \"async\" and \"auto\".", http_status_code::bad_request);
        }
    }
    return chosen_result_behavior;
}

std::map<std::string, std::string> request::get_filter_queries() const
{
    std::map<std::string, std::string> filter_queries;

    std::string query_element;
    std::stringstream query(get_request_uri().get_query());

    if (query.get() != '?') {
        query.unget();
    }
    while(std::getline(query, query_element, '&'))
    {
        size_t param_name_start = 0;
        if (query_element.find("filter[") != param_name_start) {
            continue;
        }
        size_t param_name_end = query_element.find(']', 7);
        if (param_name_end == std::string::npos) {
            continue;
        }
        size_t param_name_length = (param_name_end - param_name_start) + 1;

        if (query_element[param_name_length] != '=') {
            continue;
        }
        auto filter_name  = query_element.substr(param_name_start + 7, param_name_length - 7 - 1);
        if (filter_name.empty())
        {
            continue;
        }
        auto filter_value = query_element.substr(param_name_end + 2);
        filter_queries.emplace(filter_name, filter_value);
    }

    return filter_queries;
}

serializer_i const &request::get_serializer() const
{
    return serializer_m;
}

deserializer_i const &request::get_deserializer() const
{
    return deserializer_m;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
