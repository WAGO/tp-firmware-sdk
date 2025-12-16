//------------------------------------------------------------------------------
// Copyright (c) 2024 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Most basic test for the serializer_i interface to be overridable 
///            as intended.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/serializer_i.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::rest;
using std::string;
using std::shared_ptr;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class example_serializer final : public serializer_i
{
private:
    string content_type_m = "text/plain";
public:
    string const & get_content_type() const override 
    {
        return content_type_m;
    }

    void serialize(string                          &,
                   shared_ptr<http_exception> const) const override
    { }

    void serialize(string                 &,
                   generic_document const &) const override
    { }

    void serialize(string                            &,
                   generic_collection_document const &) const override
    { }

    void serialize(string                          &,
                   service_identity_document const &) const override
    { }

    void serialize(string                           &,
                   device_collection_document const &) const override
    { }

    void serialize(string                &,
                   device_document const &) const override
    { }

    void serialize(string                   &,
                   parameter_document const &) const override
    { }

    void serialize(string                              &,
                   parameter_collection_document const &) const override
    { }

    void serialize(string                       &out,
                   set_parameter_document const &parameter_response) const override
    { }

    void serialize(string                                  &out,
                   set_parameter_collection_document const &parameter_responses) const override
    { }

    void serialize(string                &,
                   method_document const &) const override
    { }

    void serialize(string                           &,
                   method_collection_document const &) const override
    { }

    void serialize(string                           &,
                   method_invocation_document const &) const override
    { }

    void serialize(string                           &,
                   method_invocation_collection_document const &) const override
    { }

    void serialize(string                         &,
                   monitoring_list_document const &) const override
    { }

    void serialize(string                                    &,
                   monitoring_list_collection_document const &) const override
    { }

    void serialize(string                         &,
                   enum_definition_document const &) const override
    { }

    void serialize(string                                    &,
                   enum_definition_collection_document const &) const override
    { }

    void serialize(string                 &,
                   feature_document const &) const override
    { }

    void serialize(string                            &,
                   feature_collection_document const &) const override
    { }

    void serialize(string                           &,
                   method_definition_document const &) const override
    { }

    void serialize(string                                      &,
                   method_definition_collection_document const &) const override
    { }

    void serialize(string                           &,
                   method_arg_definition_document const &) const override
    { }

    void serialize(string                                      &,
                   method_arg_definition_collection_document const &) const override
    { }

    void serialize(string                              &,
                   parameter_definition_document const &) const override
    { }

    void serialize(string                                         &,
                   parameter_definition_collection_document const &) const override
    { }

    void serialize(string                        &,
                   class_instance_document const &) const override
    { }

    void serialize(string                                   &,
                   class_instance_collection_document const &) const override
    { }
};

TEST(example_serializer, construct_delete)
{
    example_serializer serializer;
}


//---- End of source file ------------------------------------------------------
