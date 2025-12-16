//------------------------------------------------------------------------------
// Copyright (c) 2025 WAGO GmbH & Co. KG
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
///  \brief    Mock for authenticated request handler.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_INC_MOCKS_MOCK_AUTHENTICATED_REQUEST_HANDLER_HPP_
#define TEST_INC_MOCKS_MOCK_AUTHENTICATED_REQUEST_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "auth/authenticated_request_handler_i.hpp"
#include "wago/wdx/wda/http/request_i.hpp"

#include <wc/compiler.h>
#include <wc/structuring.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::auth::authenticated_request_handler_i;
using wago::wdx::wda::auth::authentication_info;
using wago::wdx::wda::http::request_i;

//------------------------------------------------------------------------------
// mock definition
//------------------------------------------------------------------------------
GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
class mock_authenticated_request_handler: public authenticated_request_handler_i
{
public:
    ~mock_authenticated_request_handler() override = default;

    // Workaround: gmock does not allow to check unique_ptr
    MOCK_METHOD(void, handle_mock, (request_i &request, authentication_info auth_info), (noexcept));
    void handle(std::unique_ptr<request_i> request, authentication_info auth_info) noexcept override
    {
        handle_mock(*request.get(), auth_info);
    }

    void set_default_expectations()
    {
        EXPECT_CALL(*this, handle_mock(::testing::_, ::testing::_))
            .Times(0);
    }
};
GNUC_DIAGNOSTIC_POP


#endif // TEST_INC_MOCKS_MOCK_AUTHENTICATED_REQUEST_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
