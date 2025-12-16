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
///  \brief    Test method_invocation_response object.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/response.hpp"
#include "rest/json_api.hpp"
#include "rest/basic_document.hpp"
#include "rest/method_invocation.hpp"
#include "rest/method_run_data.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::rest;
using std::string;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(method_invocation_response, BasicAttributes)
{
    auto                       const test_code = http_status_code::found;
    string                     const method_id = "123-456-some-method";
    string                     const id        = "0"; // must be "0" as its auto-incremented by an internal mechanisms.
    string                     const base_path = "/base/path/" + id;
    map<string, string>        const meta      = {};
    uint32_t                   const timeout   = 1234;
    method_run_data                  run_data(method_id, timeout, std::make_shared<wago::wdx::method_invocation_named_response>());
    method_invocation_resource       invocation("runs", id, run_data);
    method_invocation_document       test_invocation(base_path, "", meta, std::move(invocation));
    json_api test_serializer;

    string serialized_invocation;
    test_serializer.serialize(serialized_invocation, test_invocation);

    response<method_invocation_document> test_invocation_response(test_code, test_serializer, std::move(test_invocation));

    // Test abstract response
    response_i const * const response = &test_invocation_response;
    EXPECT_EQ(test_code, response->get_status_code());
    EXPECT_STREQ("application/vnd.api+json", response->get_content_type().c_str());
    EXPECT_STREQ(serialized_invocation.c_str(), response->get_content().c_str());
}


//---- End of source file ------------------------------------------------------
