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
///  \brief    Test of configurable request router.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/router.hpp"
#include "wago/wdx/wda/http/http_method.hpp"
#include "mocks/mock_service_identity.hpp"
#include "mocks/mock_request.hpp"
#include "mocks_libwda/mock_operation.hpp"

#include <gtest/gtest.h>

#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::rest;
using namespace wago::wdx;
using std::shared_ptr;
using std::string;

typedef parameter_service_frontend_extended_i dummy_core_frontend;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class router_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_service_identity           service_identity_mock;
    unique_ptr<mock_request>        request_mock_to_move = std::make_unique<mock_request>();
    mock_request                   &request_mock = *request_mock_to_move.get();
    mock_operations                 operations_mock;

    // test specifica
    string base_url    = "wda";
    string request_uri;
    string default_request_content_type = "application/vnd.api+json";

protected:
    router_fixture() = default;
    ~router_fixture() override = default;

    void SetUp() override
    {
        // Set default call expectations
        service_identity_mock.set_default_expectations();
        request_mock.set_default_expectations();
        operations_mock.set_default_expectations();

        // Provide some service identity information
        char const service_name[]    = "mocked_service";
        char const service_version[] = "0.1.2";
        EXPECT_CALL(service_identity_mock, get_name())
            .Times(AnyNumber())
            .WillRepeatedly(Return(service_name));
        EXPECT_CALL(service_identity_mock, get_version_string())
            .Times(AnyNumber())
            .WillRepeatedly(Return(service_version));
        EXPECT_CALL(service_identity_mock, get_major_version())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));
        EXPECT_CALL(service_identity_mock, get_minor_version())
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));
        EXPECT_CALL(service_identity_mock, get_bugfix_version())
            .Times(AnyNumber())
            .WillRepeatedly(Return(2));

        // content type should return a valid value, when called
        EXPECT_CALL(request_mock, get_content_type())
            .Times(AnyNumber())
            .WillRepeatedly(Return(default_request_content_type.c_str()));
    }

    void set_request_uri(string const &new_request_uri)
    {
        request_uri = new_request_uri;
        EXPECT_CALL(request_mock, get_request_uri())
            .Times(AnyNumber())
            .WillRepeatedly(Return(request_uri));
    }
};

static void check_handler(router::route_result const &router_result,
                          operation_handler_t const &handler,
                          operation_handler_raw_t * const raw_operation)
{
    EXPECT_NE(nullptr, *router_result.handler_m.target<operation_handler_raw_t*>());
    EXPECT_EQ(*handler.target<operation_handler_raw_t*>(), *router_result.handler_m.target<operation_handler_raw_t*>());
    EXPECT_EQ(raw_operation, *router_result.handler_m.target<operation_handler_raw_t*>());
}

TEST_F(router_fixture, construct_delete)
{
    router the_router(service_identity_mock, base_url);
}

TEST_F(router_fixture, simple_route)
{
    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const url_template = "/blub/123";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + url_template);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, route_with_doc_link)
{
    // Test settings: Route setup
    http_method         const method        = http_method::get;
    string              const url_template  = "/blub/123";
    string              const test_doc_link = "/test/doc";
    operation_handler_t const handler       = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + url_template);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler, test_doc_link);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        EXPECT_EQ(test_doc_link, router_result.request_m.get_doc_link());
        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, simple_redirect)
{
    // Test settings: Route setup
    string              const url_template = "/redirectme";
    string              const uri_request  = "/redirectme";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + uri_request);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_redirect(url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, redirect_following)
{
    // Test settings: Route setup
    string              const url_template = "/redirectme";
    string              const uri_request  = "/redirectme/123";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + uri_request);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_redirect(url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, redirect_query)
{
    // Test settings: Route setup
    string              const url_template = "/redirectme";
    string              const uri_request  = "/redirectme?x=y&a=123";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + uri_request);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_redirect(url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, redirect_following_and_query)
{
    // Test settings: Route setup
    string              const url_template = "/redirectme";
    string              const uri_request  = "/redirectme/123?x=y&a=123";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + uri_request);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_redirect(url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, redirect_following_no_route)
{
    // Test settings: Route setup
    string              const url_template = "/redirectme";
    string              const uri_request  = "/redirectme_xy/123";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + uri_request);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_redirect(url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, operation::not_found, &operation::not_found);
        //EXPECT_EQ(http_status_code::not_found, router_result.get_http_status_code());
    }
}

TEST_F(router_fixture, base_route)
{
    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const url_template = "";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url);

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result  = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, base_route_with_query)
{
    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const url_template = "";
    operation_handler_t const handler      = example_operation_1;

    // Prepare mock
    set_request_uri(base_url + "?hello=world");

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);
    }
}

TEST_F(router_fixture, route_with_path_params)
{
    // Prepare mock
    string const path_param_value = "value_for_variable";
    set_request_uri(base_url + "/blub/" + path_param_value + "/abc");

    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const parameter    = "collection_name";
    string              const url_template = "/blub/:" + parameter + ":/abc";
    operation_handler_t const handler      = example_operation_1;

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);

        ASSERT_TRUE(router_result.request_m.has_path_parameter(parameter));
        EXPECT_EQ(path_param_value, router_result.request_m.get_path_parameter(parameter));
    }
}

