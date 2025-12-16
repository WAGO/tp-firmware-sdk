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
///  \brief    Test functionality of parameter / method provider
///
///  \author   FHa : WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/auth_service_provider.hpp"
#include "mocks/mock_settings_store.hpp"
#include "mocks/mock_oauth_token_handler.hpp"

#include <iostream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
using wago::authserv::auth_service_provider;
using wago::authserv::settings_store_i;
using testing::AtLeast;

namespace {
const std::map<settings_store_i::global_setting, unsigned int> param_id = {
    {settings_store_i::auth_code_lifetime, 6000},
    {settings_store_i::access_token_lifetime, 6001},
    {settings_store_i::refresh_token_lifetime, 6002},
    {settings_store_i::silent_mode_enabled, 6005}
};
struct get_param_request
{
    std::vector<wago::wdx::parameter_instance_id> arg;
    std::vector<wago::wdx::value_response> expected_response;
};
class get_param
{
public:
    get_param(settings_store_i::global_setting param) : id(param_id.at(param)) { }
    get_param& expect(wago::wdx::value_response result)
    {
        response = result;
        return *this;
    }
    get_param& expect_uint64(uint64_t result)
    {
        response.set_value(wago::wdx::parameter_value::create_uint64(result));
        return *this;
    }
    get_param& expect_boolean(bool result)
    {
        response.set_value(wago::wdx::parameter_value::create_boolean(result));
        return *this;
    }
    get_param& expect_internal_error(std::string error_message = "")
    {
        response.set_error(wago::wdx::status_codes::internal_error, error_message);
        return *this;
    }
private:
    wago::wdx::parameter_instance_id id;
    wago::wdx::value_response response;
    friend class get_param_arg_builder;
};
class get_param_arg_builder
{
public:
    get_param_arg_builder& addParameter(get_param param)
    {
        params.push_back(param);
        return *this;
    }
    get_param_arg_builder& addParameters(std::vector<get_param> params_p)
    {
        for(get_param &param : params_p) { addParameter(param); }
        return *this;
    }
    get_param_request const build()
    {
        get_param_request res;
        for(get_param param : params)
        {
            res.arg.push_back(param.id);
            res.expected_response.push_back(param.response);
        }
        return res;
    }
private:
    std::vector<get_param> params;
};

struct set_param_request
{
    std::vector<wago::wdx::value_request> arg;
    wago::authserv::settings_store_i::global_config_map settings_store_request;
    wago::authserv::settings_store_i::configuration_error_map settings_store_response;
    std::vector<wago::wdx::set_parameter_response> expected_response;
};

class set_param
{
public:
    set_param(settings_store_i::global_setting param_arg, std::shared_ptr<wago::wdx::parameter_value> value)
    : param(param_arg)
    , request(wago::wdx::parameter_instance_id(param_id.at(param)), value)
    , value_rejected(false)
    , error_message("")
    {
        try {
            value_str = value->get_boolean() ? "true" : "false";
        }
        catch (...) {
            value_str = std::to_string(value->get_uint64());
        }
    }
    static set_param build_bool_setter(settings_store_i::global_setting param_arg, bool value)
    {
        return set_param(param_arg, wago::wdx::parameter_value::create_boolean(value));
    }
    static set_param build_uint64_setter(settings_store_i::global_setting param_arg, uint64_t value)
    {
        return set_param(param_arg, wago::wdx::parameter_value::create_uint64(value));
    }
    set_param &expect_success()
    {
        response.set_success();
        return *this;
    }
    set_param &expect_value_rejection(std::string const &message = "")
    {
        value_rejected = true;
        error_message = message;
        response.set_error(wago::wdx::status_codes::invalid_value, message);
        return *this;
    }
    set_param &expect_reject_because_of_other_value()
    {
        response.set_error(wago::wdx::status_codes::other_invalid_value_in_set);
        return *this;
    }
    set_param &expect_error(std::string message = "")
    {
        response.set_error(wago::wdx::status_codes::internal_error, message);
        return *this;
    }
private:
    settings_store_i::global_setting param;
    std::string value_str;
    wago::wdx::value_request request;
    bool value_rejected;
    std::string error_message;
    wago::wdx::set_parameter_response response;
    friend class set_param_arg_builder;
};

class set_param_arg_builder
{
public:
    set_param_arg_builder& addParameter(set_param param)
    {
        params.push_back(param);
        return *this;
    }
    set_param_arg_builder& addParameters(std::vector<set_param> params_p)
    {
        for(set_param &param : params_p) { addParameter(param); }
        return *this;
    }
    set_param_request const build()
    {
        set_param_request res;
        for(set_param param : params)
        {
            res.arg.push_back(param.request);
            res.settings_store_request.emplace(param.param, param.value_str);
            if(param.value_rejected) {
                res.settings_store_response.emplace(param.param, param.error_message);
            }
            res.expected_response.push_back(param.response);
        }
        return res;
    }
private:
    std::vector<set_param> params;
};
} // namespace anonym

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
namespace wago{
namespace wdx{

bool operator==(const response_with_value &first, const response_with_value &second) noexcept;
std::ostream& operator<< (std::ostream& out, const value_response &value_response);
std::ostream& operator<< (std::ostream& out, const set_parameter_response &value_response);

bool operator==(const response_with_value &first, const response_with_value &second) noexcept
{
    if (first.status != second.status) { return false; }
    if (first.message != second.message) { return false; }
    if (first.domain_specific_status_code != second.domain_specific_status_code) { return false; }

    if (first.value == nullptr && second.value == nullptr) { return true; }
    if (first.value != nullptr && second.value != nullptr)
    {
        return *first.value == *second.value;
    }
    return false;
}
std::ostream& operator<< (std::ostream& out, const value_response &value_response)
{
    return out << "{status: " << to_string(value_response.status)
        << "; message: " << value_response.message
        << "; specific status code: " << value_response.domain_specific_status_code
        << "; value: " << (value_response.status == status_codes::success
                                ? value_response.value->get_json()
                                : "unset"
        ) << "}";
}
std::ostream& operator<< (std::ostream& out, const set_parameter_response &value_response)
{
    return out << "{status: " << to_string(value_response.status)
        << "; message: " << value_response.message
        << "; specific status code: " << value_response.domain_specific_status_code
        << "; value: " << (value_response.status == status_codes::success_but_value_adjusted
                                ? value_response.value->get_json()
                                : "unset"
        ) << "}";
}
} // namespace wdx
} // namespace wago


