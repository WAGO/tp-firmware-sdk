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
///  \brief    Test of operation object.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest/definitions.hpp"
#include "rest/operation.hpp"
#include "rest/json_api.hpp"
#include "http/head_response.hpp"
#include "http/http_exception.hpp"
#include "mocks/mock_frontend_extended.hpp"
#include "mocks/mock_service_identity.hpp"
#include "mocks/mock_request.hpp"
#include "mocks_libwda/mock_operation.hpp"
#include "mocks_libwda/mock_run_object_manager.hpp"

#include "wago/wdx/unauthorized.hpp"
#include <wda_ipc/representation.hpp>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>

#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::resolved_future;
using namespace wago::wdx::wda::http;
using namespace wago::wdx::wda::rest;
using namespace wago::wdx;
using std::unique_ptr;
using std::shared_ptr;
using std::make_unique;
using std::make_shared;
using testing::ByMove;

//------------------------------------------------------------------------------
// macro definitions
//------------------------------------------------------------------------------
#define EXPECT_HTTP_EQ(expected_code, actual_code) \
    EXPECT_EQ(static_cast<unsigned>(expected_code), static_cast<unsigned>(actual_code)) \
    << "Expected HTTP status " << static_cast<unsigned>(expected_code) << ", "\
    << "got HTTP status " << static_cast<unsigned>(actual_code) << ". "

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class operation_fixture : public ::testing::Test
{
protected:
    // static mocks
    mock_service_identity               service_identity_mock;
    unique_ptr<mock_frontend_extended>  core_frontend_mock_ptr = make_unique<mock_frontend_extended>();
    mock_frontend_extended             &core_frontend_mock = *core_frontend_mock_ptr.get();
    unique_ptr<mock_request>            request_mock_to_move = make_unique<mock_request>();
    mock_request                       &request_mock = *request_mock_to_move.get();
    mock_operations                     operations_mock;
    shared_ptr<mock_run_object_manager> run_manager_mock_ptr = make_shared<mock_run_object_manager>();

    string                    request_path          = "/test/request/path";
    map<string, string> const path_parameters       = { { "PATH_PARAM1",  "pparam1" }, { "PATH_PARAM2",  "pparam2" } };
    map<string, string>       query_parameters      = { { "QUERY_PARAM1", "qparam1" }, { "QUERY_PARAM2", "qparam2" } };
    string                    request_query         = "?QUERY_PARAM1=qparam1&QUERY_PARAM2=qparam2";
    string                    request_uri           = request_path + request_query;
    json_api                  json_api_serializer;

    wago::wdx::parameter_instance_path         my_path;
    shared_ptr<method_run_object>              my_run  = std::make_shared<method_run_object>("42", my_path, 23);
    std::vector<shared_ptr<method_run_object>> my_runs = {my_run};

protected:
    operation_fixture() = default;
    ~operation_fixture() override = default;

    void SetUp() override
    {
        // Set default call expectations
        core_frontend_mock.set_default_expectations();
        service_identity_mock.set_default_expectations();
        request_mock.set_default_expectations();
        operations_mock.set_default_expectations();
        run_manager_mock_ptr->set_default_expectations();

        EXPECT_CALL(request_mock, get_request_uri())
            .Times(AnyNumber())
            .WillRepeatedly(Invoke([&request_uri=request_uri]() { return request_uri; }));

        EXPECT_CALL(request_mock, has_query_parameter(::testing::_))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke([&query_params=query_parameters](std::string const & name) { return query_params.count(name) > 0; }));

        EXPECT_CALL(request_mock, get_query_parameter(::testing::_, true))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke([&query_params=query_parameters](std::string const & name, bool) { return query_params.at(name); }));

        // optionally, expect check for "Origin" header 
        EXPECT_CALL(request_mock, get_http_header(::testing::StrEq("Origin")))
            .Times(AnyNumber())
            .WillRepeatedly(Return(""));
        EXPECT_CALL(request_mock, finish())
            .Times(AtMost(1));

        EXPECT_CALL(*run_manager_mock_ptr, clean_runs())
            .Times(Exactly(1));
    }
};

TEST_F(operation_fixture, construct_delete)
{  
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
}

TEST_F(operation_fixture, operation_execution)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t const & handler = &example_operation_2;

    EXPECT_CALL(operations_mock, example_operation_2_mock_proxy(::testing::_))
        .Times(1)
        .WillRepeatedly(Return(new head_response(http_status_code::ok)));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1);
    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_resource_collection_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"first\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"last\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_method_runs)
{
    map<string, string> path_parameters_local = {
        {"method_id", "0-0-some-path"}
    };
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_runs;

    parameter_response instance_response;
    shared_ptr<parameter_definition> definition = make_shared<parameter_definition>();
    definition->value_type = parameter_value_types::method;
    instance_response.definition = definition;
    vector<parameter_response> instances  = { instance_response };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(instances)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_resource_collection_response)));

    EXPECT_CALL(*run_manager_mock_ptr, get_runs(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(my_runs));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_method_run_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("23")); //Searching for given timeout value of 23
}
}

TEST_F(operation_fixture, operation_execution_get_method_run)
{
    map<string, string> path_parameters_local = {
        {"method_id", "0-0-some-path"},
        {"method_run_id", "42"}
    };
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_run;

    parameter_response instance_response;
    shared_ptr<parameter_definition> definition = make_shared<parameter_definition>();
    definition->value_type = parameter_value_types::method;
    instance_response.definition = definition;
    vector<parameter_response> instances = { instance_response };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(instances)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_method_run_response)));

    EXPECT_CALL(*run_manager_mock_ptr, get_run(::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(my_run));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_method_run_not_found_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_found, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::not_found)));
}
}

ACTION(throw_run_object_not_found)
{
    throw wago::wdx::wda::http::http_exception("Run object not found",
                                                wago::wdx::wda::http::http_status_code::not_found);
}

TEST_F(operation_fixture, operation_execution_get_method_run_failed)
{
    map<string, string> path_parameters_local = {
        {"method_id", "0-0-some-path"},
        {"method_run_id", "42"}
    };
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_run;

    parameter_response instance_response;
    shared_ptr<parameter_definition> definition = make_shared<parameter_definition>();
    definition->value_type = parameter_value_types::method;
    instance_response.definition = definition;
    vector<parameter_response> instances = { instance_response };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(instances)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_method_run_not_found_response)));

    EXPECT_CALL(*run_manager_mock_ptr, get_run(::testing::_, ::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(throw_run_object_not_found());

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_post_method_invocation_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::created, response.get_status_code());
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());

    string const content = response.get_content();
    nlohmann::json json_content = nlohmann::json::parse(content);

    string const id = json_content.at("data").at("id").get<string>();
}

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
MATCHER_P(ParameterInstancePathEq, other, "Equality matcher for parameter_instance_path") {
    // FIXME: arg == other does not work as expected (not agnostic about '/' and '-')
    string first  = arg.device_path   + arg.parameter_path;
    string second = other.device_path + other.parameter_path;
    std::replace(first.begin(),  first.end(),  '/', '-');
    std::replace(second.begin(), second.end(), '/', '-');
    return first == second;
}
GNUC_DIAGNOSTIC_POP

GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
MATCHER_P(MethodInArgsEq, other, "Equality matcher for method_invocation_in_args") {
    for(auto const &elem: arg)
    {
        auto const value = elem.second;
        auto const other_value = other.at(elem.first);
        if(value->get_type() != other_value->get_type())
        {
            return false;
        }
        if(value->get_json() != other_value->get_json())
        {
            return false;
        }
    }
    return true;
}
GNUC_DIAGNOSTIC_POP
}

