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
///  \brief    Test of authentication settings provider.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/rest_frontend.hpp"
#include "mocks/mock_service_identity.hpp"
#include "mocks/mock_settings_store.hpp"

#include <wago/wdx/test/fail.hpp>
#include "wago/wdx/unauthorized.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::wdx::wda::rest::rest_frontend;
using wago::wdx::parameter_service_frontend_extended_i;
using wago::wdx::unauthorized;
using std::string;
using testing::AtLeast;
using testing::AnyNumber;
using testing::Return;

//------------------------------------------------------------------------------
// fixture definition
//------------------------------------------------------------------------------
class auth_settings_provider_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_service_identity                 service_identity_mock;
    mock_settings_store                  *settings_store_mock;
    std::shared_ptr<mock_settings_store>  test_settings_store;

protected:
    auth_settings_provider_fixture() = default;
    ~auth_settings_provider_fixture() override = default;

    void SetUp() override
    {
        test_settings_store = std::make_shared<mock_settings_store>();
        settings_store_mock = test_settings_store.get();
        ASSERT_NE(nullptr, settings_store_mock);

        // Set default call expectations
        service_identity_mock.set_default_expectations();
        settings_store_mock->set_default_expectations();
        EXPECT_CALL(*settings_store_mock, get_setting(::testing::StrEq(settings_store_i::run_result_timeout)))
            .Times(AnyNumber())
            .WillRepeatedly(Return("60"));
    }
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const service_base[] = "/test-url";
static constexpr char const doc_base[]     = "/test-doc";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST_F(auth_settings_provider_fixture, scan_devices_allowed_unauthenticated)
{
    // Set call expectations
    EXPECT_CALL(*settings_store_mock, get_setting(::testing::StrEq(settings_store_i::allow_unauthenticated_requests_for_scan_devices)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return("true"));

    // Test
    {
        auto null_core = unauthorized<parameter_service_frontend_extended_i>(nullptr);
        rest_frontend frontend(service_base, doc_base, service_identity_mock, test_settings_store, null_core);
        EXPECT_NE("", frontend.get_unauthenticated_urls());
    }
}

TEST_F(auth_settings_provider_fixture, scan_devices_not_allowed_unauthenticated)
{
    // Set call expectations
    EXPECT_CALL(*settings_store_mock, get_setting(::testing::StrEq(settings_store_i::allow_unauthenticated_requests_for_scan_devices)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return("false"));

    // Test
    {
        auto null_core = unauthorized<parameter_service_frontend_extended_i>(nullptr);
        rest_frontend frontend(service_base, doc_base, service_identity_mock, test_settings_store, null_core);
        EXPECT_EQ("", frontend.get_unauthenticated_urls());
    }
}

TEST_F(auth_settings_provider_fixture, scan_devices_with_invalid_config_value)
{
    // Set call expectations
    EXPECT_CALL(*settings_store_mock, get_setting(::testing::StrEq(settings_store_i::allow_unauthenticated_requests_for_scan_devices)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return("invalid"));
    wago::wdx::test::add_expected_fail();

    // Test
    {
        auto null_core = unauthorized<parameter_service_frontend_extended_i>(nullptr);
        rest_frontend frontend(service_base, doc_base, service_identity_mock, test_settings_store, null_core);
        frontend.get_unauthenticated_urls();
    }
    wago::wdx::test::check_fail_count();
}


//---- End of source file ------------------------------------------------------
