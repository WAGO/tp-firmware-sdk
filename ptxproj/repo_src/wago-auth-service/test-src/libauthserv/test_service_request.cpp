//------------------------------------------------------------------------------
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
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
///  \brief    Test auth service request object.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/http_status_code.hpp"
#include "fcgi/service_request.hpp"
#include "mocks/mock_fcgi.hpp"
#include "mocks/mock_response.hpp"
#include "fail.hpp"

#include <wc/preprocessing.h>

#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::authserv;
using std::string;
using testing::AnyNumber;
using testing::AtLeast;
using testing::DoAll;
using testing::SetArrayArgument;
using testing::Return;
using testing::WithArgs;
using testing::Invoke;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class service_request_fixture : public ::testing::Test
{
public:
    mock_fcgi         fcgi_mock;
    int         const socket_number              = 55;
    std::string const request_path               = "/test";
    std::string const request_query_param        = "blub";
    std::string const request_query_param_value  = "test";
    std::string const request_query              = "?" + request_query_param + "=" + request_query_param_value;
    std::string const request_uri                = request_path + request_query;

    void SetUp() override
    {
        // Set call expectation which are already caused by construction of the service request
        fcgi_mock.set_default_expectations();
        EXPECT_CALL(fcgi_mock, InitRequest(::testing::NotNull(), socket_number, ::testing::_))
            .Times(AtLeast(1))
            .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&fcgi_mock, &mock_fcgi::InitRequest_followers)));
        EXPECT_CALL(fcgi_mock, Accept_r(::testing::NotNull()))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(0));
        EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("REQUEST_URI"), ::testing::_))
            .Times(AtLeast(1))
            .WillRepeatedly(Return(const_cast<char *>(request_uri.c_str())));
        EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_TYPE"), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(const_cast<char *>("application/x-www-form-urlencoded")));
        EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_LENGTH"), ::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(const_cast<char *>("0")));
        EXPECT_CALL(fcgi_mock, GetStr(nullptr, ::testing::_, ::testing::_))
            .Times(Exactly(0));
        EXPECT_CALL(fcgi_mock, GetStr(::testing::NotNull(), ::testing::_, nullptr))
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
    }
};

TEST(service_request, ConstructDelete)
{
    // Test settings
    int socket_number = 55;

    // Prepare mock
    mock_fcgi fcgi_mock;

    // Set call expectation
    fcgi_mock.set_default_expectations();
    EXPECT_CALL(fcgi_mock, InitRequest(::testing::NotNull(), socket_number, ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(WithArgs<0, 1, 2>(Invoke(&fcgi_mock, &mock_fcgi::InitRequest_followers)));

    // Test
    {
        fcgi::service_request request(socket_number);
    }
}

TEST_F(service_request_fixture, Accept)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
    }
}

TEST_F(service_request_fixture, RejectInvalidContentSize)
{
    // Test settings
    std::string test_content_length  = "123invalid";
    char expected_response_content[] = "Status: 400 Bad Request\r\n";

    // Prepare fcgi mock
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_LENGTH"), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(const_cast<char *>(test_content_length.c_str())));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::StrEq(expected_response_content), sizeof(expected_response_content) - 1, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, FFlush(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    // Test
    wago::authtest::add_expected_fail();
    {
        fcgi::service_request request(socket_number);
        EXPECT_FALSE(request.accept());
        EXPECT_TRUE(request.is_responded());
    }
}

TEST_F(service_request_fixture, RejectTooLargeContentSize)
{
    // Test settings
    std::string test_content_length  = std::to_string(fcgi::service_request::max_content_size + 1);
    char expected_response_content[] = "Status: 413 Payload Too Large\r\n";

    // Prepare fcgi mock
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_LENGTH"), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(const_cast<char *>(test_content_length.c_str())));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::StrEq(expected_response_content), sizeof(expected_response_content) - 1, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, FFlush(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    // Test
    wago::authtest::add_expected_fail();
    {
        fcgi::service_request request(socket_number);
        EXPECT_FALSE(request.accept());
        EXPECT_TRUE(request.is_responded());
    }
}