TEST_F(operation_fixture, operation_execution_post_method_run)
{
    char const method_id[]      = "0-0-some-path";
    char const in_arg_1_name[]  = "bla";
    char const in_arg_1_value[] = "\"blub\"";
    char const request_body[]   = R"({"data":{"attributes":{"inArgs":{"bla":{"value":"blub"}}},"type": "runs"}})";
    map<string, string> path_parameters_local = { {"method_id", method_id} };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::post_method_run;

    parameter_instance_path          const method_instance = wago::wda_ipc::from_string<parameter_instance_path>(method_id);
    method_invocation_in_args        const method_in_args = { { in_arg_1_name, parameter_value::create_with_unknown_type(in_arg_1_value) } };
    method_invocation_named_response       core_response;
    core_response.status = wago::wdx::status_codes::success;

    EXPECT_CALL(request_mock, get_content())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_body));
    EXPECT_CALL(core_frontend_mock, invoke_method_by_path(method_instance, MethodInArgsEq(method_in_args)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, add_response_header(::testing::StrEq("Location"), ::testing::_))
        .Times(Exactly(1));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(&check_post_method_invocation_response)));

    EXPECT_CALL(*run_manager_mock_ptr, add_run(::testing::_, ::testing::_, ::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](wago::wdx::parameter_instance_path const &,
                        uint16_t ,
                        wago::future<wago::wdx::method_invocation_named_response>  &,
                        ready_handler my_handler)
                        {
                            my_handler("42");
                            return "42";
                        }));
    EXPECT_CALL(*run_manager_mock_ptr, get_run(::testing::_, ::testing::Eq("42")))
        .Times(Exactly(1))
        .WillRepeatedly(Return(std::make_shared<method_run_object>("42", method_instance, 23)));
    EXPECT_CALL(*run_manager_mock_ptr, max_runs_reached())
        .Times(Exactly(1))
        .WillRepeatedly(Return(false));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_post_method_run_response_bad_request(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::bad_request, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::bad_request)));
}
}

TEST_F(operation_fixture, operation_execution_post_method_run_with_wrong_type)
{
    char const method_id[]      = "0-0-method-path";
    char const in_arg_1_name[]  = "bla";
    char const in_arg_1_value[] = "\"blub\"";
    char const request_body[] = R"({"data":{"attributes":{"inArgs":{"bla":{"value":"blub"}}},"type": "fake"}})";
    map<string, string> path_parameters_local = { {"method_id", method_id} };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::post_method_run;

    parameter_instance_path          const method_instance = wago::wda_ipc::from_string<parameter_instance_path>(method_id);
    method_invocation_in_args        const method_in_args  = { { in_arg_1_name, parameter_value::create_with_unknown_type(in_arg_1_value) } };
    method_invocation_named_response       core_response;
    core_response.status = wago::wdx::status_codes::success;

    EXPECT_CALL(*run_manager_mock_ptr, max_runs_reached())
        .Times(AnyNumber())
        .WillRepeatedly(Return(false));

    EXPECT_CALL(request_mock, get_content())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_body));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(&check_post_method_run_response_bad_request)));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_get_service_identity_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code());
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"attributes\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"apiVersion\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"serviceName\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"serviceVersion\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_service_identity)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_service;

    EXPECT_CALL(service_identity_mock, get_id())
        .Times(AnyNumber())
        .WillRepeatedly(Return("TestID"));
    EXPECT_CALL(service_identity_mock, get_name())
        .Times(AnyNumber())
        .WillRepeatedly(Return("Test identity"));
    EXPECT_CALL(service_identity_mock, get_version_string())
        .Times(AnyNumber())
        .WillRepeatedly(Return("1.2.3"));
    EXPECT_CALL(service_identity_mock, get_major_version())
        .Times(AnyNumber())
        .WillRepeatedly(Return(1));
    EXPECT_CALL(service_identity_mock, get_minor_version())
        .Times(AnyNumber())
        .WillRepeatedly(Return(2));
    EXPECT_CALL(service_identity_mock, get_bugfix_version())
        .Times(AnyNumber())
        .WillRepeatedly(Return(3));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_service_identity_response)));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_service_identity_accept_json_api_only)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_service;

    EXPECT_CALL(service_identity_mock, get_id())
        .Times(AnyNumber())
        .WillRepeatedly(Return("TestID"));
    EXPECT_CALL(service_identity_mock, get_name())
        .Times(AnyNumber())
        .WillRepeatedly(Return("Test identity"));
    EXPECT_CALL(service_identity_mock, get_version_string())
        .Times(AnyNumber())
        .WillRepeatedly(Return("1.2.3"));
    EXPECT_CALL(service_identity_mock, get_major_version())
        .Times(AnyNumber())
        .WillRepeatedly(Return(1));
    EXPECT_CALL(service_identity_mock, get_minor_version())
        .Times(AnyNumber())
        .WillRepeatedly(Return(2));
    EXPECT_CALL(service_identity_mock, get_bugfix_version())
        .Times(AnyNumber())
        .WillRepeatedly(Return(3));
    EXPECT_CALL(request_mock, get_accepted_types())
        .Times(AnyNumber())
        .WillRepeatedly(Return("application/vnd.api+json"));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_service_identity_response)));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_all_devices)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_devices;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_resource_collection_response)));
    EXPECT_CALL(core_frontend_mock, get_all_devices())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(ByMove(resolved_future(device_collection_response()))));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_single_device_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"attributes\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"firmwareVersion\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"orderNumber\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_device)
{
    device_id device_id_object(1, 2);
    string device_path = wago::wda_ipc::to_string(device_id_object);
    map<string, string> path_parameters_local = { {"device_id", device_path}, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_device;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_single_device_response)));

    device_response the_device { device_id_object, "", "" };
    EXPECT_CALL(core_frontend_mock, get_device(::testing::Eq(device_id_object)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(the_device)))));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_device_bad_id)
{
    string device_path = "1x2";
    map<string, string> path_parameters_local = { {"device_id", device_path}, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_device;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_single_device_response_not_found(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_found, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::not_found)));
}
}

TEST_F(operation_fixture, operation_execution_get_device_fails_not_found)
{
    char const device_id[]       = "4-5"; // not found
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_device;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_single_device_response_not_found)));

    device_response the_device { {1, 2}, "", "" };
    device_collection_response the_collection;
    the_collection.devices.push_back(the_device);

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_subdevices_object_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_subdevices_object)
{
    char const device_id[]       = "0-0"; // headstation!
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "PATH_PARAM2",  "pparam2" } };
    
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_subdevices_object;

    feature_list_response feature_response;
    device_feature_information feature_info;
    feature_info.name = "LocalbusMasterKBus";
    feature_response.features.push_back(feature_info);

    EXPECT_CALL(core_frontend_mock, get_features(::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ByMove(resolved_future<vector<feature_list_response>>({feature_response}))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_subdevices_object_response)));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_subdevices_object_bad_id)
{
    char const device_id[]       = "0x0";
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_subdevices_object;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_subdevices_object_response_fails_device_not_found(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_found, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::not_found)));
}
}

TEST_F(operation_fixture, operation_execution_get_subdevices_object_device_not_found)
{
    char const device_id[]       = "4-5"; // not existing device
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "PATH_PARAM2",  "pparam2" } };
    
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_subdevices_object;

    EXPECT_CALL(core_frontend_mock, get_features(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::vector<feature_list_response>({
                feature_list_response(wago::wdx::status_codes::unknown_device)
            })))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_subdevices_object_response_fails_device_not_found)));

    device_response the_device { {1, 2}, "", "" };
    device_collection_response the_collection;
    the_collection.devices.push_back(the_device);

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_subdevices_object_response_fails_no_subdevices(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_found, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::not_found)));
}
}

