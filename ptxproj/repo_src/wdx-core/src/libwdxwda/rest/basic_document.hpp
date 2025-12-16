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
///  \brief    Wrapper for single resource objects delivered by the REST-API
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_BASIC_DOCUMENT_HPP_
#define SRC_LIBWDXWDA_REST_BASIC_DOCUMENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_resource.hpp"

#include <wc/compiler.h>

#include <string>
#include <map>
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::string;
using std::map;
using std::vector;
using std::shared_ptr;

class core_error;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

template <class T>
class basic_document
{
private:
    string              const base_path_m;
    string              const query_m;
    map<string, string>       links_m;
    map<string, string>       meta_m;
    T                   const data_m;

public:

    /// This constructor is meant to be used for responses that do not contain
    /// link information (e.g. responses to PATCH or POST requests).
    basic_document(map<string, string> const  &meta,
                   T                   const &&data);

    /// This constructor is meant to be used for responses that contain
    /// link information (e.g. responses to GET requests).
    basic_document(string              const  &base_path,
                   string              const  &query,
                   map<string, string> const  &meta,
                   T                   const &&data);
 
    T                   const   get_data()  const;
    map<string, string> const & get_links() const;
    bool                        has_links() const;
    map<string, string> const & get_meta() const;
    bool                        has_meta() const;

    vector<shared_ptr<core_error>> get_errors() const;
    bool                           has_errors() const;

    template <class U>
    bool      has_included_data() const;
    template <class U>
    vector<U> get_included_data() const;

private:
    void         create_links();
};

using generic_document               = basic_document<generic_resource>;
using service_identity_document      = basic_document<service_identity_resource>;
using device_document                = basic_document<device_resource>;
using parameter_document             = basic_document<parameter_resource>;
using set_parameter_document         = basic_document<set_parameter_resource>;
using method_document                = basic_document<method_resource>;
using method_invocation_document     = basic_document<method_invocation_resource>;
using monitoring_list_document       = basic_document<monitoring_list_resource>;
using enum_definition_document       = basic_document<enum_definition_resource>;
using feature_document               = basic_document<feature_resource>;
using method_definition_document     = basic_document<method_definition_resource>;
using method_arg_definition_document = basic_document<method_arg_definition_resource>;
using parameter_definition_document  = basic_document<parameter_definition_resource>;
using class_instance_document        = basic_document<class_instance_resource>;

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_BASIC_DOCUMENT_HPP_
//---- End of source file ------------------------------------------------------
