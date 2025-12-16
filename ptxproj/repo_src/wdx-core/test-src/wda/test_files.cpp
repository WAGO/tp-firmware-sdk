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
///  \brief    Test of files frontend.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "files/files_frontend.hpp"
#include "wago/wdx/wda/http/http_method.hpp"
#include "mocks/mock_permissions.hpp"
#include "wago/wdx/unauthorized.hpp"
#include "parameter_service_core.hpp"
#include "mocks/mock_request.hpp"
#include "mocks/mock_settings_store.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::parameter_service_i;
using wago::wdx::parameter_service_core;
using wago::wdx::unauthorized;
using wago::wdx::user_permissions;
using wago::wdx::parameter_service_file_api_i;
using wago::wdx::wda::files::files_frontend;
using std::string;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const service_base[] = "/test-url";

//------------------------------------------------------------------------------
// fixture definition
//------------------------------------------------------------------------------
class files_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_permissions                     *permissions_mock;
    std::shared_ptr<parameter_service_i>  test_core;

protected:
    files_fixture() = default;
    ~files_fixture() override = default;

    void SetUp() override
    {
        auto permissions_mock_ptr = std::make_unique<mock_permissions>();
        permissions_mock = permissions_mock_ptr.get();
        test_core = std::make_unique<parameter_service_core>(std::move(permissions_mock_ptr));
        permissions_mock->set_default_expectations();
    }
};

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(files, ConstructDelete)
{
    auto settings_store_mock = std::make_shared<mock_settings_store>();
    settings_store_mock->set_default_expectations();

    // Test
    {
        auto null_core = unauthorized<parameter_service_file_api_i>(nullptr);
        files_frontend frontend(string(service_base), settings_store_mock, null_core);
    }
}

TEST_F(files_fixture, HandleInvalidRequestUri)
{
    // Test settings
    char const request_uri[]    = "/test-url/";
    char const request_range[]  = "3-5";
    char const user_name[]     = "test_user";

    // Prepare mocks
    auto settings_store_mock = std::make_shared<mock_settings_store>();
    settings_store_mock->set_default_expectations();
    auto request_mock = std::make_unique<mock_request>();
    request_mock->set_default_expectations();
    EXPECT_CALL(*request_mock, get_http_header(::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    // Test specific
    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_uri));
    EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Range")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(request_range));
    EXPECT_CALL(*request_mock, add_response_header(::testing::StrEq("Allow"), ::testing::_))
        .Times(Exactly(1));
    EXPECT_CALL(*request_mock, respond_mock(::testing::_))
        .Times(Exactly(1));

    user_permissions test_user_permissions(user_name, {}, {});
    EXPECT_CALL(*permissions_mock, get_user_permissions(::testing::StrEq(user_name)))
        .Times(AtMost(1))
        .WillRepeatedly(Return(test_user_permissions));

    // Test
    {
        auto unauthorized_core = unauthorized<parameter_service_file_api_i>(test_core);
        files_frontend frontend(service_base, settings_store_mock, unauthorized_core);
        frontend.handle(std::move(request_mock), { "test_user" });
    }
}

TEST_F(files_fixture, HandleInvalidRequestMethod)
{
    // Test settings
    char const request_uri[]   = "/test-url/a/request/uri";
    char const request_range[] = "215343-5456415";
    char const user_name[]     = "test_user";

    // Prepare mocks
    auto settings_store_mock = std::make_shared<mock_settings_store>();
    settings_store_mock->set_default_expectations();
    auto request_mock = std::make_unique<mock_request>();
    request_mock->set_default_expectations();
    EXPECT_CALL(*request_mock, get_http_header(::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(""));

    // Test specific
    EXPECT_CALL(*request_mock, get_request_uri())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_uri));
    EXPECT_CALL(*request_mock, get_http_header(::testing::StrEq("Range")))
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_range));
    EXPECT_CALL(*request_mock, get_method())
        .Times(AnyNumber())
        .WillRepeatedly(Return((wago::wdx::wda::http::http_method)255));
    EXPECT_CALL(*request_mock, add_response_header(::testing::StrEq("Allow"), ::testing::_))
        .Times(Exactly(1));
    EXPECT_CALL(*request_mock, respond_mock(::testing::_))
        .Times(Exactly(1));

    user_permissions test_user_permissions(user_name, {}, {});
    EXPECT_CALL(*permissions_mock, get_user_permissions(::testing::StrEq(user_name)))
        .Times(AtMost(1))
        .WillRepeatedly(Return(test_user_permissions));

    // Test
    {
        auto unauthorized_core = unauthorized<parameter_service_file_api_i>(test_core);
        files_frontend frontend(service_base, settings_store_mock, unauthorized_core);
        frontend.handle(std::move(request_mock), {"test_user"});
    }
}


//---- End of source file ------------------------------------------------------
