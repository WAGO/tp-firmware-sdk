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
///  \brief    Test HTTP authentication layer "secure_handler"
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "http/secure_handler.hpp"
#include "http/head_response.hpp"

#include "mocks/mock_request.hpp"
#include "mocks/mock_request_handler.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::http;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class secure_handler_fixture: public ::testing::TestWithParam<http_status_code>
{
protected:
    std::unique_ptr<mock_request> request_mock = std::make_unique<mock_request>();

protected:
    virtual void SetUp() override
    {
        request_mock->set_default_expectations();
    }

    unique_ptr<request_handler_i> get_responding_handler(response_i const& response)
    {
        auto hdl = std::make_unique<mock_request_handler>();
        hdl->set_default_expectations();
        EXPECT_CALL(*hdl, handle_mock(::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(WithArgs<0>(Invoke([&response](request_i &request) {
                request.respond(response);
            })));
        return hdl;
    }
};

TEST_F(secure_handler_fixture, handle_default_request)
{
    head_response test_response(http_status_code::ok);
    secure_handler hdl(get_responding_handler(test_response));
    EXPECT_CALL(*request_mock, respond_mock(::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(WithArgs<0>(Invoke([&test_response](response_i const &response) {
                EXPECT_EQ(&response, &test_response);
            })));
    hdl.handle(std::move(request_mock));
}

TEST_F(secure_handler_fixture, handle_nonsecure_request)
{
    head_response test_response(http_status_code::ok);
    secure_handler hdl(get_responding_handler(test_response));
    EXPECT_CALL(*request_mock, is_https())
            .Times(AnyNumber())
            .WillRepeatedly(Return(false));
    EXPECT_CALL(*request_mock, respond_mock(::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(WithArgs<0>(Invoke([&test_response](response_i const &response) {
                EXPECT_NE(&response, &test_response);
                EXPECT_EQ(response.get_status_code(), http_status_code::upgrade_required);
            })));
    hdl.handle(std::move(request_mock));
}

//---- End of source file ------------------------------------------------------
