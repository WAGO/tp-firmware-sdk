//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    paramd backend for password change.
///
///  \author   FHa: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "paramd_backend.hpp"
#include <wago/wdx/parameter_service_frontend_i.hpp>

#include <wc/log.h>
#include <regex>

#include <cstdlib>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {

constexpr char const * const paramd_path_accountmanagement_change_password = "Accountmanagement/ChangePassword";
constexpr char const * const paramd_path_all_users                         = "LocalUsers";
constexpr char const * const paramd_subpath_change_password                = "ChangePassword";
constexpr int                paramd_proxy_timeout_millis                   = 30000;
constexpr int                paramd_proxy_fetch_interval_millis            =  1000;

}

class paramd_backend_impl
{
protected:
    wdx::parameter_service_frontend_i &frontend_m;
public:
    paramd_backend_impl(wdx::parameter_service_frontend_i &frontend)
    : frontend_m(frontend)
    { }
    virtual ~paramd_backend_impl() = default;

    virtual wago::future<void> change_password(std::string                       const &user,
                                               std::string                       const &password,
                                               std::string                       const &new_password) const = 0;
};

namespace {

class wdx_passwordmanagement : public paramd_backend_impl
{
public:
    using paramd_backend_impl::paramd_backend_impl;
    ~wdx_passwordmanagement() override = default;

    wago::future<void> change_password(std::string                       const &user,
                                       std::string                       const &password,
                                       std::string                       const &new_password) const override;

private:
    wago::future<wdx::instance_id_t> get_instance_id_from_username(std::string const &username) const;
    wago::future<wdx::method_invocation_named_response> call_password_change_method(wdx::instance_id_t id, std::string old_password, std::string new_password) const;
    wdx::parameter_response find_parameter_response_by_path(wdx::parameter_response_list_response const &response_list, std::string const &path) const;
    wdx::wdmm::instance_id_t find_instance_id_by_name_value(wdx::parameter_response_list_response const &response_list, std::string const &username) const;
};

class wdx_accountmanagement : public paramd_backend_impl
{
public:
    using paramd_backend_impl::paramd_backend_impl;
    ~wdx_accountmanagement() override = default;

    wago::future<void> change_password(std::string                       const &user,
                                       std::string                       const &password,
                                       std::string                       const &new_password) const override;

private:
    wago::future<wdx::method_invocation_named_response> call_password_change_method(std::string user, std::string old_password, std::string new_password) const;
};

}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
paramd_backend::paramd_backend(wdx_user_model                     model, 
                               wdx::parameter_service_frontend_i &frontend)
: impl_m( (model == wdx_user_model::account_management)
        ? std::unique_ptr<paramd_backend_impl>(std::make_unique<wdx_accountmanagement>(frontend))
        : std::unique_ptr<paramd_backend_impl>(std::make_unique<wdx_passwordmanagement>(frontend)) )
{ }

paramd_backend::~paramd_backend() noexcept = default;

wago::future<void> paramd_backend::change_password(std::string const &user,
                                                   std::string const &password,
                                                   std::string const &new_password)
{
    wc_log(info, "Changing password of user \"" + user + "\"");
    return impl_m->change_password(user, password, new_password);
}

wago::future<void> wdx_accountmanagement::change_password(std::string                       const &user,
                                                          std::string                       const &password,
                                                          std::string                       const &new_password) const
{
    auto change_password_result = std::make_shared<wago::promise<void>>();
    auto method_call = call_password_change_method(user, password, new_password);
    method_call.set_exception_notifier(std::bind(&wago::promise<void>::set_exception, change_password_result, std::placeholders::_1));
    method_call.set_notifier([change_password_result, user](wdx::method_invocation_named_response response)
    {
        WC_DEBUG_LOG("Got response on password change method call for user \"" + user + "\"");
        if (!response.is_success())
        {
            wc_log(log_level_t::error, "Encountered error while changing user password for user " +
                                       user + ": " +
                                       std::to_string(static_cast<int>(response.status)) + " " +
                                       wdx::to_string(response.status));
            // TODO forward user input errors in password to frontend - currently not provided by paramd
            change_password_result->set_exception(std::make_exception_ptr(password_change_error("Can't change password at the moment. Please retry. If this is a permanent error contact your administrator.")));
            return;
        }
        change_password_result->set_value();
    });
    return change_password_result->get_future();
}


wago::future<wdx::method_invocation_named_response> wdx_accountmanagement::call_password_change_method(std::string user, std::string old_password, std::string new_password) const
{
    // change to new password
    wdx::parameter_instance_path method_path(paramd_path_accountmanagement_change_password);
    std::map<std::string, std::shared_ptr<wdx::parameter_value>> in_args;
    in_args.emplace("Username", wdx::parameter_value::create(user));
    in_args.emplace("OldPassword", wdx::parameter_value::create(old_password));
    in_args.emplace("NewPassword", wdx::parameter_value::create(new_password));
    return frontend_m.invoke_method_by_path(method_path, in_args);
}