TEST_F(service_request_fixture, AcceptRespond)
{
    // Test settings
    char test_param[]                     = "test-param";
    char const test_param_name[]          = "custom-param";
    char test_request_content[]           = "request content";
    constexpr auto const test_status_code = http::http_status_code::ok;
    std::map<std::string,std::string> test_response_headers;
    char test_response_content[]          = "additional response content";
    std::string empty;

    // prepare http response mock
    mock_response response_mock;
    response_mock.set_default_expectations();
    EXPECT_CALL(response_mock, get_status_code())
        .Times(Exactly(1))
        .WillRepeatedly(Return(test_status_code));
    EXPECT_CALL(response_mock, get_response_header())
        .Times(Exactly(1))
        .WillRepeatedly(::testing::ReturnRef(test_response_headers));
    EXPECT_CALL(response_mock, get_content_type())
        .Times(AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(empty));
    EXPECT_CALL(response_mock, get_content_length())
        .Times(AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(empty));
    EXPECT_CALL(response_mock, get_content())
        .Times(AtLeast(1))
        .WillRepeatedly(::testing::Return(""));

    // Prepare fcgi mock
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("HTTP_CUSTOM-PARAM"), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_param));
    EXPECT_CALL(fcgi_mock, GetLine(nullptr, ::testing::_, ::testing::_))
        .Times(Exactly(0));
    EXPECT_CALL(fcgi_mock, GetLine(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AnyNumber())
        .WillOnce(Return(test_request_content))
        .WillRepeatedly(Return(nullptr));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::StrEq(test_response_content), sizeof(test_response_content), nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, FFlush(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        request.get_http_header(test_param_name);
        request.respond(response_mock);
        EXPECT_TRUE(request.is_responded());
        request.send_data(test_response_content, sizeof(test_response_content));
        request.finish();

        // Extra finish should fail
        wago::authtest::add_expected_fail();
        EXPECT_THROW(request.finish(), std::exception);
        wago::authtest::check_fail_count();
    }
}

TEST_F(service_request_fixture, AcceptRespondError)
{
    // Test settings
    char test_param[]                       = "test-param";
    char const test_param_name[]            = "custom-param";
    char test_request_content[]             = "request content";
    http_status_code const test_status_code = http_status_code::ok;
    std::map<std::string,std::string> test_response_headers;
    char test_response_content[]            = "additional response content";
    std::string empty;

    // prepare http response mock
    mock_response response_mock;
    response_mock.set_default_expectations();
    EXPECT_CALL(response_mock, get_status_code())
        .Times(Exactly(1))
        .WillRepeatedly(Return(test_status_code));
    EXPECT_CALL(response_mock, get_response_header())
        .Times(AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(test_response_headers));
    EXPECT_CALL(response_mock, get_content_type())
        .Times(AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(empty));
    EXPECT_CALL(response_mock, get_content_length())
        .Times(AnyNumber())
        .WillRepeatedly(::testing::ReturnRef(empty));
    EXPECT_CALL(response_mock, get_content())
        .Times(AnyNumber())
        .WillRepeatedly(::testing::Return(""));

    // Prepare fcgi mock
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("HTTP_CUSTOM-PARAM"), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(test_param));
    EXPECT_CALL(fcgi_mock, GetLine(nullptr, ::testing::_, ::testing::_))
        .Times(Exactly(0));
    EXPECT_CALL(fcgi_mock, GetLine(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AnyNumber())
        .WillOnce(Return(test_request_content))
        .WillRepeatedly(Return(nullptr));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::NotNull(), ::testing::_, nullptr))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(fcgi_mock, PutStr(::testing::StrEq(test_response_content), sizeof(test_response_content), nullptr))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, FFlush(::testing::NotNull()))
        .Times(AnyNumber())
        .WillRepeatedly(Return(0));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        request.get_http_header(test_param_name);
        wago::authtest::add_expected_fail();
        request.respond(response_mock);
        wago::authtest::check_fail_count(__FILE__ ": " WC_SUBST_STR(__LINE__));
        // FIXME: EXPECT_FALSE(request.is_responded());
        wago::authtest::add_expected_fail();
        EXPECT_THROW(request.send_data(test_response_content, sizeof(test_response_content)), std::exception);
        wago::authtest::check_fail_count(__FILE__ ": " WC_SUBST_STR(__LINE__));
    }
}

