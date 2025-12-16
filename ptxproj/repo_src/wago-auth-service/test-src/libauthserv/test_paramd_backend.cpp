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
///  \brief    Test Paramd password change backend.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "paramd_backend.hpp"

#include <wago/wdx/test/mock_frontend.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::paramd_backend;
using testing::WithArg;
using testing::WithoutArgs;
using testing::Exactly;
using testing::AnyNumber;
using testing::Return;
using testing::Invoke;
using testing::HasSubstr;

namespace { 
std::string const all_local_users_path                   = "LocalUsers";
std::string const name_subpath                           = "Name";
std::string const change_password_subpath                = "ChangePassword";
std::string const accountmanagement_change_password_path = "AccountManagement/ChangePassword";
}

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
class paramd_backend_fixture : public ::testing::Test
{
protected:
    wago::wdx::test::mock_frontend frontend_mock;

    paramd_backend_fixture() = default;
    ~paramd_backend_fixture() override = default;

    void SetUp() override
    {
        frontend_mock.set_default_expectations();
    }

    static wago::future<wago::wdx::parameter_response_list_response> create_all_parameters_response(std::string const &test_user,
                                                                                                    uint16_t    const &test_user_id,
                                                                                                    std::string const &test_instantiation_path,
                                                                                                    std::string const &test_local_users_path)
    {
        // instantiation parameter
        wago::wdx::parameter_instance_path instantiation_path;
        instantiation_path.parameter_path = test_instantiation_path;

        wago::wdx::parameter_response instantiation_parameter;
        instantiation_parameter.path = instantiation_path;
        instantiation_parameter.status = wago::wdx::status_codes::success;

        // parameter
        std::shared_ptr<wago::wdx::parameter_value> parameter_value = wago::wdx::parameter_value::create_string(test_user);

        wago::wdx::parameter_instance_id id;
        id.instance_id = test_user_id;

        wago::wdx::parameter_instance_path path;
        path.parameter_path = test_local_users_path;

        wago::wdx::parameter_response parameter;
        parameter.value = parameter_value;
        parameter.id = id;
        parameter.path = path;
        parameter.status = wago::wdx::status_codes::success;

        // parameter list
        std::vector<wago::wdx::parameter_response> list;
        list.push_back(instantiation_parameter);
        list.push_back(parameter);

        wago::wdx::parameter_response_list_response parameter_list;
        parameter_list.param_responses = list;
        parameter_list.status = wago::wdx::status_codes::success;

        return wago::resolved_future(std::move(parameter_list));
    }

    static wago::future<wago::wdx::parameter_response_list_response> create_parameter_not_provided_response()
    {
        wago::wdx::parameter_instance_id id;
        id.instance_id = 1111;

        wago::wdx::parameter_instance_path path;
        path.parameter_path = all_local_users_path;

        wago::wdx::parameter_response parameter;
        parameter.id = id;
        parameter.path = path;
        parameter.status = wago::wdx::status_codes::parameter_not_provided;

        std::vector<wago::wdx::parameter_response> list;
        list.push_back(parameter);

        wago::wdx::parameter_response_list_response parameter_list;
        parameter_list.param_responses = list;
        parameter_list.status = wago::wdx::status_codes::success;

        return wago::resolved_future(std::move(parameter_list));
    }


    static wago::future<wago::wdx::parameter_response_list_response> create_parameterservice_error_response()
    {
        wago::wdx::parameter_response_list_response parameter_list;
        parameter_list.status = wago::wdx::status_codes::internal_error;

        return wago::resolved_future(std::move(parameter_list));
    }

    static wago::future<wago::wdx::method_invocation_named_response> create_password_change_response(std::string error_msg = "")
    {
        wago::wdx::method_invocation_named_response response;
        response.status  = error_msg.empty() ? wago::wdx::status_codes::success : wago::wdx::status_codes::could_not_invoke_method;
        response.message = error_msg;

        wago::promise<wago::wdx::method_invocation_named_response> promise;
        promise.set_value(std::move(response));
        return promise.get_future();
    }
};