wago::future<void> wdx_passwordmanagement::change_password(std::string                       const &user,
                                                           std::string                       const &password,
                                                           std::string                       const &new_password) const
{
    auto change_password_result = std::make_shared<wago::promise<void>>();
    auto user_id = get_instance_id_from_username(user);
    user_id.set_exception_notifier(std::bind(&wago::promise<void>::set_exception, change_password_result, std::placeholders::_1));
    user_id.set_notifier([this, change_password_result, user, password, new_password](wdx::instance_id_t id) {
        WC_DEBUG_LOG("Got user_id " + std::to_string(static_cast<int>(id)) + " for user \"" + user + "\"");
        auto method_call = call_password_change_method(id, password, new_password);
        method_call.set_exception_notifier(std::bind(&wago::promise<void>::set_exception, change_password_result, std::placeholders::_1));
        method_call.set_notifier([change_password_result, user, id](wdx::method_invocation_named_response response)
        {
            WC_DEBUG_LOG("Got response on password change method call for user \"" + user + "\"");
            if (!response.is_success())
            {
                wc_log(log_level_t::error, "Encountered error while changing user password for user " +
                                           user + "(" + std::to_string(static_cast<int>(id)) + "): " +
                                           std::to_string(static_cast<int>(response.status)) + " " +
                                           wdx::to_string(response.status));
                // TODO forward user input errors in password to frontend - currently not provided by paramd
                change_password_result->set_exception(std::make_exception_ptr(password_change_error("Can't change password at the moment. Please retry. If this is a permanent error contact your administrator.")));
                return;
            }
            change_password_result->set_value();
        });
    });
    return change_password_result->get_future();
}

wago::future<wdx::instance_id_t> wdx_passwordmanagement::get_instance_id_from_username(std::string const &username) const
{
    auto result = std::make_shared<wago::promise<wdx::instance_id_t>>();

    wago::future<wdx::parameter_response_list_response> users_future =
        frontend_m.get_all_parameters(wdx::parameter_filter::only_subpath(paramd_path_all_users) | wdx::parameter_filter::without_methods());
    users_future.set_exception_notifier(std::bind(&wago::promise<wdx::instance_id_t>::set_exception, result, std::placeholders::_1));
    users_future.set_notifier([this, result, username](wdx::parameter_response_list_response users_list)
    {
        WC_DEBUG_LOG("Got response on all parameters request for path \"" + std::string(paramd_path_all_users) + "\"");
        if(!users_list.is_success())
        {
            wc_log(log_level_t::error, "Can't change password: " + std::to_string(static_cast<int>(users_list.status)) + " " +
                                       wdx::to_string(users_list.status) + (users_list.get_message().empty() ? "" : ": ") +
                                       users_list.get_message());
            result->set_exception(std::make_exception_ptr(password_change_error("Can't change password at the moment. Please retry. If this is a permanent error contact your administrator.")));
            return;
        }
        auto instantiations_parameter = find_parameter_response_by_path(users_list, paramd_path_all_users);
        if (!instantiations_parameter.is_success())
        {
            wc_log(log_level_t::error, "Can't change password: " + std::to_string(static_cast<int>(users_list.status)) + " " +
                                       wdx::to_string(users_list.status) + ": \"" + instantiations_parameter.path.parameter_path + "\"" +
                                       (users_list.get_message().empty() ? "" : ": ") + users_list.get_message());
            result->set_exception(std::make_exception_ptr(password_change_error("Can't change password at the moment. Please retry. If this is a permanent error contact your administrator.")));
            return;
        }
        result->set_value(find_instance_id_by_name_value(users_list, username));
    });
    return result->get_future();
}

wago::future<wdx::method_invocation_named_response> wdx_passwordmanagement::call_password_change_method(wdx::instance_id_t id, std::string old_password, std::string new_password) const
{
    // change to new password
    wdx::parameter_instance_path method_path(std::string(paramd_path_all_users) + "/"
                                             + std::to_string(static_cast<int>(id)) + "/"
                                             + paramd_subpath_change_password);
    std::map<std::string, std::shared_ptr<wdx::parameter_value>> in_args;
    in_args.emplace("OldPassword", wdx::parameter_value::create(old_password));
    in_args.emplace("NewPassword", wdx::parameter_value::create(new_password));
    return frontend_m.invoke_method_by_path(method_path, in_args);
}

wdx::parameter_response wdx_passwordmanagement::find_parameter_response_by_path(wdx::parameter_response_list_response const &response_list, std::string const &path) const
{
    for(wdx::parameter_response const &response: response_list.param_responses)
    {
        if(response.path.parameter_path == path)
        {
            return response;
        }
    }
    wc_log(log_level_t::error, "Path \"" + path + "\" isn't present in parameter service response. Password change not possible.");
    throw password_change_error("Can't change password at the moment. Please retry. If this is a permanent error contact your administrator.");
}

wdx::instance_id_t wdx_passwordmanagement::find_instance_id_by_name_value(wdx::parameter_response_list_response const &response_list, std::string const &username) const
{
    for(wdx::parameter_response const &response: response_list.param_responses)
    {
        std::string name = response.path.parameter_path;
        size_t delimiter_index = 0;
        while((delimiter_index = name.find("/")) != std::string::npos) // parasoft-suppress CERT_C-EXP45-d-2 "Compliant solution - Intentional assignment. Parasoft detects a false-positive here."
        {
            name = name.substr(delimiter_index+1, name.size());
        }
        // Search for user ID in name params
        if (name == "Name" && response.value.get()->get_string() == username)
        {
            return response.id.instance_id;
        }
    }
    wc_log(log_level_t::error, "Authenticated user \"" + username + "\" isn't provided in parameter service. Password change not possible.");
    throw password_change_error("Can't change password at the moment. Please retry. If this is a permanent error contact your administrator.");
}

} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