TEST_F(service_request_fixture, HTTPS)
{
    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("HTTPS"), ::testing::_))
        .Times(Exactly(2))
        .WillOnce(Return(const_cast<char *>("on")))
        .WillRepeatedly(Return(const_cast<char *>("off")));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE( request.accept());
        EXPECT_TRUE( request.is_https());
        EXPECT_FALSE(request.is_https());
    }
}

TEST_F(service_request_fixture, Method)
{
    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("REQUEST_METHOD"), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(const_cast<char *>("POST")));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(http::http_method::post, request.get_method());
    }
}

TEST_F(service_request_fixture, RequestURI)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(request_uri, request.get_request_uri().as_string());
    }
}

TEST_F(service_request_fixture, RequestPath)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(request_path, request.get_request_uri().get_path());
    }
}

TEST_F(service_request_fixture, RequestQuery)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(request_query, request.get_request_uri().get_query());
    }
}


TEST_F(service_request_fixture, RequestHasQueryParameter)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(true, request.has_query_parameter(request_query_param));
    }
}

TEST_F(service_request_fixture, RequestQueryParameter)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(request_query_param_value, request.get_query_parameter(request_query_param));
    }
}

TEST_F(service_request_fixture, HTTPHeader)
{
    std::string const http_header_value = "some value";

    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("HTTP_SOME_HEADER"), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(const_cast<char *>(http_header_value.c_str())));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(http_header_value, request.get_http_header("SOME_HEADER"));
    }
}

TEST_F(service_request_fixture, AcceptedType)
{
    std::string const accepted_type = "some-type/*";

    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("HTTP_ACCEPT"), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(const_cast<char *>(accepted_type.c_str())));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        EXPECT_EQ(accepted_type, request.get_accepted_types());
    }
}

TEST_F(service_request_fixture, GetBodyContent)
{
    // Test settings
    char   const request_param_name[] = "CONTENT_TYPE";
    char   const content_type[]       = "test/content";
    char   const content_length[]     = "21";
    char   const content[]            = "test-content to check";
    size_t const content_size         = sizeof(content);

    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq(request_param_name), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(const_cast<char *>(content_type)));
    EXPECT_CALL(fcgi_mock, GetStr(::testing::_, ::testing::Ge(32), ::testing::_))
        .Times(AtLeast(1))
        .WillOnce(DoAll(SetArrayArgument<0>(content, content + content_size), Return(content_size)))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_LENGTH"), ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(const_cast<char *>(content_length)));

    // Test
    {
        fcgi::service_request request(socket_number);
        request.accept();
        EXPECT_STREQ(content,        request.get_content().c_str());
        EXPECT_STREQ(content_type,   request.get_content_type().c_str());
        EXPECT_STREQ(content_length, request.get_content_length().c_str());
    }
}

TEST_F(service_request_fixture, GetBodyContentWithoutType)
{
    // Test settings
    char   const request_param_name[] = "CONTENT_TYPE";
    char   const * const content_type = nullptr;
    char   const content[]            = "test-content to check";
    size_t const content_size         = sizeof(content);

    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq(request_param_name), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(const_cast<char *>(content_type)));
    EXPECT_CALL(fcgi_mock, GetStr(::testing::_, ::testing::Ge(32), ::testing::_))
        .Times(AtLeast(1))
        .WillOnce(DoAll(SetArrayArgument<0>(content, content + content_size), Return(content_size)))
        .WillRepeatedly(Return(0));

    // Test
    {
        fcgi::service_request request(socket_number);
        request.accept();
        EXPECT_STREQ(content, request.get_content().c_str());
    }
}

