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
///  \brief    Implementation of paginated collection allowing paged REST-API
///            results.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "collection_document.hpp"
#include "definitions.hpp"

#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static void append_query_parameter_to_url(string       &url,
                                          string const &name,
                                          string const &value);

static string const create_pagination_link_url(string   const &base_url,
                                               string   const &query_without_pagination,
                                               unsigned const  limit_value,
                                               unsigned const  offset_value);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
static void append_query_parameter_to_url(string       &url,
                                          string const &name,
                                          string const &value)
{
    bool const is_first = (url.find('?') == string::npos);
    url += is_first ? '?' : query_separator;
    url += name + "=" + value;
}

static string const create_pagination_link_url(string   const &base_url,
                                               string   const &query_without_pagination,
                                               unsigned const  limit_value,
                                               unsigned const  offset_value)
{
    string link = base_url + (query_without_pagination.empty() ? "" : query_without_pagination);
    append_query_parameter_to_url(link, page_limit_query_key, std::to_string(limit_value));
    append_query_parameter_to_url(link, page_offset_query_key, std::to_string(offset_value));
    return link;
}

template<class wrapped_type>
static vector<basic_resource<wrapped_type>> resources_from_wrapped_types(vector<wrapped_type> const &wrapped_objects)
{
    vector<basic_resource<wrapped_type>> resources;
    for(auto &object : wrapped_objects)
    {
        resources.emplace_back(object);
    }
    return resources;
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
template class collection_document<generic_resource>;
template class collection_document<device_resource>;
template class collection_document<parameter_resource>;
template class collection_document<set_parameter_resource>;
template class collection_document<method_resource>;
template class collection_document<method_invocation_resource>;
template class collection_document<monitoring_list_resource>;
template class collection_document<enum_definition_resource>;
template class collection_document<feature_resource>;
template class collection_document<method_definition_resource>;
template class collection_document<method_arg_definition_resource>;
template class collection_document<parameter_definition_resource>;
template class collection_document<class_instance_resource>;

template <class T>
collection_document<T>::collection_document(map<string, string> const  &meta,
                                            vector<T>           const &&data)
: base_path_m("")
, query_m("")
, data_m(data)
, meta_m(meta)
, page_offset_m(0)
, page_limit_m(0)
, page_element_max_m(0)
{
    // Dont create links and self link in this case. 
    // Constructor is meant to be used for responses that do not contain
    // such information (e.g. responses to PATCH or POST requests)
    create_errors();
}

template <class T>
collection_document<T>::collection_document(map<string, string>  const  &meta,
                                            vector<wrapped_type> const &&data)
: collection_document(meta, resources_from_wrapped_types(data))
{ }

template <class T>
collection_document<T>::collection_document(string              const  &base_path,
                                            string              const  &query,
                                            map<string, string> const  &meta,
                                            vector<T>           const &&data,
                                            unsigned            const   page_offset,
                                            unsigned            const   page_limit,
                                            unsigned            const   page_element_max)
: base_path_m(base_path)
, query_m(query)
, data_m(data)
, meta_m(meta)
, page_offset_m(page_offset)
, page_limit_m(page_limit)
, page_element_max_m(page_element_max)
{
    create_filtered_query();
    // May include applied default parameters (e.g. pagination) in meta area like done on
    // https://jsonapi.org/examples/#pagination
    create_links();
    create_errors();
}

template <class T>
collection_document<T>::collection_document(string               const  &base_path,
                                            string               const  &query,
                                            map<string, string>  const  &meta,
                                            vector<wrapped_type> const &&data,
                                            unsigned             const   page_offset,
                                            unsigned             const   page_limit,
                                            unsigned             const   page_element_max)
: collection_document(base_path, query, meta, resources_from_wrapped_types(data), page_offset, page_limit, page_element_max)
{ }

template <class T>
vector<T> const collection_document<T>::get_data() const
{
    return data_m;
}

template <class T>
bool collection_document<T>::has_links() const
{
    return !links_m.empty();
}

template <class T>
map<string, string> const & collection_document<T>::get_links() const
{
    return links_m;
}

template <class T>
bool collection_document<T>::has_errors() const
{
    return !errors_m.empty();
}

template <class T>
vector<data_error> const & collection_document<T>::get_errors() const
{
    return errors_m;
}

template <class T>
map<string, string> const & collection_document<T>::get_meta() const
{
    return meta_m;
}

template <class T>
bool collection_document<T>::has_meta() const
{
    return !meta_m.empty();
}

template <class T>
void collection_document<T>::create_filtered_query()
{
    WC_ASSERT_RETURN_VOID(query_m.empty() || (query_m[0] == '?'));
    WC_STATIC_ASSERT(page_common_start_base[0] != '?');

    // Filter pagination query parameters
    size_t next_data_pos = 0;
    size_t page_start_pos;
    while((page_start_pos = query_m.find(page_common_start_base, next_data_pos)) != std::string::npos) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional Assignment. Parasoft has a bug here."
    {
        WC_ASSERT(page_start_pos > 0);
        WC_ASSERT_RETURN_VOID(next_data_pos < page_start_pos);
        query_without_pagination_m += query_m.substr(next_data_pos, page_start_pos - next_data_pos - 1);

        size_t const page_end_pos = query_m.find('&', page_start_pos);
        if(page_end_pos == std::string::npos)
        {
            // Query string invalid or this pagination parameter was the last query parameter
            next_data_pos = query_m.length();
            break;
        }
        next_data_pos = page_end_pos;
    }
    query_without_pagination_m += query_m.substr(next_data_pos, query_m.length() - next_data_pos);
    if(!query_without_pagination_m.empty() && query_without_pagination_m[0] == '&')
    {
        query_without_pagination_m[0] = '?';
    }
}

template <class T>
void collection_document<T>::create_links()
{
    if(links_m.empty())
    {
        links_m.insert( { self_link_name,       get_self_link()       } );
        links_m.insert( { first_page_link_name, get_first_page_link() } );
        links_m.insert( { last_page_link_name,  get_last_page_link()  } );

        string const next_page_link = get_next_page_link();
        if(!next_page_link.empty())
        {
            links_m.insert( { next_page_link_name, next_page_link } );
        }
        string const previous_page_link = get_previous_page_link();
        if(!previous_page_link.empty())
        {
            links_m.insert( { previous_page_link_name, previous_page_link } );
        }
    }
}

template <class T>
string const collection_document<T>::get_self_link() const
{
    // Include each (query) parameter, even applied default values as suggested in https://stackoverflow.com/a/37454140
    return create_pagination_link_url(base_path_m, query_without_pagination_m, page_limit_m, page_offset_m);
}

template <class T>
string const collection_document<T>::get_next_page_link() const
{
    string next_link; // empty by default
    unsigned const next_page_offset = page_offset_m + page_limit_m;
    if(page_element_max_m > next_page_offset)
    {
        next_link = create_pagination_link_url(base_path_m, query_without_pagination_m, page_limit_m, next_page_offset);
    }
    return next_link;
}

template <class T>
string const collection_document<T>::get_previous_page_link() const
{
    string prev_link; // empty by default
    if(page_offset_m > 0)
    {
        // offset of previous page is capped to 0
        unsigned const prev_page_offset = (page_limit_m > page_offset_m)
            ? 0
            : page_offset_m - page_limit_m;

        prev_link = create_pagination_link_url(base_path_m, query_without_pagination_m, page_limit_m, prev_page_offset);
    }
    return prev_link;
}

template <class T>
string const collection_document<T>::get_last_page_link() const
{
    unsigned last_page_offset = (page_element_max_m > page_limit_m) ? page_offset_m : 0;
    while( (page_limit_m > 0) && (page_element_max_m > (last_page_offset + page_limit_m)) )
    {
        last_page_offset += page_limit_m;
    }
    return create_pagination_link_url(base_path_m, query_without_pagination_m, page_limit_m, last_page_offset);
}

template <class T>
string const collection_document<T>::get_first_page_link() const
{
    return create_pagination_link_url(base_path_m, query_without_pagination_m, page_limit_m, 0);
}

template <class T>
void collection_document<T>::create_errors()
{ }

template <>
void parameter_collection_document::create_errors()
{
    if(errors_m.empty())
    {
        for(parameter_resource const &resource : get_data())
        {
            auto data = resource.get_data();
            if(data.has_error())
            {
                errors_m.push_back(data_error(data, data.path));
            }
        }
    }
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