TEST_F(operation_fixture, operation_execution_get_subdevices_object_no_subdevices)
{
    char const device_id[]       = "0-0";
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "PATH_PARAM2",  "pparam2" } };
    
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_subdevices_object;

    feature_list_response feature_response;
    EXPECT_CALL(core_frontend_mock, get_features(::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ByMove(resolved_future<vector<feature_list_response>>({feature_response}))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_subdevices_object_response_fails_no_subdevices)));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_subdevices_by_collection_name)
{
    char const device_id[]       = "0-0"; // has to be the headstation!
    char const collection_name[] = "kbus";
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "collection_name", collection_name }, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_subdevices_by_collection_name;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_resource_collection_response)));
    
    EXPECT_CALL(core_frontend_mock, get_subdevices_by_collection_name(::testing::StrEq(collection_name)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(device_collection_response()))));
    feature_list_response feature_response;
    device_feature_information feature_info;
    feature_info.name = "LocalbusMasterKBus";
    feature_response.features.push_back(feature_info);
    EXPECT_CALL(core_frontend_mock, get_features(::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ByMove(resolved_future<vector<feature_list_response>>({feature_response}))));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_subdevices_by_collection_name_response_fail(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_found, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::not_found)));
}
}

TEST_F(operation_fixture, operation_execution_get_subdevices_by_collection_name_fails)
{
    char const device_id[]       = "0-0"; // has to be the headstation!
    char const collection_name[] = "invalid_collection";
    map<string, string> path_parameters_local = { {"device_id", device_id}, { "collection_name", collection_name }, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_subdevices_by_collection_name;

    feature_list_response feature_response;
    device_feature_information feature_info;
    feature_info.name = "LocalbusMasterKBus";
    feature_response.features.push_back(feature_info);
    EXPECT_CALL(core_frontend_mock, get_features(::testing::_))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ByMove(resolved_future<vector<feature_list_response>>({feature_response}))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_subdevices_by_collection_name_response_fail)));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_redirect_headstation_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::moved_permanently, response.get_status_code());
    string const redirected_uri = string("/servicename") + wago::wdx::wda::rest::short_headstation_target + "/test" + "?QUERY_PARAM1=qparam1&QUERY_PARAM2=qparam2";
    EXPECT_STREQ(redirected_uri.c_str(), response.get_response_header().at("Location").c_str());
    EXPECT_STREQ("", response.get_content().c_str());
}
}

TEST_F(operation_fixture, operation_execution_redirect_headstation)
{
    string              const request_path_local     = string("/servicename") + wago::wdx::wda::rest::short_headstation + "/test";
    string              const request_query_local    = "?QUERY_PARAM1=qparam1&QUERY_PARAM2=qparam2";
    map<string, string> const query_parameters_local = { { "QUERY_PARAM1", "qparam1" }, { "QUERY_PARAM2", "qparam2" } };
    query_parameters = query_parameters_local;
    string              const request_uri_local      = request_path_local + request_query_local;

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::redirect_headstation;

    EXPECT_CALL(request_mock, get_request_uri())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_uri_local));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_redirect_headstation_response)));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_redirect_trailing_slash)
{
    string              const request_path_local     = "/any/path/with/a/trailing/slash/";
    string              const request_query_local    = "?QUERY_PARAM1=qparam1&QUERY_PARAM2=qparam2";
    map<string, string> const query_parameters_local = { { "QUERY_PARAM1", "qparam1" }, { "QUERY_PARAM2", "qparam2" } };
    query_parameters = query_parameters_local;
    string              const request_uri_local      = request_path_local + request_query_local;
    string              const request_uri_redirected = request_path_local.substr(0, request_path_local.size() - 1) + request_query_local;

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::redirect_trailing_slash;

    EXPECT_CALL(request_mock, get_request_uri())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_uri_local));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke([request_uri_redirected](response_i const &response) {
            EXPECT_HTTP_EQ(http_status_code::moved_permanently, response.get_status_code());
            EXPECT_STREQ(request_uri_redirected.c_str(), response.get_response_header().at("Location").c_str());
            EXPECT_STREQ("", response.get_content().c_str());
        })));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_redirect_to_lowercase)
{
    string              const request_path_local     = "/any/pATH/wiTh/So-m3/Mi_xeD/casEs";
    request_path = request_path_local;
    string              const request_path_lower     = "/any/path/with/so-m3/mi_xed/cases";
    string              const request_query_local    = "?QUERY_param1=qpAram1&query-param=qpaRam2";
    request_query = request_query_local;
    map<string, string> const query_parameters_local = { { "QUERY_PARAM1", "qparam1" }, { "QUERY_PARAM2", "qparam2" } };
    query_parameters = query_parameters_local;
    string              const request_uri_local      = request_path_local + request_query_local;
    request_uri = request_uri_local;
    string              const request_uri_lower      = request_path_lower + request_query_local;

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::redirect_to_lowercase;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke([request_uri_lower](response_i const &response) {
            EXPECT_HTTP_EQ(http_status_code::moved_permanently, response.get_status_code());
            EXPECT_STREQ(request_uri_lower.c_str(), response.get_response_header().at("Location").c_str());
            EXPECT_STREQ("", response.get_content().c_str());
        })));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_not_found_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_found, response.get_status_code());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr(to_string(http_status_code::not_found)));
}
}

TEST_F(operation_fixture, operation_not_found)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::not_found;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_not_found_response)));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_not_implemented_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::not_implemented, response.get_status_code());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("not implemented"));
}
}

TEST_F(operation_fixture, operation_not_implemented)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::not_implemented;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_not_implemented_response)));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_method_not_allowed)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::method_not_allowed;

    EXPECT_CALL(request_mock, add_response_header(::testing::StrEq("Allow"), ::testing::_))
        .Times(1);

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &response) {
            EXPECT_HTTP_EQ(http_status_code::method_not_allowed, response.get_status_code());
            EXPECT_THAT(response.get_content(), testing::HasSubstr("error"));
        })));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_not_supported_content_type)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::not_supported_content_type;

    EXPECT_CALL(request_mock, get_content_type())
        .Times(AnyNumber())
        .WillRepeatedly(Return("text/plain"));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &response) {
            EXPECT_HTTP_EQ(http_status_code::unsupported_media_type, response.get_status_code());
            EXPECT_THAT(response.get_content(), testing::HasSubstr("error"));
        })));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_not_acceptable)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::not_acceptable;

    EXPECT_CALL(request_mock, get_http_header(::testing::StrEq("Accept")))
        .Times(AnyNumber())
        .WillRepeatedly(Return("text/plain"));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &response) {
            EXPECT_HTTP_EQ(http_status_code::not_acceptable, response.get_status_code());
            EXPECT_THAT(response.get_content(), testing::HasSubstr("error"));
        })));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_head_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code());
    EXPECT_STREQ("", response.get_content().c_str());
}
}

TEST_F(operation_fixture, operation_get_head)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_head;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_head_response)));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_single_parameter_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"attributes\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"value\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_parameter)
{
    char const parameter_id[]                 = "0-0-a-path";
    map<string, string> path_parameters_local = { {"parameter_id", parameter_id}, { "PATH_PARAM2",  "pparam2" } };
    
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_parameter;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_single_parameter_response)));

    parameter_response the_parameter;
    the_parameter.definition = std::make_shared<wdmm::parameter_definition>();
    the_parameter.value = parameter_value::create(true);
    parameter_instance_path param_id = wago::wda_ipc::from_string<parameter_instance_path>("0-0-a-path");
    vector<parameter_instance_path> parameter_requests = { param_id };
    the_parameter.path = param_id;
    vector<parameter_response> parameter_responses  = { the_parameter };
    vector<parameter_response> parameter_responses2 = { the_parameter };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::Eq(parameter_requests)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(parameter_responses)))));
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::Eq(parameter_requests)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(parameter_responses2)))));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_parameter_bad_id)
{
    char const parameter_id[]                 = "0-0xa-malformed-path";
    map<string, string> path_parameters_local = { {"parameter_id", parameter_id}, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_parameter;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    test_operation.handle(handler, std::move(rest_request));
}

namespace {
void check_get_single_method_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"attributes\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"relationships\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"runs\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_method)
{
    char const method_id[]                    = "0-0-a-path";
    map<string, string> path_parameters_local = { {"method_id", method_id}, { "PATH_PARAM2",  "pparam2" } };
    
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(1)
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_single_method_response)));

    parameter_response the_method;
    the_method.definition = std::make_shared<wdmm::parameter_definition>();
    the_method.definition->value_type = wdmm::parameter_value_types::method;
    parameter_instance_path meth_id = wago::wda_ipc::from_string<parameter_instance_path>("0-0-a-path");
    vector<parameter_instance_path> method_requests = { meth_id };
    the_method.path = meth_id;
    vector<parameter_response> method_responses = { the_method };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::Eq(method_requests)))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(method_responses)))));

    test_operation.handle(handler, std::move(rest_request));
}

