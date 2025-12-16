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
///  \brief    JSON:API serialization and deserialization.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_JSON_API_HPP_
#define SRC_LIBWDXWDA_REST_JSON_API_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/service_identity_i.hpp"
#include "method_invocation.hpp"
#include "serializer_i.hpp"
#include "deserializer_i.hpp"

#include <string>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::string;
using std::vector;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
class json_api : public serializer_i, public deserializer_i
{
private:
    string const content_type_m;

public:
    json_api();
    ~json_api() noexcept override;

    string const & get_content_type() const override;

    void serialize(string                           &out,
                   shared_ptr<http_exception> const  error) const override;

    void serialize(string                 &out,
                   generic_document const &resource) const override;

    void serialize(string                            &out,
                   generic_collection_document const &resource_collection) const override;

    void serialize(string                          &out,
                   service_identity_document const &service_identity) const override;

    void serialize(string                &out,
                   device_document const &resource) const override;

    void serialize(string                           &out,
                   device_collection_document const &device_responses) const override;

    void serialize(string                   &out,
                   parameter_document const &parameter_response) const override;

    void serialize(string                              &out,
                   parameter_collection_document const &parameter_responses) const override;

    void serialize(string                       &out,
                   set_parameter_document const &parameter_response) const override;

    void serialize(string                                  &out,
                   set_parameter_collection_document const &parameter_responses) const override;

    void serialize(string                &out,
                   method_document const &method_response) const override;

    void serialize(string                           &out,
                   method_collection_document const &method_responses) const override;

    void serialize(string                           &out,
                   method_invocation_document const &method_invocation_response) const override;
    
    void serialize(string                                      &out,
                   method_invocation_collection_document const &method_invocation_response) const override;

    void serialize(string                         &out,
                   monitoring_list_document const &monitoring_list_response) const override;

    void serialize(string                                    &out,
                   monitoring_list_collection_document const &monitoring_lists_response) const override;

    void serialize(string                         &out,
                   enum_definition_document const &enum_definition_response) const override;

    void serialize(string                                    &out,
                   enum_definition_collection_document const &enum_definitions_response) const override;

    void serialize(string                 &out,
                   feature_document const &feature_response) const override;

    void serialize(string                            &out,
                   feature_collection_document const &features_response) const override;

    void serialize(string                           &out,
                   method_definition_document const &method_def_response) const override;

    void serialize(string                                      &out,
                   method_definition_collection_document const &method_def_responses) const override;

    void serialize(string                               &out,
                   method_arg_definition_document const &arg_def_response) const override;

    void serialize(string                                          &out,
                   method_arg_definition_collection_document const &arg_def_responses) const override;

    void serialize(string                              &out,
                   parameter_definition_document const &parameter_definition_response) const override;

    void serialize(string                                         &out,
                   parameter_definition_collection_document const &parameter_definition_responses) const override;

    void serialize(string                        &out,
                   class_instance_document const &class_instance_response) const override;

    void serialize(string                                   &out,
                   class_instance_collection_document const &class_instance_responses) const override;
    
    void deserialize(method_invocation_in_args       &request,
                     string                    const &deserializable) const override;

    void deserialize(create_monitoring_list_request       &request,
                     string                         const &deserializable) const override;

    void deserialize(wdx::value_path_request       &parameter_value,
                     string                  const &deserializable) const override;

    void deserialize(std::vector<wdx::value_path_request>       &parameter_value,
                     string                               const &deserializable) const override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_JSON_API_HPP_
//---- End of source file ------------------------------------------------------