TEST_F(service_request_fixture, GetContentStreamOnlyOnce)
{
    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_TYPE"), ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(const_cast<char *>("some/content-type")));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        request.get_content_stream();
        wago::authtest::add_expected_fail();
        EXPECT_THROW(request.get_content_stream(), std::exception);
        wago::authtest::check_fail_count(__FILE__ ": " WC_SUBST_STR(__LINE__));
        wago::authtest::add_expected_fail();
        EXPECT_THROW(request.get_content(), std::exception);
        wago::authtest::check_fail_count(__FILE__ ": " WC_SUBST_STR(__LINE__));
    }
}

TEST_F(service_request_fixture, SendDataInWrongState)
{
    char const some_data[] = "some data to send";

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        wago::authtest::add_expected_fail();
        EXPECT_THROW(request.send_data(some_data, sizeof(some_data)), std::exception);
        wago::authtest::check_fail_count(__FILE__ ": " WC_SUBST_STR(__LINE__));
    }
}

TEST_F(service_request_fixture, FinishInWrongState)
{
    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE(request.accept());
        wago::authtest::add_expected_fail();
        EXPECT_THROW(request.finish(), std::exception);
        wago::authtest::check_fail_count(__FILE__ ": " WC_SUBST_STR(__LINE__));
    }
}


// Following tests check functionality of interface extension
TEST_F(service_request_fixture, ContentTypeSet)
{
    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_TYPE"), ::testing::_))
        .Times(Exactly(3))
        .WillOnce(Return(const_cast<char *>("some-content-type")))
        .WillOnce(Return(const_cast<char *>("some-content-type")))
        .WillRepeatedly(Return(nullptr));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE( request.accept());
        EXPECT_TRUE( request.is_content_type_set());
        EXPECT_FALSE(request.is_content_type_set());
    }
}

TEST_F(service_request_fixture, AcceptedContentTypeMatch)
{
    // Test settings
    char const request_param_name[]       = "HTTP_ACCEPT";
    char const content_type_part1[]       = "test";
    char const content_type_part1_other[] = "not_requested";
    char const content_type_part2[]       = "content-type";
    char const content_type_part2_other[] = "wrong-type";
    string content_request_string(          "dummy/content,");
    content_request_string                = content_request_string + "test" +'/' + "content-type" + ";content-param=0.42";

    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq(request_param_name), ::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(const_cast<char *>(content_request_string.c_str())));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE( request.accept());
        EXPECT_TRUE( request.is_response_content_type_accepted(content_type_part1,       content_type_part2));
        EXPECT_FALSE(request.is_response_content_type_accepted(content_type_part1,       content_type_part2_other));
        EXPECT_FALSE(request.is_response_content_type_accepted(content_type_part1_other, content_type_part2));
    }
}

TEST_F(service_request_fixture, ContentTypeMatch)
{
    // Set call expectation
    EXPECT_CALL(fcgi_mock, GetParam(::testing::StrEq("CONTENT_TYPE"), ::testing::_))
        .Times(Exactly(4))
        .WillRepeatedly(Return(const_cast<char *>("some-type/sub-type")));

    // Test
    {
        fcgi::service_request request(socket_number);
        EXPECT_TRUE( request.accept());
        EXPECT_TRUE( request.is_content_type_matching("some-type", "sub-type"));
        EXPECT_FALSE(request.is_content_type_matching("some-other-type", "sub-type"));
        EXPECT_FALSE(request.is_content_type_matching("some-type", "sub-other-type"));
    }
}


//---- End of source file ------------------------------------------------------
