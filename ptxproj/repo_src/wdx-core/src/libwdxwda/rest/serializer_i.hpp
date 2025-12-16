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
///  \brief    Serialization and deserialization interface
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_SERIALIZER_I_HPP_
#define SRC_LIBWDXWDA_REST_SERIALIZER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/service_identity_i.hpp"
#include "core_exception.hpp"
#include "basic_resource.hpp"
#include "collection_document.hpp"
#include "basic_document.hpp"
#include "method_invocation.hpp"

#include <wc/structuring.h>

#include <vector>
#include <string>
#include <memory>


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::vector;
using std::string;
using std::shared_ptr;
using http::http_exception;

class serializer_i
{
    WC_INTERFACE_CLASS(serializer_i)

public:
    virtual string const & get_content_type() const = 0;

    virtual void serialize(string                           &out,
                           shared_ptr<http_exception> const  error) const = 0;

    virtual void serialize(string                 &out,
                           generic_document const &resource) const = 0;

    virtual void serialize(string                            &out,
                           generic_collection_document const &resource_collection) const = 0;

    virtual void serialize(string                          &out,
                           service_identity_document const &service_identity) const = 0;

    virtual void serialize(string                           &out,
                           device_collection_document const &device_responses) const = 0;

    virtual void serialize(string                &out,
                           device_document const &device_response) const = 0;

    virtual void serialize(string                   &out,
                           parameter_document const &parameter_response) const = 0;

    virtual void serialize(string                              &out,
                           parameter_collection_document const &parameter_responses) const = 0;

    virtual void serialize(string                       &out,
                           set_parameter_document const &parameter_response) const = 0;

    virtual void serialize(string                                  &out,
                           set_parameter_collection_document const &parameter_responses) const = 0;

    virtual void serialize(string                &out,
                           method_document const &method_response) const = 0;

    virtual void serialize(string                           &out,
                           method_collection_document const &method_responses) const = 0;

    virtual void serialize(string                           &out,
                           method_invocation_document const &method_invocation_response) const = 0;

    virtual void serialize(string                                      &out,
                           method_invocation_collection_document const &method_invocation_response) const = 0;

    virtual void serialize(string                         &out,
                           monitoring_list_document const &monitoring_list_response) const = 0;

    virtual void serialize(string                                    &out,
                           monitoring_list_collection_document const &monitoring_lists_response) const = 0;

    virtual void serialize(string                         &out,
                           enum_definition_document const &enum_definition_response) const = 0;

    virtual void serialize(string                                    &out,
                           enum_definition_collection_document const &enum_definitions_response) const = 0;
    virtual void serialize(string                 &out,
                           feature_document const &feature_response) const = 0;

    virtual void serialize(string                            &out,
                           feature_collection_document const &features_response) const = 0;
    virtual void serialize(string                           &out,
                           method_definition_document const &method_def_response) const = 0;

    virtual void serialize(string                                      &out,
                           method_definition_collection_document const &method_def_responses) const = 0;

    virtual void serialize(string                               &out,
                           method_arg_definition_document const &arg_def_response) const = 0;

    virtual void serialize(string                                          &out,
                           method_arg_definition_collection_document const &arg_def_responses) const = 0;
    virtual void serialize(string                              &out,
                           parameter_definition_document const &parameter_definition_response) const = 0;

    virtual void serialize(string                                         &out,
                           parameter_definition_collection_document const &parameter_definition_responses) const = 0;

    virtual void serialize(string                        &out,
                           class_instance_document const &class_instance_response) const = 0;

    virtual void serialize(string                                   &out,
                           class_instance_collection_document const &class_instance_responses) const = 0;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_SERIALIZER_I_HPP_
//---- End of source file ------------------------------------------------------