TEST_F(operation_fixture, operation_execution_get_method_bad_id)
{
    char const method_id[]                    = "0-0xa-malformed-path";
    map<string, string> path_parameters_local = { {"method_id", method_id}, { "PATH_PARAM2",  "pparam2" } };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    test_operation.handle(handler, std::move(rest_request));
}


GNUC_DIAGNOSTIC_PUSH
GNUC_DIAGNOSTIC_IGNORE("-Wsuggest-override")
MATCHER_P(ValuePathRequestsEq, other, "Equality matcher for vector of wdx::value_request") {
    if (arg.size() != other.size())
    {
        return false;
    }
    for (size_t i = 0; i < arg.size(); ++i)
    {
        auto const path = arg.at(i).param_path;
        auto const other_path = other.at(i).param_path;
        // FIXME: path != other_path does not work as expected (not agnostic about '/' and '-')
        string first  = path.device_path       + path.parameter_path;
        string second = other_path.device_path + other_path.parameter_path;
        std::replace(first.begin(),  first.end(),  '/', '-');
        std::replace(second.begin(), second.end(), '/', '-');
        if(first != second)
        {
            return false;
        }
        auto const value = arg.at(i).value;
        auto const other_value = other.at(i).value;
        if(value->get_type() != other_value->get_type())
        {
            return false;
        }
        if(value->get_json() != other_value->get_json())
        {
            return false;
        }
    }
    return true;
}
GNUC_DIAGNOSTIC_POP

TEST_F(operation_fixture, operation_execution_set_parameter)
{
    char const param_id[]       = "0-0-a-path";
    char const value[]          = "\"blub\"";
    char const request_body[]   = R"({"data":{"attributes":{"value":"blub"},"type":"parameters"}})";
    map<string, string> path_parameters_local = { {"parameter_id", param_id} };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameter;

    parameter_instance_path          const param_instance = wago::wda_ipc::from_string<parameter_instance_path>(param_id);
    std::shared_ptr<parameter_value> const param_value = parameter_value::create_with_unknown_type(value);
    value_path_request                     param_request(param_instance, param_value);
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::success));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    std::vector<value_path_request> requests = { param_request };
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::no_content, res.get_status_code())
                << "Actual status code: " + to_string(res.get_status_code()) << std::endl
                << "Actual error body: " + res.get_content();
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_set_parameter_with_ID_in_body)
{
    char const param_id[]     = "0-0-example-path";
    char const value[]        = "\"blub\"";
    char const request_body[] = R"({"data":{"id":"0-0-example-path","attributes":{"value":"blub"},"type":"parameters"}})";
    map<string, string> path_parameters_local = { {"parameter_id", param_id} };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameter;

    parameter_instance_path          const param_instance = wago::wda_ipc::from_string<parameter_instance_path>(param_id);
    std::shared_ptr<parameter_value> const param_value = parameter_value::create_with_unknown_type(value);
    value_path_request               const param_request(param_instance, param_value);
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::success));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    std::vector<value_path_request> requests = { param_request };
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::no_content, res.get_status_code())
                << "Actual status code: " + to_string(res.get_status_code()) << std::endl
                << "Actual error body: " + res.get_content();
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_set_parameter_value_adjusted)
{
    char const param_id[]       = "0-0-a-path";
    char const value[]          = "\"blub\"";
    char const request_body[]   = R"({"data":{"attributes":{"value":"blub"},"type":"parameters"}})";
    map<string, string> path_parameters_local = { {"parameter_id", param_id} };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameter;

    parameter_instance_path          const param_instance = wago::wda_ipc::from_string<parameter_instance_path>(param_id);
    std::shared_ptr<parameter_value> const param_value = parameter_value::create_with_unknown_type(value);
    value_path_request                     param_request(param_instance, param_value);
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::parameter_value::create_string("blub adjusted")));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    std::vector<value_path_request> requests = { param_request };
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    std::vector<parameter_instance_path> read_requests = { param_instance };

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::ok, res.get_status_code())
                << "Actual status code: " + to_string(res.get_status_code()) << std::endl
                << "Actual error body: " + res.get_content();

            auto const response_content = res.get_content();
            EXPECT_THAT(response_content, testing::HasSubstr("blub adjusted"));
            EXPECT_THAT(response_content, testing::HasSubstr("value"));
            EXPECT_THAT(response_content, testing::Not(testing::HasSubstr("writeable")));
            EXPECT_THAT(response_content, testing::Not(testing::HasSubstr("relationships")));
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_set_parameters)
{
    char const value_1[]          = "\"blub\"";
    char const value_2[]          = "123";
    char const request_body[]   = R"({"data":[
        {"attributes":{"value":"blub"},"type":"parameters","id":"0-0-a-path"},
        {"attributes":{"value":123},"type":"parameters","id":"0-0-other-path"}
    ]})";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameters;

    parameter_instance_path          const param_instance_1 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-a-path");
    parameter_instance_path          const param_instance_2 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-other-path");
    std::shared_ptr<parameter_value> const param_value_1 = parameter_value::create_with_unknown_type(value_1);
    std::shared_ptr<parameter_value> const param_value_2 = parameter_value::create_with_unknown_type(value_2);
    std::vector<value_path_request>        param_requests = {
        value_path_request(param_instance_1, param_value_1),
        value_path_request(param_instance_2, param_value_2)
    };
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::success));
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::success));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(param_requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::no_content, res.get_status_code())
                << "Actual status code: " + to_string(res.get_status_code()) << std::endl
                << "Actual error body: " + res.get_content();
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}


TEST_F(operation_fixture, operation_execution_set_parameters_value_adjusted)
{
    char const value_1[]          = "\"blub\"";
    char const value_2[]          = "123";
    char const request_body[]   = R"({"data":[
        {"attributes":{"value":"blub"},"type":"parameters","id":"0-0-a-path"},
        {"attributes":{"value":123},"type":"parameters","id":"0-0-other-path"}
    ]})";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameters;

    parameter_instance_path          const param_instance_1 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-a-path");
    parameter_instance_path          const param_instance_2 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-other-path");
    std::shared_ptr<parameter_value> const param_value_1 = parameter_value::create_with_unknown_type(value_1);
    std::shared_ptr<parameter_value> const param_value_2 = parameter_value::create_with_unknown_type(value_2);
    std::vector<value_path_request>        param_requests = {
        value_path_request(param_instance_1, param_value_1),
        value_path_request(param_instance_2, param_value_2)
    };
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::parameter_value::create_string("blub adjusted")));
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::success));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(param_requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::ok, res.get_status_code())
                << "Actual status code: " + to_string(res.get_status_code()) << std::endl
                << "Actual error body: " + res.get_content();

            auto const response_content = res.get_content();
            EXPECT_THAT(response_content, testing::HasSubstr("blub adjusted"));
            EXPECT_THAT(response_content, testing::HasSubstr("0-0-a-path"));
            EXPECT_THAT(response_content, testing::HasSubstr("value"));
            EXPECT_THAT(response_content, testing::Not(testing::HasSubstr("0-0-other-path")));
            EXPECT_THAT(response_content, testing::Not(testing::HasSubstr("writeable")));
            EXPECT_THAT(response_content, testing::Not(testing::HasSubstr("relationships")));
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}


