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
///  \brief    Implementation of wrapper for single resource objects delivered
///            by the REST-API
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_document.hpp"
#include "method_invocation.hpp"
#include "definitions.hpp"

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

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
template class basic_document<generic_resource>;
template class basic_document<service_identity_resource>;
template class basic_document<device_resource>;
template class basic_document<parameter_resource>;
template class basic_document<set_parameter_resource>;
template class basic_document<method_resource>;
template class basic_document<method_invocation_resource>;
template class basic_document<monitoring_list_resource>;
template class basic_document<enum_definition_resource>;
template class basic_document<feature_resource>;
template class basic_document<method_definition_resource>;
template class basic_document<method_arg_definition_resource>;
template class basic_document<parameter_definition_resource>;
template class basic_document<class_instance_resource>;

template <class T>
basic_document<T>::basic_document(map<string, string> const  &meta,
                                  T                   const &&data)
: basic_document("", "", meta, std::move(data))
{ }

template <class T>
basic_document<T>::basic_document(string const               &base_path,
                                  string const               &query,
                                  map<string, string> const  &meta,
                                  T      const              &&data)
: base_path_m(base_path)
, query_m(query)
, meta_m(meta)
, data_m(data)
{
    create_links();
}

template <class T>
T const basic_document<T>::get_data() const
{
    return data_m;
}

template <class T>
map<string, string> const & basic_document<T>::get_links() const
{
    return links_m;
}

template <class T>
bool basic_document<T>::has_links() const
{
    return !links_m.empty();
}

template <class T>
map<string, string> const & basic_document<T>::get_meta() const
{
    return meta_m;
}

template <class T>
bool basic_document<T>::has_meta() const
{
    return !meta_m.empty();
}

template <class T>
void basic_document<T>::create_links()
{
    if(!base_path_m.empty())
    {
        links_m.insert( { self_link_name, base_path_m + query_m } );
    }
}

template <class T>
vector<shared_ptr<core_error>> basic_document<T>::get_errors() const 
{
    return data_m.get_errors();
}

template <class T>
bool basic_document<T>::has_errors() const
{
    return data_m.has_errors();
}

template <>
template <>
bool monitoring_list_document::has_included_data<parameter_resource>() const
{
    return data_m.get_data().has_related_resources("parameters");
}

template <>
template <>
vector<parameter_resource> monitoring_list_document::get_included_data() const
{
    return data_m.get_data().get_related_resources("parameters");
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