TEST_F(paramd_backend_fixture, construct_delete)
{
    paramd_backend paramd_via_passwordmanagement(paramd_backend::wdx_user_model::password_management, frontend_mock);
    paramd_backend paramd_via_accountmanagement(paramd_backend::wdx_user_model::account_management, frontend_mock);
}

TEST_F(paramd_backend_fixture, change_password_via_accountmanagement_succeeds)
{
    char        const test_user[]           = "testuser";
    char        const test_password[]       = "testpassword";
    char        const test_new_password[]   = "newtestpassword";

    wago::wdx::parameter_instance_path const expected_method_path = accountmanagement_change_password_path;
    EXPECT_CALL(frontend_mock, invoke_method_by_path(expected_method_path, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<1>(Invoke([test_user, test_password, test_new_password]
                (std::map<std::string, std::shared_ptr<wago::wdx::parameter_value>> in_args) {
            std::string username_param = (*in_args.find("Username")->second).get_string();
            EXPECT_EQ(username_param, test_user);
            std::string old_password_param = (*in_args.find("OldPassword")->second).get_string();
            EXPECT_EQ(old_password_param, test_password);
            std::string new_password_param = (*in_args.find("NewPassword")->second).get_string();
            EXPECT_EQ(new_password_param, test_new_password);
            //result
            return create_password_change_response();
        })));

    paramd_backend paramd(paramd_backend::wdx_user_model::account_management, frontend_mock);

    paramd.change_password(test_user, test_password, test_new_password);
}


TEST_F(paramd_backend_fixture, change_password_via_accountmanagement_method_invocation_fails)
{
    char        const test_user[]           = "testuser";
    char        const test_wrong_password[] = "wrongpassword";
    char        const test_new_password[]   = "newpassword";

    wago::wdx::parameter_instance_path const expected_method_path = accountmanagement_change_password_path;
    EXPECT_CALL(frontend_mock, invoke_method_by_path(expected_method_path, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<1>(Invoke([test_user, test_wrong_password, test_new_password]
                (std::map<std::string, std::shared_ptr<wago::wdx::parameter_value>> in_args) {
            std::string username_param = (*in_args.find("Username")->second).get_string();
            EXPECT_EQ(username_param, test_user);
            std::string old_password_param = (*in_args.find("OldPassword")->second).get_string();
            EXPECT_EQ(old_password_param, test_wrong_password);
            std::string new_password_param = (*in_args.find("NewPassword")->second).get_string();
            EXPECT_EQ(new_password_param, test_new_password);
            //result
            return create_password_change_response("an internal error occured");
        })));

    paramd_backend paramd(paramd_backend::wdx_user_model::account_management, frontend_mock);

    wago::future<void> result = paramd.change_password(test_user, test_wrong_password, test_new_password);
    result.wait();
    EXPECT_TRUE(result.has_exception()) << "Password change should error when the parameter is not provided.";
    try
    {
        std::rethrow_exception(result.get_exception());
    }
    catch (wago::authserv::password_change_error &e)
    {
        EXPECT_THAT(e.what(), HasSubstr("change password"));
        EXPECT_THAT(e.what(), HasSubstr("retry"));
        EXPECT_THAT(e.what(), HasSubstr("contact your administrator"));
    }
}

TEST_F(paramd_backend_fixture, change_password_via_password_management_succeeds)
{
    char        const test_user[]           = "testuser";
    uint16_t    const test_user_id          = 1234;
    std::string const test_local_users_path = all_local_users_path + "/" + std::to_string((int)test_user_id) + "/" + name_subpath;
    char        const test_password[]       = "testpassword";
    char        const test_new_password[]   = "newtestpassword";

    EXPECT_CALL(frontend_mock, get_all_parameters(
        wago::wdx::parameter_filter::only_subpath(all_local_users_path) | wago::wdx::parameter_filter::without_methods(),
        ::testing::_, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithoutArgs(Invoke([test_user, test_user_id, test_local_users_path]() {
            return create_all_parameters_response(test_user, test_user_id, all_local_users_path, test_local_users_path);
        })));

    wago::wdx::parameter_instance_path const expected_method_path = all_local_users_path + "/" + std::to_string((int)test_user_id) + "/" + change_password_subpath;
    EXPECT_CALL(frontend_mock, invoke_method_by_path(expected_method_path, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<1>(Invoke([test_password, test_new_password]
                (std::map<std::string, std::shared_ptr<wago::wdx::parameter_value>> in_args) {
            std::string old_password_param = (*in_args.find("OldPassword")->second).get_string();
            EXPECT_EQ(old_password_param, test_password);
            std::string new_password_param = (*in_args.find("NewPassword")->second).get_string();
            EXPECT_EQ(new_password_param, test_new_password);
            //result
            return create_password_change_response();
        })));

    paramd_backend paramd(paramd_backend::wdx_user_model::password_management, frontend_mock);

    paramd.change_password(test_user, test_password, test_new_password);
}

TEST_F(paramd_backend_fixture, change_password_via_password_management_fails_on_wrong_user)
{
    char        const test_user[]           = "testuser";
    char        const test_wrong_user[]     = "otheruser";
    uint16_t    const test_user_id          = 1234;
    std::string const test_local_users_path = all_local_users_path + "/" + std::to_string((int)test_user_id) + "/" + name_subpath;
    char        const test_password[]       = "unused";
    char        const test_new_password[]   = "unused2";

    EXPECT_CALL(frontend_mock, get_all_parameters(
        wago::wdx::parameter_filter::only_subpath(all_local_users_path) | wago::wdx::parameter_filter::without_methods(),
        ::testing::_, ::testing::_))        .Times(Exactly(1))
        .WillRepeatedly(WithoutArgs(Invoke([test_user, test_user_id, test_local_users_path]() {
            return create_all_parameters_response(test_user, test_user_id, all_local_users_path, test_local_users_path);
        })));

    paramd_backend paramd(paramd_backend::wdx_user_model::password_management, frontend_mock);

    try
    {
        paramd.change_password(test_wrong_user, test_password, test_new_password);
        FAIL() << "Password change should error when the user doesn't exist.";
    }
    catch (wago::authserv::password_change_error &e)
    {
        EXPECT_THAT(e.what(), HasSubstr("change password"));
        EXPECT_THAT(e.what(), HasSubstr("retry"));
        EXPECT_THAT(e.what(), HasSubstr("contact your administrator"));
    }
}

TEST_F(paramd_backend_fixture, change_password_via_password_management_method_invocation_fails)
{
    char        const test_user[]           = "testuser";
    uint16_t    const test_user_id          = 1234;
    std::string const test_local_users_path = all_local_users_path + "/" + std::to_string((int)test_user_id) + "/" + name_subpath;
    char        const test_wrong_password[] = "wrongpassword";
    char        const test_new_password[]   = "newpassword";

    EXPECT_CALL(frontend_mock, get_all_parameters(
        wago::wdx::parameter_filter::only_subpath(all_local_users_path) | wago::wdx::parameter_filter::without_methods(),
        ::testing::_, ::testing::_))        .Times(Exactly(1))
        .WillRepeatedly(WithoutArgs(Invoke([test_user, test_user_id, test_local_users_path]() {
            return create_all_parameters_response(test_user, test_user_id, all_local_users_path, test_local_users_path);
        })));

    wago::wdx::parameter_instance_path const expected_method_path = all_local_users_path + "/" + std::to_string((int)test_user_id) + "/" + change_password_subpath;
    EXPECT_CALL(frontend_mock, invoke_method_by_path(expected_method_path, ::testing::_))
        .Times(Exactly(1))
        .WillRepeatedly(WithArg<1>(Invoke([test_wrong_password, test_new_password]
                (std::map<std::string, std::shared_ptr<wago::wdx::parameter_value>> in_args) {
            std::string old_password_param = (*in_args.find("OldPassword")->second).get_string();
            EXPECT_EQ(old_password_param, test_wrong_password);
            std::string new_password_param = (*in_args.find("NewPassword")->second).get_string();
            EXPECT_EQ(new_password_param, test_new_password);
            //result
            return create_password_change_response("an internal error occured");
        })));

    paramd_backend paramd(paramd_backend::wdx_user_model::password_management, frontend_mock);

    wago::future<void> result = paramd.change_password(test_user, test_wrong_password, test_new_password);
    result.wait();
    EXPECT_TRUE(result.has_exception()) << "Password change should error when the parameter is not provided.";
    try
    {
        std::rethrow_exception(result.get_exception());
    }
    catch (wago::authserv::password_change_error &e)
    {
        EXPECT_THAT(e.what(), HasSubstr("change password"));
        EXPECT_THAT(e.what(), HasSubstr("retry"));
        EXPECT_THAT(e.what(), HasSubstr("contact your administrator"));
    }
}

TEST_F(paramd_backend_fixture, change_password_via_password_management_instantiations_error)
{
    char        const test_user[]           = "testuser";
    char        const test_password[]       = "testpassword";
    char        const test_new_password[]   = "newtestpassword";

    EXPECT_CALL(frontend_mock, get_all_parameters(
        wago::wdx::parameter_filter::only_subpath(all_local_users_path) | wago::wdx::parameter_filter::without_methods(),
        ::testing::_, ::testing::_))        .Times(Exactly(1))
        .WillRepeatedly(WithoutArgs(Invoke([]() {
            return create_parameter_not_provided_response();
        })));

    paramd_backend paramd(paramd_backend::wdx_user_model::password_management, frontend_mock);

    wago::future<void> result = paramd.change_password(test_user, test_password, test_new_password);
    result.wait();
    EXPECT_TRUE(result.has_exception()) << "Password change should error when the parameter is not provided.";
    try
    {
        std::rethrow_exception(result.get_exception());
    }
    catch (wago::authserv::password_change_error &e)
    {
        EXPECT_THAT(e.what(), HasSubstr("change password"));
        EXPECT_THAT(e.what(), HasSubstr("retry"));
        EXPECT_THAT(e.what(), HasSubstr("contact your administrator"));
    }
}

TEST_F(paramd_backend_fixture, change_password_via_password_management_paramd_error)
{
    char        const test_user[]           = "testuser";
    char        const test_password[]       = "testpassword";
    char        const test_new_password[]   = "newtestpassword";

    EXPECT_CALL(frontend_mock, get_all_parameters(
        wago::wdx::parameter_filter::only_subpath(all_local_users_path) | wago::wdx::parameter_filter::without_methods(),
        ::testing::_, ::testing::_))        .Times(Exactly(1))
        .WillRepeatedly(WithoutArgs(Invoke([]() {
            return create_parameterservice_error_response();
    })));

    paramd_backend paramd(paramd_backend::wdx_user_model::password_management, frontend_mock);

    wago::future<void> result = paramd.change_password(test_user, test_password, test_new_password);
    result.wait();
    EXPECT_TRUE(result.has_exception()) << "Password change should error when the parameter is not provided.";
    try
    {
        std::rethrow_exception(result.get_exception());
    }
    catch (wago::authserv::password_change_error &e)
    {
        EXPECT_THAT(e.what(), HasSubstr("change password"));
        EXPECT_THAT(e.what(), HasSubstr("retry"));
        EXPECT_THAT(e.what(), HasSubstr("contact your administrator"));
    }
}

//---- End of source file ------------------------------------------------------
