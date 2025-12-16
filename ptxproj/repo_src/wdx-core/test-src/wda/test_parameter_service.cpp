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
///  \brief    Test WAGO Parameter Service (basic behavior).
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/parameter_service.hpp"
#include "mocks/mock_settings_store.hpp"
#include "mocks/mock_password_backend.hpp"
#include "mocks/mock_permissions.hpp"

//#include <wago/wdx/test/wda_check.hpp>
#include <gtest/gtest.h>

//#include <sstream>

//#include <fcntl.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda;
using namespace wago::wdx;
using std::vector;
using testing::Return;
using testing::AnyNumber;
using testing::AtLeast;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::WithArgs;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const rest_api_service_base[] = "/test-url";
static constexpr char const file_api_service_base[] = "/test-url-2";
static constexpr char const doc_base_path[]         = "/doc/path";

class identity_provider : public wda::service_identity_i
{
    char const * get_id()              const override { return "test"; };
    char const * get_name()            const override { return "Test Name"; };
    char const * get_version_string()  const override { return "1.2.3-alpha.42"; };
    uint16_t     get_major_version()   const override { return 1; };
    uint16_t     get_minor_version()   const override { return 2; };
    uint16_t     get_bugfix_version()  const override { return 3; };
    char const * get_revision_string() const override { return "alpha.42"; };
};

static identity_provider const identity;

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class ParameterService_Fixture : public ::testing::Test
{
private:
    std::unique_ptr<mock_permissions> permissions_mock_ptr = std::make_unique<mock_permissions>();

protected:
    std::unique_ptr<parameter_service>      service;
    std::shared_ptr<mock_settings_store>    settings_store_mock;
    std::shared_ptr<mock_password_backend>  password_backend_mock;
    mock_permissions                       &permissions_mock = *permissions_mock_ptr;

    wdm_content_t wdm = "";
    wdm_content_t wdd = "";

protected:
    ParameterService_Fixture() = default;
    ~ParameterService_Fixture() override = default;
    void SetUp() override
    {
        settings_store_mock = std::make_shared<mock_settings_store>();
        settings_store_mock->set_default_expectations();

        password_backend_mock = std::make_shared<mock_password_backend>();
        password_backend_mock->set_default_expectations();

        permissions_mock.set_default_expectations();

        service = std::make_unique<parameter_service>(rest_api_service_base,
                                                      file_api_service_base,
                                                      doc_base_path,
                                                      identity,
                                                      settings_store_mock,
                                                      std::move(permissions_mock_ptr),
                                                      [wdm=wdm](){ return wdm; },
                                                      [wdd=wdd](std::string ordernumber, std::string firmware_version){ return wdd; },
                                                      password_backend_mock,
                                                      nullptr);
    }
};

TEST_F(ParameterService_Fixture, ConstructDelete)
{
    // nothing to do
}

TEST_F(ParameterService_Fixture, Cleanup)
{
    uint32_t const timeout_ms = 100;
    service->cleanup(timeout_ms);
}


//---- End of source file ------------------------------------------------------