TEST_F(operation_fixture, operation_execution_set_parameters_value_adjusted_and_deferred)
{
    char const value_1[]          = "\"blub\"";
    char const value_2[]          = "123";
    char const request_body[]   = R"({"data":[
        {"attributes":{"value":"blub"},"type":"parameters","id":"0-0-a-path"},
        {"attributes":{"value":123},"type":"parameters","id":"0-0-other-path"}
    ]})";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameters;

    parameter_instance_path          const param_instance_1 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-a-path");
    parameter_instance_path          const param_instance_2 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-other-path");
    std::shared_ptr<parameter_value> const param_value_1 = parameter_value::create_with_unknown_type(value_1);
    std::shared_ptr<parameter_value> const param_value_2 = parameter_value::create_with_unknown_type(value_2);
    std::vector<value_path_request>        param_requests = {
        value_path_request(param_instance_1, param_value_1),
        value_path_request(param_instance_2, param_value_2)
    };
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::parameter_value::create_string("blub adjusted")));
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::wda_connection_changes_deferred));

    std::vector<set_parameter_response>        core_response_for_deferred;
    core_response_for_deferred.push_back(set_parameter_response(wago::wdx::status_codes::success));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(param_requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    std::vector<value_path_request> deferred_param_requests = { param_requests.at(1) };
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path(ValuePathRequestsEq(deferred_param_requests)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    std::vector<parameter_instance_path> read_requests = { param_instance_1 };

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::accepted, res.get_status_code())
                << "Actual status code: " + to_string(res.get_status_code()) << std::endl
                << "Actual error body: " + res.get_content();

            auto const response_content = res.get_content();
            EXPECT_THAT(response_content, testing::HasSubstr("blub adjusted"));
            EXPECT_THAT(response_content, testing::HasSubstr("0-0-a-path"));
            EXPECT_THAT(response_content, testing::Not(testing::HasSubstr("0-0-other-path")));
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_set_parameters_with_error)
{
    char const value_1[]      = "\"blub\"";
    char const value_2[]      = "123";
    char const request_body[] = R"({"data":[
        {"attributes":{"value":"blub"},"type":"parameters","id":"0-0-a-path"},
        {"attributes":{"value":123},"type":"parameters","id":"0-0-other-path"}
    ]})";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::set_parameters;

    parameter_instance_path          const param_instance_1 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-a-path");
    parameter_instance_path          const param_instance_2 = wago::wda_ipc::from_string<parameter_instance_path>("0-0-other-path");
    std::shared_ptr<parameter_value> const param_value_1 = parameter_value::create_with_unknown_type(value_1);
    std::shared_ptr<parameter_value> const param_value_2 = parameter_value::create_with_unknown_type(value_2);
    std::vector<value_path_request>        param_requests = {
        value_path_request(param_instance_1, param_value_1),
        value_path_request(param_instance_2, param_value_2)
    };
    std::vector<set_parameter_response>    core_response;
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::success));
    core_response.push_back(set_parameter_response(wago::wdx::status_codes::internal_error));

    EXPECT_CALL(request_mock, get_content())
        .Times(Exactly(1))
        .WillRepeatedly(Return(request_body));
    EXPECT_CALL(core_frontend_mock, set_parameter_values_by_path_connection_aware(ValuePathRequestsEq(param_requests), ::testing::Eq(true)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::internal_server_error, res.get_status_code());
        }));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_post_monitoring_list_response(response_i const &response,
                                         bool              included_params = false)
{
    EXPECT_HTTP_EQ(http_status_code::created, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"timeout\":"));

    if (included_params)
    {
        EXPECT_THAT(response.get_content(), testing::HasSubstr("\"included\":"));
    }
    else 
    {
        EXPECT_THAT(response.get_content(), testing::Not(testing::HasSubstr("\"included\":")));
    }
}
}

TEST_F(operation_fixture, operation_execution_post_monitoring_list)
{
    uint16_t    request_timeout = 60;
    std::string parameter_instance_str = "0-0-a-path";
    std::string request_body    = "{\
                                        \"data\":{\
                                            \"type\":\"monitoring-lists\",\
                                            \"attributes\":{\
                                                \"timeout\":" + std::to_string(static_cast<unsigned>(request_timeout)) + "\
                                            },\
                                            \"relationships\":{\
                                                \"parameters\":{\
                                                    \"data\":[{\
                                                        \"id\":\"" + parameter_instance_str + "\",\
                                                        \"type\":\"parameters\"\
                                                    }]\
                                                }\
                                            }\
                                        }\
                                    }";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::post_monitoring_list;

    std::vector<wago::wdx::parameter_response> check_response;

    monitoring_list_response core_response = {};
    core_response.status = wago::wdx::status_codes::success;
    core_response.monitoring_list.id = 123;
    core_response.monitoring_list.one_off = request_timeout == 0 ? true : false;
    core_response.monitoring_list.timeout_seconds = request_timeout;

    vector<parameter_instance_path> parameter_instances = {
        ::wago::wda_ipc::from_string<parameter_instance_path>(parameter_instance_str) // must match parameter_instance_str
    };

    EXPECT_CALL(request_mock, get_content())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_body.c_str()));
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::Eq(parameter_instances)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(check_response)))));
    EXPECT_CALL(core_frontend_mock, create_monitoring_list_with_paths(::testing::Eq(parameter_instances), request_timeout))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, add_response_header(::testing::StrEq("Location"), ::testing::_))
        .Times(Exactly(1));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](auto const &res) {
            check_post_monitoring_list_response(res, false);
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_post_monitoring_list_with_include_parameters)
{
    map<string, string> query_parameters_local = {
        {"include", "parameters"}
    };
    query_parameters = query_parameters_local;
    uint16_t    request_timeout = 60;
    std::string parameter_instance_str = "0-0-100-1";
    std::string request_body    = "{\
                                        \"data\":{\
                                            \"type\":\"monitoring-lists\",\
                                            \"attributes\":{\
                                                \"timeout\":" + std::to_string(static_cast<unsigned>(request_timeout)) + "\
                                            },\
                                            \"relationships\":{\
                                                \"parameters\":{\
                                                    \"data\":[{\
                                                        \"id\":\"" + parameter_instance_str + "\",\
                                                        \"type\":\"parameters\"\
                                                    }]\
                                                }\
                                            }\
                                        }\
                                    }";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::post_monitoring_list;

    std::vector<wago::wdx::parameter_response> check_response;

    monitoring_list_response core_response = {};
    core_response.status = wago::wdx::status_codes::success;
    core_response.monitoring_list.id = 123;
    core_response.monitoring_list.one_off = request_timeout == 0 ? true : false;
    core_response.monitoring_list.timeout_seconds = request_timeout;

    vector<parameter_instance_path> parameter_instances = {
        ::wago::wda_ipc::from_string<parameter_instance_path>(parameter_instance_str) // must match parameter_instance_str
    };
    vector<parameter_response> parameter_responses = {
        parameter_response()
    };
    parameter_responses.at(0).value = parameter_value::create("test");
    parameter_responses.at(0).path  = parameter_instances.at(0);
    parameter_responses.at(0).definition = std::make_shared<wdmm::parameter_definition>();
    monitoring_list_values_response monitoring_list_values_response(parameter_responses);

    EXPECT_CALL(request_mock, get_content())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_body.c_str()));
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::Eq(parameter_instances)))
        .Times(AnyNumber())
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(check_response)))));
    EXPECT_CALL(core_frontend_mock, create_monitoring_list_with_paths(::testing::Eq(parameter_instances), request_timeout))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(core_frontend_mock, get_values_for_monitoring_list(core_response.monitoring_list.id)) //NOLINT (bugprone-use-after-move)
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(monitoring_list_values_response)))));
    EXPECT_CALL(request_mock, add_response_header(::testing::StrEq("Location"), ::testing::_))
        .Times(Exactly(1));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](auto const &res) {
            check_post_monitoring_list_response(res, true);
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_post_monitoring_list_one_time_error)
{
    uint16_t    request_timeout = 0;
    std::string parameter_instance_str = "0-0-a-path";
    std::string request_body    = "{\
                                        \"data\":{\
                                            \"type\":\"monitoring-lists\",\
                                            \"attributes\":{\
                                                \"timeout\":" + std::to_string(static_cast<unsigned>(request_timeout)) + "\
                                            },\
                                            \"relationships\":{\
                                                \"parameters\":{\
                                                    \"data\":[{\
                                                        \"id\":\"" + parameter_instance_str + "\",\
                                                        \"type\":\"parameters\"\
                                                    }]\
                                                }\
                                            }\
                                        }\
                                    }";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::post_monitoring_list;

    EXPECT_CALL(request_mock, get_content())
        .Times(AnyNumber())
        .WillRepeatedly(Return(request_body.c_str()));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_HTTP_EQ(http_status_code::bad_request, res.get_status_code());
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_get_monitoring_lists_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"timeout\":"));
    EXPECT_THAT(response.get_content(), testing::Not(testing::HasSubstr("\"included\":")));
}
}

