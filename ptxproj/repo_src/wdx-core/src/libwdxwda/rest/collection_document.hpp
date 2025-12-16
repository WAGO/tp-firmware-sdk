//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Paginated collection allowing paged REST-API results.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_COLLECTION_DOCUMENT_HPP_
#define SRC_LIBWDXWDA_REST_COLLECTION_DOCUMENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_resource.hpp"
#include "data_error.hpp"

#include <string>
#include <vector>
#include <map>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::string;
using std::vector;
using std::map;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

template <class T>
class collection_document
{
public:
    using wrapped_type = typename T::wrapped_type;

private:
    string              base_path_m;
    string              query_m;
    string              query_without_pagination_m;
    vector<T>           data_m;
    map<string, string> links_m;
    map<string, string> meta_m;
    vector<data_error>  errors_m;
    unsigned            page_offset_m;
    unsigned            page_limit_m;
    unsigned            page_element_max_m;

public:
    /// This constructor is meant to be used for responses that do not contain
    /// link information and pagination (e.g. responses to PATCH or POST requests).
    collection_document(map<string, string> const  &meta,
                        vector<T>           const &&data);

    /// This constructor is meant to be used for responses that do not contain
    /// link information and pagination (e.g. responses to PATCH or POST requests).
    collection_document(map<string, string>  const  &meta,
                        vector<wrapped_type> const &&data);

    /// This constructor is meant to be used for responses that contain
    /// link information and pagination (e.g. responses to GET requests).
    collection_document(string              const  &base_path,
                        string              const  &query,
                        map<string, string> const  &meta,
                        vector<T>           const &&data,
                        unsigned            const   page_offset,
                        unsigned            const   page_limit,
                        unsigned            const   page_element_max);

    /// This constructor is meant to be used for responses that contain
    /// link information and pagination (e.g. responses to GET requests).
    collection_document(string               const  &base_path,
                        string               const  &query,
                        map<string, string>  const  &meta,
                        vector<wrapped_type> const &&data,
                        unsigned             const   page_offset,
                        unsigned             const   page_limit,
                        unsigned             const   page_element_max);

    vector<T>           const   get_data()   const;
    bool                        has_links()  const;
    map<string, string> const & get_links()  const;
    bool                        has_errors() const;
    vector<data_error>  const & get_errors() const;
    
    map<string, string> const & get_meta() const;
    bool has_meta() const;

private:
    void         create_filtered_query();
    void         create_links();
    void         create_errors();
    string const get_self_link()          const;
    string const get_next_page_link()     const;
    string const get_previous_page_link() const;
    string const get_last_page_link()     const;
    string const get_first_page_link()    const;
};

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
using generic_collection_document               = collection_document<generic_resource>;
using device_collection_document                = collection_document<device_resource>;
using parameter_collection_document             = collection_document<parameter_resource>;
using set_parameter_collection_document         = collection_document<set_parameter_resource>;
using method_collection_document                = collection_document<method_resource>;
using method_invocation_collection_document     = collection_document<method_invocation_resource>;
using monitoring_list_collection_document       = collection_document<monitoring_list_resource>;
using enum_definition_collection_document       = collection_document<enum_definition_resource>;
using feature_collection_document               = collection_document<feature_resource>;
using method_definition_collection_document     = collection_document<method_definition_resource>;
using method_arg_definition_collection_document = collection_document<method_arg_definition_resource>;
using parameter_definition_collection_document  = collection_document<parameter_definition_resource>;
using class_instance_collection_document        = collection_document<class_instance_resource>;

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_COLLECTION_DOCUMENT_HPP_
//---- End of source file ------------------------------------------------------
