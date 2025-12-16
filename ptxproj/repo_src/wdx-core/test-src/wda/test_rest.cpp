//------------------------------------------------------------------------------
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
///  \brief    Test of rest frontend.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/rest_frontend.hpp"
#include "mocks/mock_service_identity.hpp"
#include "mocks/mock_settings_store.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_permissions.hpp"
#include "wago/wdx/unauthorized.hpp"
#include "parameter_service_core.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::rest::rest_frontend;
using wago::wdx::parameter_service_frontend_extended_i;
using wago::wdx::parameter_service_i;
using wago::wdx::parameter_service_core;
using wago::wdx::unauthorized;
using wago::wdx::user_permissions;
using std::string;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const service_base[] = "/test-url";
static constexpr char const doc_base[]     = "/test-doc";

//------------------------------------------------------------------------------
// fixture definition
//------------------------------------------------------------------------------
class rest_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_service_identity                 service_identity_mock;
    mock_permissions                     *permissions_mock;
    mock_settings_store                  *settings_store_mock;
    std::shared_ptr<mock_settings_store>  test_settings_store;
    std::shared_ptr<parameter_service_i>  test_core;

protected:
    rest_fixture() = default;
    ~rest_fixture() override = default;

    void SetUp() override
    {
        auto permissions_mock_ptr = std::make_unique<mock_permissions>();
        permissions_mock = permissions_mock_ptr.get();
        test_core = std::make_unique<parameter_service_core>(std::move(permissions_mock_ptr));
        test_settings_store = std::make_shared<mock_settings_store>();
        settings_store_mock = test_settings_store.get();
        ASSERT_NE(nullptr, settings_store_mock);

        // Set default call expectations
        service_identity_mock.set_default_expectations();
        settings_store_mock->set_default_expectations();
        EXPECT_CALL(*settings_store_mock, get_setting(::testing::StrEq(settings_store_i::run_result_timeout)))
            .Times(AnyNumber())
            .WillRepeatedly(Return("60"));
        permissions_mock->set_default_expectations();
    }
};

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(rest_fixture, ConstructDelete)
{
    // Test
    {
        auto null_core = unauthorized<parameter_service_frontend_extended_i>(nullptr);
        rest_frontend frontend(service_base, doc_base, service_identity_mock, test_settings_store, null_core);
    }
}

TEST_F(rest_fixture, HandleSimpleRequest)
{
    // Test settings
    char const service_name[] = "mocked_service";
    char const service_version[] = "0.1.2";
    char const request_uri[] = "/test-url/a/request/uri";
    char const request_content_type[] = "application/vnd.api+json";
    char const user_name[] = "test_user";

    // Prepare request mock
    auto request_mock = std::make_unique<mock_request>();

    // Set default call expectations
    request_mock->set_default_expectations();

    // We must provide some service identity information to
    // satisfy the handle call
    EXPECT_CALL(service_identity_mock, get_name())
        .Times(AnyNumber())
        .WillRepeatedly(Return(service_name));
    EXPECT_CALL(service_identity_mock, get_version_string())
        .Times(AnyNumber())
        .WillRepeatedly(Return(service_version));

    // Test specific
    EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Origin")))
        .Times(1)
        .WillRepeatedly(Return(""));
    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_uri));
    EXPECT_CALL(*request_mock, get_content_type())
            .Times(AnyNumber())
            .WillRepeatedly(Return(request_content_type));
    EXPECT_CALL(*request_mock, finish())
        .Times(Exactly(1));

    EXPECT_CALL(*request_mock, respond_mock(::testing::_))
        .Times(Exactly(1));

    user_permissions test_user_permissions(user_name, {}, {});
    EXPECT_CALL(*permissions_mock, get_user_permissions(::testing::StrEq(user_name)))
        .Times(AtMost(1))
        .WillRepeatedly(Return(test_user_permissions));

    // Test
    {
        auto unauthorized_core = unauthorized<parameter_service_frontend_extended_i>(test_core);
        rest_frontend frontend(service_base, doc_base, service_identity_mock, test_settings_store, unauthorized_core);
        frontend.handle(std::move(request_mock), {user_name});
    }
}


//---- End of source file ------------------------------------------------------