TEST_F(operation_fixture, operation_execution_get_monitoring_lists)
{
    monitoring_list_id_t const list_id = 123;

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_monitoring_lists;

    monitoring_lists_response core_response;
    core_response.status = wago::wdx::status_codes::success;
    core_response.monitoring_lists = {
        { list_id, false, 123 }
    };

    EXPECT_CALL(core_frontend_mock, get_all_monitoring_lists())
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_monitoring_lists_response)));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_get_monitoring_list_response(response_i const &response,
                                        bool              included_params = false)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"timeout\":"));
    if (included_params)
    {
        EXPECT_THAT(response.get_content(), testing::HasSubstr("\"included\":"));
    }
    else 
    {
        EXPECT_THAT(response.get_content(), testing::Not(testing::HasSubstr("\"included\":")));
    }
}

monitoring_list_response create_monitoring_list_response(monitoring_list_id_t const id,
                                                         uint16_t             const timeout)
{
    monitoring_list_response response = {};
    response.status = wago::wdx::status_codes::success;
    response.monitoring_list.id = id;
    response.monitoring_list.one_off = timeout == 0 ? true : false;
    response.monitoring_list.timeout_seconds = timeout;

    return response;
}
}

TEST_F(operation_fixture, operation_execution_get_monitoring_list)
{
    monitoring_list_id_t const list_id = 123;
    map<string, string> path_parameters_local = {
        {"monitoring_list_id", wago::wda_ipc::to_string(list_id)}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_monitoring_list;

    monitoring_list_response core_response = create_monitoring_list_response(list_id, 60);

    EXPECT_CALL(core_frontend_mock, get_monitoring_list(core_response.monitoring_list.id)) //NOLINT (bugprone-use-after-move)
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](auto const &res) {
            check_get_monitoring_list_response(res, false);
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_delete_monitoring_list_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::no_content, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("", response.get_content().c_str());
}
}

TEST_F(operation_fixture, operation_execution_delete_monitoring_list)
{
    monitoring_list_id_t const list_id = 123;
    map<string, string> path_parameters_local = {
        {"monitoring_list_id", wago::wda_ipc::to_string(list_id)}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer,  path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::delete_monitoring_list;

    delete_monitoring_list_response core_response = delete_monitoring_list_response(wago::wdx::status_codes::success);

    EXPECT_CALL(core_frontend_mock, delete_monitoring_list(list_id))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(&check_delete_monitoring_list_response)));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_monitoring_list_with_include_parameters)
{
    monitoring_list_id_t const list_id = 123;
    map<string, string> path_parameters_local = {
        {"monitoring_list_id", wago::wda_ipc::to_string(list_id)}
    };
    map<string, string> query_parameters_local = {
        {"include", "parameters"}
    };
    query_parameters = query_parameters_local;

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_monitoring_list;

    monitoring_list_response core_response = create_monitoring_list_response(list_id, 60);

    parameter_response param_response;
    param_response.definition = std::make_shared<wdmm::parameter_definition>();
    param_response.value = parameter_value::create("42");
    param_response.id    = parameter_instance_id(100, 1, device_id(0, 0));;
    monitoring_list_values_response monitoring_list_values_response({param_response});

    EXPECT_CALL(core_frontend_mock, get_monitoring_list(core_response.monitoring_list.id)) //NOLINT (bugprone-use-after-move)
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(core_frontend_mock, get_values_for_monitoring_list(core_response.monitoring_list.id))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(monitoring_list_values_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](auto const &res) {
            check_get_monitoring_list_response(res, true);
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_all_enum_definitions)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, {});
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_enum_definitions;

    std::vector<enum_definition_response> core_responses;
    core_responses.push_back(enum_definition_response(status_codes::success));
    core_responses.push_back(enum_definition_response(status_codes::success));
    core_responses.at(0).definition = std::make_shared<wago::wdx::enum_definition>();
    core_responses.at(0).definition->name = "TestEnum";
    core_responses.at(0).definition->members = { {"A", 1} , {"B", 2} };
    core_responses.at(1).definition = std::make_shared<wago::wdx::enum_definition>();
    core_responses.at(1).definition->name = "OtherEnum";
    core_responses.at(1).definition->members = { {"X", 42} };

    EXPECT_CALL(core_frontend_mock, get_all_enum_definitions())
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_resource_collection_response)));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

namespace {
void check_get_enum_definition_response(response_i const &response)
{
    EXPECT_HTTP_EQ(http_status_code::ok, response.get_status_code()) << response.get_content().c_str();
    EXPECT_STREQ("application/vnd.api+json", response.get_content_type().c_str());
    EXPECT_STRNE("", response.get_content().c_str());
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"jsonapi\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"self\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"id\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"type\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"links\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"data\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"name\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"cases\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"value\":"));
    EXPECT_THAT(response.get_content(), testing::HasSubstr("\"stringValue\":"));
}
}

