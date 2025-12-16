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
///  \brief    Mock frontend operation.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_LIBWDA_MOCK_OPERATION_HPP_
#define TEST_INC_MOCKS_LIBWDA_MOCK_OPERATION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/response_i.hpp"
#include "rest/operation.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
class mock_operations;
extern mock_operations * current_operations_mock;

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::http::response_i;
using wago::wdx::wda::rest::operation_i;
using wago::wdx::wda::rest::request;
using wago::future;
using wago::resolved_future;
using std::unique_ptr;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
future<unique_ptr<response_i>> example_operation_1(operation_i * operation, std::shared_ptr<request> request_);
future<unique_ptr<response_i>> example_operation_2(operation_i * operation, std::shared_ptr<request> request_);

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_operations
{
public:
    mock_operations()
    {
        current_operations_mock = this;
    }
    virtual ~mock_operations()
    {
        current_operations_mock = nullptr;
    }
    
    virtual future<unique_ptr<response_i>> example_operation_1_mock(operation_i *, std::shared_ptr<request> request_)
    {
        return resolved_future(unique_ptr<response_i>(example_operation_1_mock_proxy(*request_)));
    }
    MOCK_METHOD1(example_operation_1_mock_proxy, response_i* (request &request_));

    virtual future<unique_ptr<response_i>> example_operation_2_mock(operation_i *, std::shared_ptr<request> request_)
    {
        return resolved_future(unique_ptr<response_i>(example_operation_2_mock_proxy(*request_)));
    }
    MOCK_METHOD1(example_operation_2_mock_proxy, response_i* (request &request_));

    void set_default_expectations()
    {
        EXPECT_CALL(*this, example_operation_1_mock_proxy(::testing::_))
            .Times(0);
        EXPECT_CALL(*this, example_operation_2_mock_proxy(::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_LIBWDA_MOCK_OPERATION_HPP_
//---- End of source file ------------------------------------------------------