class pp_auth_service_fixture: public ::testing::Test
{
protected:
    std::shared_ptr<mock_settings_store> settings_store_mock;
    std::shared_ptr<mock_oauth_token_handler> oauth_token_handler_mock;

    pp_auth_service_fixture()
    : settings_store_mock(std::make_shared<mock_settings_store>())
    , oauth_token_handler_mock(std::make_shared<mock_oauth_token_handler>())
    { }
    ~pp_auth_service_fixture() override = default;

    void SetUp() override
    {
        settings_store_mock->set_default_expectations();
        oauth_token_handler_mock->set_default_expectations();
    }
};

TEST_F(pp_auth_service_fixture, get_parameter)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);
    uint64_t auth_code_test_value = 42;

    get_param_request request = get_param_arg_builder().addParameters({
        get_param(settings_store_i::auth_code_lifetime).expect_uint64(auth_code_test_value),
    }).build();
    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::auth_code_lifetime, std::to_string(42));
    auto future = provider.get_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, get_multiple_parameters)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);
    uint64_t auth_code_test_value = 60;
    uint64_t access_token_test_value = 120;
    uint64_t refresh_token_test_value = 240;
    bool silent_mode_test_value = true;

    std::string auth_code_test_value_str = std::to_string(auth_code_test_value);
    std::string access_token_test_value_str = std::to_string(access_token_test_value);
    std::string refresh_token_test_value_str = std::to_string(refresh_token_test_value);
    std::string silent_mode_test_value_str = silent_mode_test_value ? "true" : "false";

    get_param_request request = get_param_arg_builder().addParameters({
        get_param(settings_store_i::auth_code_lifetime).expect_uint64(auth_code_test_value),
        get_param(settings_store_i::access_token_lifetime).expect_uint64(access_token_test_value),
        get_param(settings_store_i::refresh_token_lifetime).expect_uint64(refresh_token_test_value),
        get_param(settings_store_i::silent_mode_enabled).expect_boolean(silent_mode_test_value)
    }).build();

    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::auth_code_lifetime, auth_code_test_value_str);
    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::access_token_lifetime, access_token_test_value_str);
    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::refresh_token_lifetime, refresh_token_test_value_str);
    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::silent_mode_enabled, silent_mode_test_value_str);

    auto future = provider.get_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, error_on_parameter_get)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);
    uint64_t access_token_test_value = 120;
    uint64_t refresh_token_test_value = 240;
    std::string access_token_test_value_str = std::to_string(access_token_test_value);
    std::string refresh_token_test_value_str = std::to_string(refresh_token_test_value);

    get_param_request request = get_param_arg_builder().addParameters({
        get_param(settings_store_i::auth_code_lifetime).expect_internal_error(),
        get_param(settings_store_i::access_token_lifetime).expect_uint64(access_token_test_value),
        get_param(settings_store_i::refresh_token_lifetime).expect_uint64(refresh_token_test_value)
    }).build();

    EXPECT_CALL(*settings_store_mock, get_global_setting(wago::authserv::settings_store_i::auth_code_lifetime))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Throw(std::runtime_error("internal error on auth code read")));
    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::access_token_lifetime, access_token_test_value_str);
    settings_store_mock->expect_global_setting_read(wago::authserv::settings_store_i::refresh_token_lifetime, refresh_token_test_value_str);

    auto future = provider.get_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, set_uint64_parameter)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);

    set_param_request request = set_param_arg_builder().addParameters({
        set_param::build_uint64_setter(settings_store_i::auth_code_lifetime, 42).expect_success()
    }).build();

    settings_store_mock->expect_global_setting_write(request.settings_store_request, request.settings_store_response);

    auto future = provider.set_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, set_bool_parameter)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);

    set_param_request request = set_param_arg_builder().addParameters({
        set_param::build_bool_setter(settings_store_i::silent_mode_enabled, true).expect_success()
    }).build();

    settings_store_mock->expect_global_setting_write(request.settings_store_request, request.settings_store_response);

    auto future = provider.set_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, set_multiple_parameters)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);

    set_param_request request = set_param_arg_builder().addParameters({
        set_param::build_uint64_setter(settings_store_i::auth_code_lifetime,      60).expect_success(),
        set_param::build_uint64_setter(settings_store_i::access_token_lifetime,  120).expect_success(),
        set_param::build_uint64_setter(settings_store_i::refresh_token_lifetime, 240).expect_success(),
        set_param::build_bool_setter(  settings_store_i::silent_mode_enabled,   true).expect_success()
    }).build();

    settings_store_mock->expect_global_setting_write(request.settings_store_request, request.settings_store_response);

    auto future = provider.set_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, set_invalid_parameters)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);

    set_param_request request = set_param_arg_builder().addParameters({
        set_param::build_uint64_setter(settings_store_i::auth_code_lifetime,      60).expect_value_rejection(),
        set_param::build_uint64_setter(settings_store_i::access_token_lifetime,   60).expect_value_rejection(),
        set_param::build_uint64_setter(settings_store_i::refresh_token_lifetime, 240).expect_reject_because_of_other_value()
    }).build();

    settings_store_mock->expect_global_setting_write(request.settings_store_request, request.settings_store_response);

    auto future = provider.set_parameter_values(request.arg);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_EQ(future.get(), request.expected_response);
}

TEST_F(pp_auth_service_fixture, call_revoke_all_tokens)
{
    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);

    EXPECT_CALL(*oauth_token_handler_mock, revoke_all_tokens())
        .Times(testing::Exactly(1));

    std::vector<std::shared_ptr<wago::wdx::parameter_value>> in_args;

    auto future = provider.invoke_method(wago::wdx::parameter_instance_id(6003), in_args);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_TRUE(future.get().is_success());
}

TEST_F(pp_auth_service_fixture, call_revoke_spezific_token)
{
    std::string const token = "revokable_token";

    auth_service_provider provider(settings_store_mock, oauth_token_handler_mock);

    EXPECT_CALL(*oauth_token_handler_mock, revoke_token(token))
        .Times(testing::Exactly(1));

    std::vector<std::shared_ptr<wago::wdx::parameter_value>> in_args;
    in_args.push_back(wago::wdx::parameter_value::create(token));

    auto future = provider.invoke_method(wago::wdx::parameter_instance_id(6004), in_args);
    future.wait();
    EXPECT_TRUE(future.has_value());
    EXPECT_TRUE(future.get().is_success());
}

//---- End of source file ------------------------------------------------------