TEST_F(operation_fixture, operation_execution_get_enum_definition)
{
    std::string const enum_def_id = "testenum";
    map<string, string> path_parameters_local = {
        {"enum_definition_id", enum_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_enum_definition;

    enum_definition_response core_response(status_codes::success);
    core_response.definition = std::make_shared<wago::wdx::enum_definition>();
    core_response.definition->name = "TestEnum";
    core_response.definition->members = { {"A", 1} , {"B", 2} };

    EXPECT_CALL(core_frontend_mock, get_enum_definition(enum_def_id))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(check_get_enum_definition_response)));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_enum_definition_not_found)
{
    std::string const enum_def_id = "testenum";
    map<string, string> path_parameters_local = {
        {"enum_definition_id", enum_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_enum_definition;

    enum_definition_response core_response(status_codes::unknown_enum_name);

    EXPECT_CALL(core_frontend_mock, get_enum_definition(enum_def_id))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_all_features)
{
    device_feature_information feature_def;
    feature_def.name = "emptyFeature";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, {});
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_features;

    std::vector<feature_list_response> core_responses;
    core_responses.push_back(feature_list_response(status_codes::success));
    core_responses.push_back(feature_list_response(status_codes::success));
    core_responses.at(0).device_path = "0-0";
    core_responses.at(0).features.push_back(feature_def);
    core_responses.at(1).device_path = "1-1";

    EXPECT_CALL(core_frontend_mock, get_features_of_all_devices())
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke(&check_get_resource_collection_response)));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_feature)
{
    device_path_t const device_path    = "0-0";
    std::string   const feature        = "testFeature";
    std::string   const feature_def_id = device_path + "-" + feature;
    map<string, string> path_parameters_local = {
        {"feature_id", feature_def_id}
    };
    device_feature_information feature_def;
    feature_def.name = "testFeature";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_feature;

    feature_response core_response(status_codes::success);
    core_response.device_path = device_path;
    core_response.feature     = feature_def;

    EXPECT_CALL(core_frontend_mock, get_feature_definition(device_path, feature))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_feature_not_found)
{
    device_path_t const device_path    = "0-0";
    std::string   const feature        = "testFeature";
    std::string   const feature_def_id = device_path + "-" + feature;
    map<string, string> path_parameters_local = {
        {"feature_id", feature_def_id}
    };
    device_feature_information feature_def;
    feature_def.name = "testFeature";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_feature;

    feature_response core_response(status_codes::unknown_feature_name);
    core_response.device_path = device_path;
    core_response.feature     = feature_def;

    EXPECT_CALL(core_frontend_mock, get_feature_definition(device_path, feature))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_feature_bad_id)
{
    device_path_t const device_path    = "0-0";
    std::string   const feature        = "testFeature";
    std::string   const feature_def_id = device_path + "x" + feature;
    map<string, string> path_parameters_local = {
        {"feature_id", feature_def_id}
    };
    device_feature_information feature_def;
    feature_def.name = "testFeature";

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_feature;

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

struct arg_info {
    std::string                      arg_name;
    wago::wdx::parameter_value_types data_type;
    wago::wdx::parameter_value_rank  data_rank;

    operator wago::wdx::method_argument_definition() const
    {
        wago::wdx::method_argument_definition arg_def;
        arg_def.name = arg_name;
        arg_def.value_type = data_type;
        arg_def.value_rank = data_rank;
        return arg_def;
    }
};

static std::shared_ptr<wago::wdx::method_definition> create_method_definition(std::initializer_list<arg_info> const &in_args,
                                                                              std::initializer_list<arg_info> const &out_args)
{
    auto meth_def = std::make_shared<wago::wdx::method_definition>();
    meth_def->value_type = wago::wdx::parameter_value_types::method;
    meth_def->value_rank = wago::wdx::parameter_value_rank::scalar;
    meth_def->in_args = std::vector<wago::wdx::method_argument_definition>();
    meth_def->in_args.insert(meth_def->in_args.begin(), in_args.begin(), in_args.end());
    meth_def->out_args = std::vector<wago::wdx::method_argument_definition>();
    meth_def->out_args.insert(meth_def->out_args.begin(), out_args.begin(), out_args.end());
    return meth_def;
}

static wago::wdx::parameter_response create_method_definition_parameter_response(std::string                     const &method_definition_id,
                                                                                 std::initializer_list<arg_info> const &in_args,
                                                                                 std::initializer_list<arg_info> const &out_args)
{
    parameter_response response(wago::wdx::status_codes::success);
    response.path = wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_definition_id);
    response.definition = create_method_definition(in_args, out_args);
    return response;
}

TEST_F(operation_fixture, operation_execution_get_all_method_definitions)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, {});
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_method_definitions;

    parameter_response_list_response core_response(wago::wdx::status_codes::success);
    core_response.param_responses = {
        create_method_definition_parameter_response(
            "0-0-test-method", 
            {
                { "A", wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            }),
        create_method_definition_parameter_response(
            "0-0-other-method", 
            {
                { "X", wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "Y", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "Z", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };
    core_response.total_entries = 42;

    EXPECT_CALL(core_frontend_mock, get_all_method_definitions(::testing::_, ::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":["));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"method-definitions\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_method_definition)
{
    std::string const method_def_id = "0-0-test-method";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { "A", wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":{"));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"method-definitions\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_method_definition_not_found)
{
    std::string const method_def_id = "0-0-test-method";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path)
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_all_method_inarg_definitions)
{
    std::string const method_def_id = "0-0-test-method";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_method_inarg_definitions;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { "A", wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"method-inarg-definitions\""));
            EXPECT_NE(std::string::npos, res_body.find("\"name\":\"A\""));
            EXPECT_NE(std::string::npos, res_body.find("\"name\":\"B\""));
            EXPECT_EQ(std::string::npos, res_body.find("\"name\":\"C\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_method_inarg_definition)
{
    std::string const method_def_id = "0-0-test-method";
    std::string const inarg_name    = "A";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id},
        {"method_inarg_name",    inarg_name   }
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_inarg_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { inarg_name, wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([inarg_name](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"method-inarg-definitions\""));
            EXPECT_NE(std::string::npos, res_body.find("\"name\":\"" + inarg_name + "\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_method_inarg_definition_not_found)
{
    std::string const method_def_id = "0-0-test-method";
    std::string const inarg_name    = "A";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id},
        {"method_inarg_name",    inarg_name   }
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_inarg_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_all_method_outarg_definitions)
{
    std::string const method_def_id = "0-0-test-method";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_method_outarg_definitions;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { "A", wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"method-outarg-definitions\""));
            EXPECT_NE(std::string::npos, res_body.find("\"name\":\"C\""));
            EXPECT_EQ(std::string::npos, res_body.find("\"name\":\"A\""));
            EXPECT_EQ(std::string::npos, res_body.find("\"name\":\"B\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_method_outarg_definition)
{
    std::string const method_def_id = "0-0-test-method";
    std::string const outarg_name    = "C";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id},
        {"method_outarg_name",    outarg_name   }
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_outarg_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { "A", wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar },
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { outarg_name, wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([outarg_name](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"method-outarg-definitions\""));
            EXPECT_NE(std::string::npos, res_body.find("\"name\":\"" + outarg_name + "\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_method_outarg_definition_not_found)
{
    std::string const method_def_id = "0-0-test-method";
    std::string const outarg_name    = "A";
    map<string, string> path_parameters_local = {
        {"method_definition_id", method_def_id},
        {"method_outarg_name",   outarg_name  }
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_method_outarg_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
            create_method_definition_parameter_response(
            method_def_id, 
            {
                { "B", wago::wdx::parameter_value_types::string,  wago::wdx::parameter_value_rank::array  }
            }, {
                { "C", wago::wdx::parameter_value_types::bytes,   wago::wdx::parameter_value_rank::scalar }
            })
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(method_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}


static std::shared_ptr<wago::wdx::parameter_definition> create_parameter_definition(wago::wdx::parameter_value_types const &type,
                                                                                    wago::wdx::parameter_value_rank  const &rank)
{
    auto param_def = std::make_shared<wago::wdx::parameter_definition>();
    param_def->value_type = type;
    param_def->value_rank = rank;
    return param_def;
}

static wago::wdx::parameter_response create_parameter_definition_parameter_response(std::string                      const &param_definition_id,
                                                                                    wago::wdx::parameter_value_types const &type,
                                                                                    wago::wdx::parameter_value_rank  const &rank)
{
    parameter_response response(wago::wdx::status_codes::success);
    response.path = wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_definition_id);
    response.definition = create_parameter_definition(type, rank);
    return response;
}

static wago::wdx::parameter_response create_parameter_value_response(std::string                                 const &param_id,
                                                                     std::shared_ptr<wago::wdx::parameter_value> const &value)
{
    parameter_response response(wago::wdx::status_codes::success);
    response.path = wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id);
    response.definition = create_parameter_definition(value->get_type(), value->get_rank());
    response.value = value;
    return response;
}

TEST_F(operation_fixture, operation_execution_get_all_parameter_definitions)
{
    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, {});
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_all_parameter_definitions;

    parameter_response_list_response core_response(wago::wdx::status_codes::success);
    core_response.param_responses = {
        create_parameter_definition_parameter_response(
            "0-0-test-param", 
            wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar
        ),
        create_parameter_definition_parameter_response(
            "0-0-other-param", 
            wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar
        )
    };
    core_response.total_entries = 42;

    EXPECT_CALL(core_frontend_mock, get_all_parameter_definitions(::testing::_, ::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_response)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":["));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"parameter-definitions\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_parameter_definition)
{
    std::string const param_def_id = "0-0-test-param";
    map<string, string> path_parameters_local = {
        {"parameter_definition_id", param_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_parameter_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
        create_parameter_definition_parameter_response(
            param_def_id, 
            wago::wdx::parameter_value_types::boolean, wago::wdx::parameter_value_rank::scalar
        )
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":{"));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"parameter-definitions\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_parameter_definition_not_found)
{
    std::string const parameter_def_id = "0-0-test-param";
    map<string, string> path_parameters_local = {
        {"parameter_definition_id", parameter_def_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_parameter_definition;

    std::vector<wago::wdx::parameter_response> core_responses = {
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path)
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(parameter_def_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameter_definitions_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_class_instances)
{
    std::string const param_id = "0-0-test-class";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_instantiations({class_instantiation(42, "TestClass")})
        )
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":["));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"instances\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}


TEST_F(operation_fixture, operation_execution_get_class_instances_not_found_wrong_type)
{
    std::string const param_id = "0-0-test-class";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_boolean(false)
        )
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}


TEST_F(operation_fixture, operation_execution_get_class_instances_not_found_unknown_param)
{
    std::string const param_id = "0-0-test-class";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses = {
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path)
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_class_instance)
{
    std::string const param_id    = "0-0-test-class";
    std::string const instance_no = "42";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id},
        {"instance_no",  instance_no}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instance;

    std::vector<wago::wdx::parameter_response> core_responses = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_instantiations({class_instantiation(42, "TestClass")})
        )
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":{"));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"instances\""));
            EXPECT_NE(std::string::npos, res_body.find("\"instanceNo\":42"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_class_instance_not_found)
{
    std::string const param_id = "0-0-test-class";
    std::string const instance_no = "1337"; // not included in core response
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id},
        {"instance_no",  instance_no}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instance;

    std::vector<wago::wdx::parameter_response> core_responses = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_instantiations({class_instantiation(42, "TestClass")})
        )
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_class_instance_not_found_wrong_type)
{
    std::string const param_id = "0-0-test-class";
    std::string const instance_no = "42";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id},
        {"instance_no",  instance_no}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instance;

    std::vector<wago::wdx::parameter_response> core_responses = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_boolean(false)
        )
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_class_instance_not_found_unknown_param)
{
    std::string const param_id = "0-0-test-class";
    std::string const instance_no = "42";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id},
        {"instance_no",  instance_no}
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_class_instance;

    std::vector<wago::wdx::parameter_response> core_responses = {
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path)
    };

    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };
    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses)))));
    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}


