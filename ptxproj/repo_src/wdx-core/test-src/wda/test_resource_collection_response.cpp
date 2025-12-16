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
///  \brief    Test generic_collection_response object.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/response.hpp"
#include "rest/json_api.hpp"
#include "rest/collection_document.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::rest;
using std::string;
using std::vector;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(generic_collection_response, BasicAttributes)
{
    auto                     const test_status_code = http_status_code::found;
    string                   const base_path = "/base/path";
    map<string, string>      const meta = { {"test", "value"} };
    unsigned                 const page_limit = 8;
    unsigned                 const page_offset = 2;
    string                   const resource_type = "test-resource";
    generic_resource         const res_1 = {resource_type,  "0", nullptr, {}, {{"self", "/self/link"}} };
    generic_resource         const res_2 = {resource_type, "53", nullptr, {}, {{"self", "/self/link"}} };
    vector<generic_resource> const res = { res_1, res_2 };
    auto test_collection = generic_collection_document(base_path, "", meta, vector<generic_resource>(res), page_offset, page_limit, res.size());
    json_api test_serializer;

    string serialized_collection;
    test_serializer.serialize(serialized_collection, test_collection);

    response<generic_collection_document> test_collection_response(test_status_code, test_serializer, std::move(test_collection));

    // Test abstract response
    response_i const * const response = &test_collection_response;
    EXPECT_EQ(test_status_code, response->get_status_code());
    EXPECT_STREQ("application/vnd.api+json", response->get_content_type().c_str());
    EXPECT_STREQ(serialized_collection.c_str(), response->get_content().c_str());
}


//---- End of source file ------------------------------------------------------
