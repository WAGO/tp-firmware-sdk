//------------------------------------------------------------------------------
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
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
#include "wago/wdx/linuxos/serv/parameter_service.hpp"
#include "mocks/mock_fcgi.hpp"
#include "mocks/mock_filesystem.hpp"
#include "mocks/mock_systemd.hpp"

#include <wago/wdx/test/wda_check.hpp>
#include <gtest/gtest.h>

#include <sstream>

#include <fcntl.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::linuxos::serv;
using namespace wago::wdx;
using std::vector;
using testing::Return;
using testing::AnyNumber;
using testing::AtLeast;
using testing::Invoke;
using testing::InvokeWithoutArgs;
using testing::WithArgs;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
#define EXPECT_STR_CONTAINS(string_to_search, content) \
    if(std::string(string_to_search).find(content) == std::string::npos) \
    FAIL() << "Expected \"" << (content) << "\" not contained in \"" << (string_to_search) << "\""

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const rest_api_service_base[] = "/test-url";
static constexpr char const file_api_service_base[] = "/test-url-2";
static constexpr char const rest_api_socket_path[]  = "/socketpath1";
static constexpr char const file_api_socket_path[]  = "/socketpath2";
static constexpr char const rest_api_socket_name[]  = "socket1";
static constexpr char const file_api_socket_name[]  = "socket2";
static constexpr char const doc_base_path[]         = "/doc/path";
static constexpr char const socket_user[]           = "www";
static constexpr char const socket_group[]          = "www";

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class ParameterService_Fixture_Base : public ::testing::Test
{
protected:
    mock_fcgi       fcgi_mock;
    mock_filesystem filesystem_mock;

    std::unique_ptr<parameter_service> service;

    std::string ordernumber      = "1234-5678";
    std::string firmware_version = "00.11.22";
private:
    std::string wdd_filename     = "1234-5678_00_11_22";

protected:
    ParameterService_Fixture_Base() = default;
    ~ParameterService_Fixture_Base() override = default;
    void SetUp() override
    {
        // Set default call expectations
        fcgi_mock.set_default_expectations();
        filesystem_mock.set_default_expectations();

        EXPECT_CALL(filesystem_mock, open_stream_proxy(::testing::StrEq("/etc/paramd/paramd.conf"), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(InvokeWithoutArgs([]() {
                    return new std::stringstream();
            }));

        // Will load main model file
        EXPECT_CALL(filesystem_mock, open_stream_proxy(::testing::EndsWith("/WAGO.bundle.wdm.json"), ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(Return(nullptr));

        // Will look for headstation device description
        EXPECT_CALL(filesystem_mock, is_file_existing(::testing::EndsWith("/" + wdd_filename + ".wdd.json")))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(false));
        EXPECT_CALL(filesystem_mock, is_file_existing(::testing::EndsWith("/" + ordernumber + ".wdd.json")))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(false));

        // Will try to find additional WDMs
        EXPECT_CALL(filesystem_mock, glob(::testing::EndsWith("/*.wdm.json")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(std::vector<std::string> { "/AdditionalModel.wdm.json" }));
        EXPECT_CALL(filesystem_mock, is_file_existing(::testing::Eq("/AdditionalModel.wdm.json")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));
        EXPECT_CALL(filesystem_mock, open_stream_proxy(::testing::Eq("/AdditionalModel.wdm.json"), ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(Return(nullptr));

        // Will try to find additional WDDs
        EXPECT_CALL(filesystem_mock, glob(::testing::EndsWith("/*.wdd.json")))
            .Times(Exactly(1))
            .WillRepeatedly(Return(std::vector<std::string> { "/AdditionalDeviceDescription.wdd.json" }));
        EXPECT_CALL(filesystem_mock, is_file_existing(::testing::Eq("/AdditionalDeviceDescription.wdd.json")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(true));
        EXPECT_CALL(filesystem_mock, open_stream_proxy(::testing::Eq("/AdditionalDeviceDescription.wdd.json"), ::testing::_))
            .Times(Exactly(1))
            .WillRepeatedly(Return(nullptr));
    }
};

#ifdef SYSTEMD_INTEGRATION
class ParameterService_Fixture_SystemdSocket : public ParameterService_Fixture_Base
{
protected:
    mock_systemd systemd_mock;

    ParameterService_Fixture_SystemdSocket() {}
    ~ParameterService_Fixture_SystemdSocket() override {}
    void SetUp() override
    {
        ParameterService_Fixture_Base::SetUp();
        systemd_mock.set_default_expectations();

        EXPECT_CALL(systemd_mock, find_systemd_socket(::testing::StrCaseEq(rest_api_socket_name)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(13));
        EXPECT_CALL(systemd_mock, find_systemd_socket(::testing::StrCaseEq(file_api_socket_name)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(12));

        EXPECT_CALL(filesystem_mock, close(::testing::_))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(0));

        service = std::make_unique<parameter_service>(ordernumber, firmware_version,
                                                      rest_api_service_base, rest_api_socket_name, 
                                                      file_api_service_base, file_api_socket_name,
                                                      doc_base_path);
    }
};

TEST_F(ParameterService_Fixture_SystemdSocket, ConstructDelete)
{
    // nothing to do
}

#endif // SYSTEMD_INTEGRATION

class ParameterService_Fixture_SocketCreation : public ParameterService_Fixture_Base
{
protected:
    ParameterService_Fixture_SocketCreation() {}
    ~ParameterService_Fixture_SocketCreation() override {}
    void SetUp() override
    {
        ParameterService_Fixture_Base::SetUp();

        EXPECT_CALL(fcgi_mock, OpenSocket(::testing::StrCaseEq(rest_api_socket_path), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
        EXPECT_CALL(fcgi_mock, OpenSocket(::testing::StrCaseEq(file_api_socket_path), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
        EXPECT_CALL(filesystem_mock, fcntl(::testing::_, F_SETFD, O_CLOEXEC))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(0));
        EXPECT_CALL(filesystem_mock, chown(::testing::StrCaseEq(rest_api_socket_path), ::testing::MatcherCast<char const*>(::testing::StrCaseEq(socket_user)), ::testing::StrCaseEq(socket_group)))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(0));
        EXPECT_CALL(filesystem_mock, chown(::testing::StrCaseEq(file_api_socket_path), ::testing::MatcherCast<char const*>(::testing::StrCaseEq(socket_user)), ::testing::StrCaseEq(socket_group)))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(0));
        EXPECT_CALL(filesystem_mock, close(::testing::_))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(0));

        service = std::make_unique<parameter_service>(ordernumber, firmware_version,
                                                      rest_api_service_base, rest_api_socket_path, 
                                                      file_api_service_base, file_api_socket_path,
                                                      doc_base_path);
    }
};

TEST_F(ParameterService_Fixture_SocketCreation, ConstructDelete)
{
    // nothing to do
}

TEST_F(ParameterService_Fixture_SocketCreation, ServiceIdentity)
{
    auto const & identity = service->get_service_identity();
    EXPECT_NE(nullptr,  identity.get_name());
    EXPECT_NE('\0',    *identity.get_name());
    EXPECT_NE(nullptr,  identity.get_version_string());
    EXPECT_NE('\0',    *identity.get_version_string());
    EXPECT_TRUE(       (identity.get_major_version()  != 0)
                    || (identity.get_minor_version()  != 0)
                    || (identity.get_bugfix_version() != 0));
    EXPECT_NE(nullptr,  identity.get_revision_string());

    EXPECT_STR_CONTAINS(identity.get_version_string(), std::to_string(identity.get_major_version()));
    EXPECT_STR_CONTAINS(identity.get_version_string(), std::to_string(identity.get_minor_version()));
    EXPECT_STR_CONTAINS(identity.get_version_string(), std::to_string(identity.get_bugfix_version()));
    EXPECT_STR_CONTAINS(identity.get_version_string(), std::to_string(identity.get_major_version()) + '.' +
                                                       std::to_string(identity.get_minor_version()) + '.' +
                                                       std::to_string(identity.get_bugfix_version()));
    EXPECT_STR_CONTAINS(identity.get_version_string(), identity.get_revision_string());
}

TEST_F(ParameterService_Fixture_SocketCreation, rest_api_server_turnaround)
{
    // Test settings
    char param_dummy[]        = "custom-param=test";
    char method[]             = "GET";

    // Set call expectations for test
    EXPECT_CALL(fcgi_mock, InitRequest(::testing::_, ::testing::_, ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&fcgi_mock, &mock_fcgi::InitRequest_followers)));
    EXPECT_CALL(fcgi_mock, Accept_r(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, GetParam(::testing::NotNull(), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(param_dummy));
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrCaseEq("REQUEST_METHOD"), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(method));
    char empty[] = "";
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_TYPE"), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(empty));
    EXPECT_CALL(fcgi_mock, GetLine(::testing::NotNull(), ::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(nullptr));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, FFlush(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));
    EXPECT_CALL(filesystem_mock, poll(::testing::_, ::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(WithArgs<0,1>(Invoke([](pollfd* fds, nfds_t nfds) {
            EXPECT_EQ(nfds, 1);
            fds->revents = POLLIN;
            return 0;
        })));

    // Test
    {
        auto &rest_api = service->get_rest_api_server();
        rest_api.receive_next();
    }
}

TEST_F(ParameterService_Fixture_SocketCreation, file_api_server_turnaround)
{
    // Test settings
    char param_dummy[]        = "custom-param=test";
    char method[]             = "GET";

    // Set call expectations for test
    EXPECT_CALL(fcgi_mock, InitRequest(::testing::_, ::testing::_, ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&fcgi_mock, &mock_fcgi::InitRequest_followers)));
    EXPECT_CALL(fcgi_mock, Accept_r(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, GetParam(::testing::NotNull(), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(param_dummy));
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrCaseEq("REQUEST_METHOD"), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(method));
    char empty[] = "";
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_TYPE"), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(empty));
    EXPECT_CALL(fcgi_mock, GetLine(::testing::NotNull(), ::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(nullptr));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, FFlush(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));
    EXPECT_CALL(filesystem_mock, poll(::testing::_, ::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(WithArgs<0,1>(Invoke([](pollfd* fds, nfds_t nfds) {
            EXPECT_EQ(nfds, 1);
            fds->revents = POLLIN;
            return 0;
        })));

    // Test
    {
        auto &rest_api = service->get_rest_api_server();
        rest_api.receive_next();
    }
}


//---- End of source file ------------------------------------------------------