TEST_F(operation_fixture, operation_execution_get_instance_references_single_instance)
{
    std::string const param_id = "0-0-test-param";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };
    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_referenced_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses_for_ref_param = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_instance_identity_ref("TestClass/Path", 42)
        )
    };

    std::string const referenced_param_id = "0-0-testclass-path"; 
    std::vector<wago::wdx::parameter_instance_path> const referenced_class_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(referenced_param_id)
    };
    std::vector<wago::wdx::parameter_response> core_responses_for_instances = {
        create_parameter_value_response(
            referenced_param_id,
            wago::wdx::parameter_value::create_instantiations({class_instantiation(42, "TestClass")})
        )
    };

    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses_for_ref_param)))));
    

    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(referenced_class_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses_for_instances)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":["));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"instances\""));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_instance_references_mixed_result)
{
    std::string const param_id = "0-0-test-param";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };
    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_referenced_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses_for_ref_param = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_instance_identity_ref_array(
                {
                    std::pair<std::string, instance_id_t>({"Test/ClassPath",        42}),
                    std::pair<std::string, instance_id_t>({"",                      0}), // explicitly nulled
                    std::pair<std::string, instance_id_t>({"Test/ClassPath",      1337}), // doesn't exist
                    std::pair<std::string, instance_id_t>({"OtherTestClassPath",   42})  // doesn't exist
                }
            )
        )
    };

    std::string const referenced_param_id_1 = "0-0-test-classpath"; 
    std::string const referenced_param_id_2 = "0-0-"; 
    std::string const referenced_param_id_3 = "0-0-othertestclasspath"; 
    std::vector<wago::wdx::parameter_instance_path> const referenced_class_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(referenced_param_id_1),
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(referenced_param_id_2),
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(referenced_param_id_1),
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(referenced_param_id_3)
    };
    std::vector<wago::wdx::parameter_response> core_responses_for_instances = {
        create_parameter_value_response(
            referenced_param_id_1,
            wago::wdx::parameter_value::create_instantiations({class_instantiation(42, "TestClass")})
        ),
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path), // (empty path)
        create_parameter_value_response(
            referenced_param_id_1,
            wago::wdx::parameter_value::create_instantiations({class_instantiation(42, "TestClass")})
        ),
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path)  // OtherTestClassPath
    };

    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses_for_ref_param)))));
    

    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(referenced_class_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses_for_instances)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::ok, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"data\":["));
            EXPECT_NE(std::string::npos, res_body.find("\"type\":\"instances\""));
            EXPECT_EQ(std::string::npos, res_body.find("null"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_instance_references_wrong_type)
{
    std::string const param_id = "0-0-test-param";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };
    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_referenced_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses_for_ref_param = {
        create_parameter_value_response(
            param_id,
            wago::wdx::parameter_value::create_boolean(true)
        )
    };

    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses_for_ref_param)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

TEST_F(operation_fixture, operation_execution_get_instance_references_parameter_not_found)
{
    std::string const param_id = "0-0-test-param";
    map<string, string> path_parameters_local = {
        {"parameter_id", param_id}
    };
    std::vector<wago::wdx::parameter_instance_path> const instance_paths = { 
        wago::wda_ipc::from_string<wago::wdx::parameter_instance_path>(param_id)
    };

    request rest_request(std::move(request_mock_to_move), json_api_serializer, json_api_serializer, path_parameters_local);
    operation test_operation(service_identity_mock, std::move(core_frontend_mock_ptr), {}, run_manager_mock_ptr);
    operation_handler_t handler = &operation::get_referenced_class_instances;

    std::vector<wago::wdx::parameter_response> core_responses_for_ref_param = {
        wago::wdx::parameter_response(wago::wdx::status_codes::unknown_parameter_path)
    };

    EXPECT_CALL(core_frontend_mock, get_parameters_by_path(::testing::ContainerEq(instance_paths)))
        .Times(Exactly(1))
        .WillRepeatedly(Return(ByMove(resolved_future(std::move(core_responses_for_ref_param)))));

    EXPECT_CALL(request_mock, respond_mock(::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArgs<0>(Invoke([](response_i const &res) {
            EXPECT_EQ(http_status_code::not_found, res.get_status_code());
            auto res_body = res.get_content();
            EXPECT_NE(std::string::npos, res_body.find("\"errors\":["));
            EXPECT_EQ(std::string::npos, res_body.find("\"errors\":[]"));
        })));

    // test code execution
    {
        test_operation.handle(handler, std::move(rest_request));
    }
}

//---- End of source file ------------------------------------------------------