TEST_F(router_fixture, route_with_query_param)
{
    // Prepare mock
    string const query_param_value = "world";
    set_request_uri(base_url + "/blub?hello=" + query_param_value);

    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const parameter    = "hello";
    string              const url_template = "/blub";
    operation_handler_t const handler      = example_operation_1;

    EXPECT_CALL(request_mock, has_query_parameter(parameter))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(request_mock, get_query_parameter(parameter, true))
        .Times(1)
        .WillRepeatedly(Return(query_param_value));

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);

        ASSERT_TRUE(router_result.request_m.has_query_parameter(parameter));
        EXPECT_EQ(query_param_value, router_result.request_m.get_query_parameter(parameter));
    }
}

TEST_F(router_fixture, route_with_bool_query_params)
{
    // Prepare mock
    set_request_uri(base_url + "/foo?myflag");

    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const parameter    = "myflag";
    string              const url_template = "/foo";
    operation_handler_t const handler      = example_operation_1;

    EXPECT_CALL(request_mock, has_query_parameter(parameter))
        .Times(1)
        .WillRepeatedly(Return(true));

    EXPECT_CALL(request_mock, get_query_parameter(parameter, true))
        .Times(1)
        .WillRepeatedly(Return("true"));

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);

        ASSERT_TRUE(router_result.request_m.has_query_parameter(parameter));
        EXPECT_EQ("true", router_result.request_m.get_query_parameter(parameter));
    }
}

TEST_F(router_fixture, route_with_multiple_query_params)
{
    // Prepare mock
    string const query_param_value = "world";
    set_request_uri(base_url + "/blub?myflag-a&myflag-b;hello=" + query_param_value);

    // Test settings: Route setup
    http_method         const method           = http_method::get;
    string              const parameter        = "hello";
    string              const parameter_flag_1 = "myflag-a";
    string              const parameter_flag_2 = "myflag-b";
    string              const url_template     = "/blub";
    operation_handler_t const handler          = example_operation_1;

    EXPECT_CALL(request_mock, has_query_parameter(parameter))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(request_mock, get_query_parameter(parameter, true))
        .Times(1)
        .WillRepeatedly(Return(query_param_value));
    EXPECT_CALL(request_mock, has_query_parameter(parameter_flag_1))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(request_mock, get_query_parameter(parameter_flag_1, true))
        .Times(1)
        .WillRepeatedly(Return("true"));
    EXPECT_CALL(request_mock, has_query_parameter(parameter_flag_2))
        .Times(1)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(request_mock, get_query_parameter(parameter_flag_2, true))
        .Times(1)
        .WillRepeatedly(Return("true"));

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler, &example_operation_1);

        ASSERT_TRUE(router_result.request_m.has_query_parameter(parameter));
        EXPECT_EQ(query_param_value, router_result.request_m.get_query_parameter(parameter));

        ASSERT_TRUE(router_result.request_m.has_query_parameter(parameter_flag_1));
        EXPECT_EQ("true", router_result.request_m.get_query_parameter(parameter_flag_1));

        ASSERT_TRUE(router_result.request_m.has_query_parameter(parameter_flag_2));
        EXPECT_EQ("true", router_result.request_m.get_query_parameter(parameter_flag_2));
    }
}

TEST_F(router_fixture, route_to_first_match)
{
    // Prepare mock
    string const path_param_value = "world";
    set_request_uri(base_url + "/hello/" + path_param_value);

    // Test settings: Route setup
    http_method         const method         = http_method::get;

    string              const parameter_1    = "collection_name";
    string              const url_template_1 = "/hello/:" + parameter_1 + ":";
    operation_handler_t const handler_1      = example_operation_1;

    string              const url_template_2 = "/hello/world";
    operation_handler_t const handler_2      = example_operation_2;

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template_1, handler_1);
        the_router.add_route(method, url_template_2, handler_2);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler_1, &example_operation_1);

        ASSERT_TRUE(router_result.request_m.has_path_parameter(parameter_1));
        EXPECT_EQ(path_param_value, router_result.request_m.get_path_parameter(parameter_1));
    }
}

TEST_F(router_fixture, route_to_first_match_reverse)
{
    // Prepare mock
    string const path_param_value = "world";
    set_request_uri(base_url + "/hello/" + path_param_value);

    // Test settings: Route setup
    http_method         const method         = http_method::get;

    string              const parameter_1    = "collection_name";
    string              const url_template_1 = "/hello/:" + parameter_1 + ":";
    operation_handler_t const handler_1      = example_operation_1;

    string              const url_template_2 = "/hello/world";
    operation_handler_t const handler_2      = example_operation_2;

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template_2, handler_2);
        the_router.add_route(method, url_template_1, handler_1);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, handler_2, &example_operation_2);

        ASSERT_FALSE(router_result.request_m.has_path_parameter(parameter_1));
    }
}

TEST_F(router_fixture, no_route)
{
    // Prepare mock
    set_request_uri(base_url + "/hello/world");

    // Test settings: Route setup
    http_method         const method       = http_method::get;
    string              const url_template = "/hello";
    operation_handler_t const handler      = example_operation_1;

    // Test execution
    {
        router the_router(service_identity_mock, base_url);

        the_router.add_route(method, url_template, handler);
        auto router_result = the_router.route(std::move(request_mock_to_move));

        check_handler(router_result, operation::not_found, &operation::not_found);
    }
}


//---- End of source file ------------------------------------------------------
