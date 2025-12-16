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
///  \brief    Container for a generic basic resource.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_BASIC_RESOURCE_HPP_
#define SRC_LIBWDXWDA_REST_BASIC_RESOURCE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_relationship.hpp"
#include "monitoring_list_data.hpp"
#include "method_run_data.hpp"
#include "parameter_definition_data.hpp"
#include "class_instance_data.hpp"
#include "wago/wdx/wda/service_identity_i.hpp"
#include "method_response.hpp"
#include "method_definition_data.hpp"
#include "method_arg_definition_data.hpp"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <cstddef>

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

template <class WrappedType>
class basic_resource
{
public: 
    using wrapped_type = WrappedType;

private:
    string                          type_m;
    string                          id_m;
    wrapped_type                    data_m;
    map<string, basic_relationship> relationships_m;
    map<string, string>             links_m;
public:
    basic_resource(string                          const &type,
                   string                          const &id,
                   wrapped_type                    const &data,
                   map<string, basic_relationship> const &relationships = {},
                   map<string, string>             const &data_links = {});

    template<class IdType>
    basic_resource(IdType                          const &id_type,
                   wrapped_type                    const &data,
                   map<string, basic_relationship> const &relationships = {},
                   map<string, string>             const &data_links = {});

    explicit basic_resource(wrapped_type                    const &data,
                            map<string, basic_relationship> const &relationships = {},
                            map<string, string>             const &data_links = {});

    string const & get_type() const;
    string const & get_id()   const;
    wrapped_type const & get_data() const;
    bool has_relationships() const;
    map<string, basic_relationship> const & get_relationships() const;
    bool has_links() const;
    map<string, string> const & get_links() const;
    bool has_errors() const;
    vector<shared_ptr<core_error>> get_errors() const;
    bool has_error_like_meta() const;
    map<string, shared_ptr<core_error>> get_error_like_meta() const;
};

using generic_resource               = basic_resource<std::nullptr_t>;
using service_identity_resource      = basic_resource<service_identity_i const *>;
using device_resource                = basic_resource<wdx::device_response>;
using parameter_resource             = basic_resource<wdx::parameter_response>;
using set_parameter_resource         = basic_resource<wdx::set_parameter_response>;
using method_resource                = basic_resource<method_response>;
using method_invocation_resource     = basic_resource<method_run_data>;
using monitoring_list_resource       = basic_resource<monitoring_list_data>;
using enum_definition_resource       = basic_resource<wdx::enum_definition_response>;
using feature_resource               = basic_resource<wdx::feature_response>;
using method_definition_resource     = basic_resource<method_definition_data>;
using method_arg_definition_resource = basic_resource<method_arg_definition_data>;
using parameter_definition_resource  = basic_resource<parameter_definition_data>;
using class_instance_resource        = basic_resource<class_instance_data>;

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


#endif // SRC_LIBWDXWDA_REST_BASIC_RESOURCE_HPP_
//---- End of source file ------------------------------------------------------
