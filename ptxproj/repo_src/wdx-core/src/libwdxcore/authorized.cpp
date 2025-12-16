//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------

#include "authorized.hpp"
#include "wago/wdx/permissions_i.hpp"
#include "instances/parameter_instance.hpp"
#include "wdmm/device_model.hpp"
#include "utils/mapped_list.hpp"

#include <wc/log.h>


namespace wago {
namespace wdx {

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace {
using shared_instances = std::shared_ptr<std::vector<parameter_instance*>>;

//------------------------------------------------------------------------------
// internal function declaration
//------------------------------------------------------------------------------
bool has_permission(std::string          const &feature_name, 
                    user_permissions     const &permissions, 
                    permissions_i::types const  permission_type);

bool create_parameter_filter_for_permissions(user_permissions const &permissions, 
                                             permissions_i::types permission_type, 
                                             parameter_filter &filter);

template <class ResponseType>
void determine_unauthorized_instances(wago::wdx::parameter_service_i   const &service,
                                      std::vector<parameter_instance*> const &instances,
                                      user_permissions                 const &permissions,
                                      permissions_i::types             const  permission_type,
                                      std::vector<ResponseType>              &result);

template <typename ResponseType>
wago::future<ResponseType> add_change_log_trigger_single(std::string                const  &affected_resource,
                                                         wago::future<ResponseType>       &&response_future,
                                                         std::string                const  &user_name,
                                                         std::string                const  &change);

template <typename ResponseType>
wago::future<ResponseType> add_change_log_trigger_vector(shared_instances           const   instances,
                                                         wago::future<ResponseType>       &&response_future,
                                                         std::string                const  &user_name,
                                                         std::string                const  &change);
}


//------------------------------------------------------------------------------
// frontend implementation
//------------------------------------------------------------------------------
wago::future<device_collection_response> authorized::get_all_devices()
{
    return service_m->get_all_devices();
}

wago::future<parameter_response_list_response> authorized::get_all_parameters(parameter_filter filter, size_t paging_offset, size_t paging_limit)
{
    parameter_filter filter_by_permissions;
    if (!create_parameter_filter_for_permissions(permissions_m, permissions_i::types::readonly, filter_by_permissions)) 
    {
        return resolved_future(parameter_response_list_response(status_codes::success));
    }
    return service_m->get_all_parameters(filter | filter_by_permissions, paging_offset, paging_limit);
}

template<typename T>
wago::future<std::vector<parameter_response>> get_parameters_impl(parameter_service_i       &service,
                                                                  user_data           const &user,
                                                                  user_permissions    const &permissions,
                                                                  std::vector<T>             ids)
{
    std::vector<parameter_response> result(ids.size());

    std::lock_guard<std::mutex> guard(service.get_internal_param_mutex_that_should_not_be_used_outside());
    auto instances = service.get_parameter_instances(std::move(ids), result);
    determine_unauthorized_instances(service, *instances, permissions, permissions_i::types::readonly, result);
    return service.get_parameters_internal(instances, std::move(result));  
}

wago::future<std::vector<parameter_response>> authorized::get_parameters(std::vector<parameter_instance_id> ids)
{
    return get_parameters_impl(*service_m, user_m, permissions_m, std::move(ids));
}

wago::future<std::vector<parameter_response>> authorized::get_parameters_by_path(std::vector<parameter_instance_path> paths)
{
    return get_parameters_impl(*service_m, user_m, permissions_m, std::move(paths));
}

using in_arg_map = std::map<std::string, std::shared_ptr<parameter_value>>;
template<typename T>
wago::future<method_invocation_named_response> invoke_method_impl(parameter_service_i       &service,
                                                                  user_data           const &user,
                                                                  user_permissions    const &permissions,
                                                                  T                   const &id,
                                                                  in_arg_map                 in_args)
{
    std::vector<method_invocation_named_response> result(1);

    std::lock_guard<std::mutex> guard(service.get_internal_param_mutex_that_should_not_be_used_outside());
    auto const instances = service.get_parameter_instances(std::vector<T> {id}, result);
    if(instances->at(0) == nullptr)
    {
        return resolved_future(std::move(result.at(0)));
    }

    determine_unauthorized_instances(service, *instances, permissions, permissions_i::types::readwrite, result);
    auto future = service.invoke_method_internal(instances->at(0), std::move(in_args), result.at(0));
    return add_change_log_trigger_single(instances->at(0)->definition->path, std::move(future), user.name, "invoked method");
}

wago::future<method_invocation_named_response> authorized::invoke_method(parameter_instance_id method_id, in_arg_map in_args)
{
    return invoke_method_impl(*service_m, user_m, permissions_m, method_id, in_args);
}

wago::future<method_invocation_named_response> authorized::invoke_method_by_path(parameter_instance_path method_path, in_arg_map in_args)
{
    return invoke_method_impl(*service_m, user_m, permissions_m, method_path, in_args); 
}

wago::future<std::vector<set_parameter_response>> authorized::set_parameter_values(std::vector<value_request> value_requests)
{
    std::lock_guard<std::mutex> guard(service_m->get_internal_param_mutex_that_should_not_be_used_outside());
    auto param_resps = std::vector<parameter_response>(value_requests.size());
    auto param_ids = mapped_list<value_request, parameter_instance_id>(
        value_requests, [](const value_request& vr) { return vr.param_id; });

    auto const instances = service_m->get_parameter_instances(*param_ids.get_iterator(), param_resps);
    std::vector<set_parameter_response> result;
    result.reserve(param_resps.size());
    for(auto& r : param_resps)
    {
        result.push_back(set_parameter_response(r.status));
    }
    determine_unauthorized_instances(*service_m, *instances, permissions_m, permissions_i::types::readwrite, result);
    
    auto param_values = mapped_list<value_request, std::shared_ptr<parameter_value>>(
        value_requests, [](const value_request& vr) { return vr.value; });

    auto future = service_m->set_parameter_values_internal(instances, std::move(result), *param_values.get_iterator(), false);
    return add_change_log_trigger_vector(instances, std::move(future), user_m.name, "sets parameter value");
}

wago::future<std::vector<set_parameter_response>> authorized::set_parameter_values_by_path(std::vector<value_path_request> value_path_requests)
{
    return set_parameter_values_by_path_connection_aware(std::move(value_path_requests), false);
}


//------------------------------------------------------------------------------
// frontend extended implementation
//------------------------------------------------------------------------------
wago::future<device_response> authorized::get_device(device_id device)
{
    return service_m->get_device(device);
}

wago::future<device_collection_response> authorized::get_subdevices(device_collection_id_t device_collection_id)
{
    return service_m->get_subdevices(device_collection_id);
}

wago::future<device_collection_response> authorized::get_subdevices_by_collection_name(std::string device_collection_name)
{
    return service_m->get_subdevices_by_collection_name(device_collection_name);
}

wago::future<std::vector<set_parameter_response>> authorized::set_parameter_values_by_path_connection_aware(std::vector<value_path_request> value_path_requests, bool defer_wda_web_connection_changes)
{
    std::lock_guard<std::mutex> guard(service_m->get_internal_param_mutex_that_should_not_be_used_outside());
    auto param_resps = std::vector<parameter_response>(value_path_requests.size());
    auto param_paths = mapped_list<value_path_request, parameter_instance_path>(
        value_path_requests, [](const value_path_request& vr) { return vr.param_path; });

    auto const instances = service_m->get_parameter_instances(*param_paths.get_iterator(), param_resps);
    std::vector<set_parameter_response> result;
    result.reserve(param_resps.size());
    for(auto& r : param_resps)
    {
        result.push_back(set_parameter_response(r.status));
    }
    determine_unauthorized_instances(*service_m, *instances, permissions_m, permissions_i::types::readwrite, result);
    
    auto param_values = mapped_list<value_path_request, std::shared_ptr<parameter_value>>(
        value_path_requests, [](const value_path_request& vr) { return vr.value; });


    auto future = service_m->set_parameter_values_internal(instances, std::move(result), *param_values.get_iterator(), defer_wda_web_connection_changes);
    return add_change_log_trigger_vector(instances, std::move(future), user_m.name, "sets parameter value");
}

wago::future<std::vector<parameter_response>> authorized::get_parameter_definitions(std::vector<parameter_instance_id> ids)
{
    std::lock_guard<std::mutex> guard(service_m->get_internal_param_mutex_that_should_not_be_used_outside());
    std::vector<parameter_response> result(ids.size());
    auto instances = service_m->get_parameter_instances(ids, result);
    determine_unauthorized_instances(*service_m, *instances, permissions_m, permissions_i::types::readonly, result);
    return service_m->get_parameters_internal(instances, std::move(result), true);
}

wago::future<std::vector<parameter_response>> authorized::get_parameter_definitions_by_path(std::vector<parameter_instance_path> paths)
{
    std::lock_guard<std::mutex> guard(service_m->get_internal_param_mutex_that_should_not_be_used_outside());
    std::vector<parameter_response> result(paths.size());
    auto instances = service_m->get_parameter_instances(paths, result);
    determine_unauthorized_instances(*service_m, *instances, permissions_m, permissions_i::types::readonly, result);
    return service_m->get_parameters_internal(instances, std::move(result), true);
}

wago::future<parameter_response_list_response> authorized::get_all_parameter_definitions(parameter_filter filter, size_t paging_offset, size_t paging_limit)
{
    parameter_filter filter_by_permissions;
    if (!create_parameter_filter_for_permissions(permissions_m, permissions_i::types::readonly, filter_by_permissions)) 
    {
        return resolved_future(parameter_response_list_response(status_codes::success));
    }
    return service_m->get_all_parameter_definitions(filter | filter_by_permissions, paging_offset, paging_limit); 
}

wago::future<parameter_response_list_response> authorized::get_all_method_definitions(parameter_filter filter, size_t paging_offset, size_t paging_limit)
{
    parameter_filter filter_by_permissions;
    if (!create_parameter_filter_for_permissions(permissions_m, permissions_i::types::readwrite, filter_by_permissions)) 
    {
        return resolved_future(parameter_response_list_response(status_codes::success));
    }
    return service_m->get_all_method_definitions(filter | filter_by_permissions, paging_offset, paging_limit); 
}

wago::future<std::vector<feature_list_response>> authorized::get_features(std::vector<device_path_t> device_paths)
{
    return service_m->get_features(device_paths);
}

wago::future<std::vector<feature_list_response>> authorized::get_features_of_all_devices()
{
    return service_m->get_features_of_all_devices();
}

wago::future<feature_response> authorized::get_feature_definition(device_path_t device, name_t feature_name)
{
    return service_m->get_feature_definition(device, feature_name);
}

wago::future<monitoring_list_response> authorized::create_monitoring_list(std::vector<parameter_instance_id> ids, uint16_t timeout_seconds)
{
    return service_m->create_monitoring_list(ids, timeout_seconds);
}

wago::future<monitoring_list_response> authorized::create_monitoring_list_with_paths(std::vector<parameter_instance_path> paths, uint16_t timeout_seconds)
{
    return service_m->create_monitoring_list_with_paths(paths, timeout_seconds);
}

wago::future<monitoring_list_values_response> authorized::get_values_for_monitoring_list(monitoring_list_id_t id)
{
    std::lock_guard<std::mutex> guard(service_m->get_internal_param_mutex_that_should_not_be_used_outside());
    status_codes status = status_codes::internal_error;
    auto monitoring_list = service_m->get_monitoring_list_internal(id, status);
    if(status == status_codes::success)
    {
        auto p = std::make_shared<promise<monitoring_list_values_response>>();
        auto result = monitoring_list->results;
        for(size_t idx=0, e=result.size(); idx < e; idx++)
        {
            if(monitoring_list->parameter_instances->at(idx) != nullptr)
            {
                result[idx].status = status_codes::no_error_yet;
                result[idx].value = nullptr;
            }
        }

        // FIXME: this call is the only difference to the original implementation
        determine_unauthorized_instances(*service_m, *monitoring_list->parameter_instances, permissions_m, permissions_i::types::readonly, result);

        auto f = service_m->get_parameters_internal(monitoring_list->parameter_instances, result);
        f.set_exception_notifier([p](std::exception_ptr eptr) {
            p->set_exception(std::move(eptr));
        });
        f.set_notifier([p](auto &&r) {
            p->set_value(monitoring_list_values_response(std::move(r)));
        });
        return p->get_future();
    }
    else
    {
        return resolved_future(monitoring_list_values_response(status));
    }
}

wago::future<monitoring_list_response> authorized::get_monitoring_list(monitoring_list_id_t id)
{
    return service_m->get_monitoring_list(id);
}

wago::future<monitoring_lists_response> authorized::get_all_monitoring_lists()
{
    return service_m->get_all_monitoring_lists();
}

wago::future<enum_definition_response> authorized::get_enum_definition(name_t enum_name)
{
    return service_m->get_enum_definition(enum_name);
}

wago::future<std::vector<enum_definition_response>> authorized::get_all_enum_definitions()
{
    return service_m->get_all_enum_definitions();
}

wago::future<delete_monitoring_list_response> authorized::delete_monitoring_list(monitoring_list_id_t id)
{
    return service_m->delete_monitoring_list(id);
}


//------------------------------------------------------------------------------
// file api implementation
//------------------------------------------------------------------------------
wago::future<file_read_response> authorized::file_read(file_id id, uint64_t offset, size_t length)
{
    if(!service_m->has_file_id(id))
    {
        return resolved_future(file_read_response(status_codes::unknown_file_id));
    }

    auto context = service_m->get_file_context(id);
    if (!has_permission(context.feature, permissions_m, permissions_i::types::readonly))
    {
        file_read_response response;
        if (context.feature.empty())
        {
            response.set_error(status_codes::unauthorized, "Unable to determine permissions for file context.");
        }
        else
        {
            std::string permission_name = service_m->get_permission_name(context.feature, permissions_i::types::readonly);
            response.set_error(status_codes::unauthorized, "Permission \"" + permission_name + "\" is missing.");
        }
        return resolved_future(std::move(response));
    }
    return service_m->file_read(id, offset, length);
}

wago::future<response> authorized::file_write(file_id id, uint64_t offset, std::vector<uint8_t> data)
{
    if(!service_m->has_file_id(id))
    {
        return resolved_future(response(status_codes::unknown_file_id));
    }

    auto context = service_m->get_file_context(id);
    if (!has_permission(context.feature, permissions_m, permissions_i::types::readwrite))
    {
        response response;
        if (context.feature.empty())
        {
            response.set_error(status_codes::unauthorized, "Unable to determine permissions for file context.");
        }
        else
        {
            std::string permission_name = service_m->get_permission_name(context.feature, permissions_i::types::readwrite);
            response.set_error(status_codes::unauthorized, "Permission \"" + permission_name + "\" is missing.");
        }
        return resolved_future(std::move(response));
    }
    auto future = service_m->file_write(id, offset, data);
    return add_change_log_trigger_single(context.path, std::move(future), user_m.name, "wrote data to file ID \"" + id + "\"");
}

wago::future<file_info_response> authorized::file_get_info(file_id id)
{
    if(!service_m->has_file_id(id))
    {
        return resolved_future(file_info_response(status_codes::unknown_file_id));
    }

    auto context = service_m->get_file_context(id);
    if (!has_permission(context.feature, permissions_m, permissions_i::types::readonly))
    {
        file_info_response response;
        if (context.feature.empty())
        {
            response.set_error(status_codes::unauthorized, "Unable to determine permissions for file context.");
        }
        else
        {
            std::string permission_name = service_m->get_permission_name(context.feature, permissions_i::types::readonly);
            response.set_error(status_codes::unauthorized, "Permission \"" + permission_name + "\" is missing.");
        }
        return resolved_future(std::move(response));
    }
    return service_m->file_get_info(id);
}

wago::future<response> authorized::file_create(file_id id, uint64_t capacity)
{
    if(!service_m->has_file_id(id))
    {
        return resolved_future(response(status_codes::unknown_file_id));
    }

    auto context = service_m->get_file_context(id);
    if (!has_permission(context.feature, permissions_m, permissions_i::types::readwrite))
    {
        response response;
        if (context.feature.empty())
        {
            response.set_error(status_codes::unauthorized, "Unable to determine permissions for file context.");
        }
        else
        {
            std::string permission_name = service_m->get_permission_name(context.feature, permissions_i::types::readwrite);
            response.set_error(status_codes::unauthorized, "Permission \"" + permission_name + "\" is missing.");
        }
        return resolved_future(std::move(response));
    }
    auto future = service_m->file_create(id, capacity);
    return add_change_log_trigger_single(context.path, std::move(future), user_m.name, "created file ID \"" + id + "\"");
}

wago::future<file_id_response> authorized::create_parameter_upload_id(wago::wdx::parameter_instance_path context, uint16_t timeout_seconds)
{
    std::string feature_name;
    {
        std::lock_guard<std::mutex> guard(service_m->get_internal_param_mutex_that_should_not_be_used_outside());
        file_id_response response;
        auto instance = service_m->get_parameter_instance(context, &response);
        if (instance != nullptr)
        {
            feature_name = instance->definition->feature_def.lock()->name;
        }
    }

    if (!has_permission(feature_name, permissions_m, permissions_i::types::readwrite))
    {
        file_id_response response;
        if (feature_name.empty())
        {
            response.set_error(status_codes::unauthorized, "Unable to determine permissions for file context.");
        }
        else
        {
            std::string permission_name = service_m->get_permission_name(feature_name, permissions_i::types::readwrite);
            response.set_error(status_codes::unauthorized, "Permission \"" + permission_name + "\" is missing.");
        }
        return resolved_future(std::move(response));
    }
    auto future = service_m->create_parameter_upload_id(context, timeout_seconds);
    return add_change_log_trigger_single(context.parameter_path, std::move(future), user_m.name, "created file upload ID");
}


//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
bool has_permission(std::string          const &feature_name,
                    user_permissions     const &permissions,
                    permissions_i::types const  permission_type)
{
#if ADMIN_WITH_ALL_PERMISSIONS
    if (permissions.user_name == "admin")
    {
        return true; // admin user has all permissions, no need to check
    }
#endif
    if (permissions.user_name == "root")
    {
        return true; // root user has all permissions, no need to check
    }

    auto set_of_permissions = permission_type == permissions_i::types::readonly 
                                ? permissions.read_permissions 
                                : permissions.write_permissions;
    return set_of_permissions.end() != std::find_if(set_of_permissions.begin(), 
                                                    set_of_permissions.end(), 
                                                    [&feature_name](auto const &feature) {
                                                        return device_model::names_equal(feature_name, feature);
                                                    });
}

bool create_parameter_filter_for_permissions(user_permissions const &permissions, permissions_i::types permission_type, parameter_filter &filter)
{
    // root (and possibly admin) are allowed to access all methods without permission checks
    auto skip_permission_check = (permissions.user_name == "root");
#if ADMIN_WITH_ALL_PERMISSIONS
    skip_permission_check      = skip_permission_check || (permissions.user_name == "admin");
#endif
    if (skip_permission_check)
    {
        WC_DEBUG_LOG("User is " + permissions.user_name + ", no permission check needed.");
        return true;
    }
    auto owned_permissions = (permission_type == permissions_i::types::readonly)
                           ? permissions.read_permissions
                           : permissions.write_permissions;

    std::string permission_type_str = (permission_type == permissions_i::types::readonly)
                                    ? "RO-"
                                    : "RW-";
    if(owned_permissions.empty())
    {
        WC_DEBUG_LOG("No " + permission_type_str + "permissions found for user " + permissions.user_name + ", generate empty response.");
        return false;
    }
    WC_DEBUG_LOG(permission_type_str + "permissions found for user " + permissions.user_name + ", apply filter for response.");

    filter = parameter_filter::only_features(owned_permissions);
    return true;
}

template <class ResponseType>
void determine_unauthorized_instances(wago::wdx::parameter_service_i   const &service,
                                      std::vector<parameter_instance*> const &instances,
                                      user_permissions                 const &permissions,
                                      permissions_i::types             const  permission_type,
                                      std::vector<ResponseType>              &result)
{
#if ADMIN_WITH_ALL_PERMISSIONS
    if (permissions.user_name == "admin")
    {
        return; // admin user has all permissions, no need to check
    }
#endif
    if (permissions.user_name == "root")
    {
        return; // root user has all permissions, no need to check
    }

    std::set<std::string> affected_features;
    for (size_t i = 0; i < result.size(); ++i)
    {
        auto &response = result[i];
        auto &instance = instances[i];

        // skip those that are already determined by any preceeding instance handling
        // but check those only marked because of other unauthorized in set
        // these may also be unauthorized (and not only in the same set)
        if (response.is_determined() && response.status != status_codes::other_unauthorized_request_in_set)
        {
            continue;
        }

        // methods always require readwrite permissions!
        auto instance_permission_type = instance->definition->value_type == parameter_value_types::method
                                        ? permissions_i::types::readwrite
                                        : permission_type;
        auto instance_feature = instance->definition->feature_def.lock();
        if (instance_feature == nullptr)
        {
            auto instance_class = instance->definition->class_def.lock();
            if (instance_class != nullptr)
            {
                instance_feature = instance_class->feature_def.lock();
            }
        }
        if (instance_feature == nullptr)
        {
            // override original response, as this might contain
            // data which must not be passed to the client
            response = ResponseType(status_codes::internal_error,  "Undeterminable permission.");
            continue;
        }

        // no permission
        if (!has_permission(instance_feature->name, permissions, instance_permission_type))
        {
            std::string permission_name = service.get_permission_name(instance_feature->name, instance_permission_type);
            response = ResponseType(status_codes::unauthorized,  "Permission \"" + permission_name + "\" is missing.");

            // look again at all other instances to mark those with the same provider
            // for write operations also as unauthorized (should not occur for methods)
            if (permission_type == permissions_i::types::readwrite)
            {
                for (size_t j = 0; j < result.size(); ++j) {
                    auto &other_response = result[j];
                    auto &other_instance = instances[j];
                    if ((other_response.status != status_codes::unauthorized) &&
                        (other_instance->provider == instance->provider))
                    {
                        other_response = ResponseType(status_codes::other_unauthorized_request_in_set, "Permission \"" + permission_name + "\" is missing.");
                    }
                }
            }
        }
    }
}

template <typename ResponseType>
wago::future<ResponseType> add_change_log_trigger_single(std::string                const  &affected_resource,
                                                         wago::future<ResponseType>       &&response_future,
                                                         std::string                const  &user_name,
                                                         std::string                const  &change)
{
    WC_ASSERT(!user_name.empty());
    WC_ASSERT(!change.empty());
    std::string change_text = user_name + " " + change + ": " + affected_resource;

    auto response_future_ptr = std::make_shared<future<ResponseType>>(std::move(response_future));
    auto chained_promise     = std::make_shared<promise<ResponseType>>(wago::promise<ResponseType>::create_with_dismiss_notifier([future = response_future_ptr](){
        future->dismiss();
    }));
    response_future_ptr->set_exception_notifier(std::bind(&wago::promise<ResponseType>::set_exception, chained_promise, std::placeholders::_1));

    response_future_ptr->set_notifier([promise     = chained_promise,
                                       change_text = std::move(change_text)](ResponseType response){
        // if the response is not successful, we do not log it
        if(is_success(response.status, status_codes_context::parameter_write))
        {
            wc_log(log_level_t::notice, change_text);
        }
        
        promise->set_value(std::move(response));
    });
    
    return chained_promise->get_future();
}

template <typename ResponseType>
wago::future<ResponseType> add_change_log_trigger_vector(shared_instances           const   instances,
                                                         wago::future<ResponseType>       &&response_future,
                                                         std::string                const  &user_name,
                                                         std::string                const  &change)
{
    WC_ASSERT(!user_name.empty());
    WC_ASSERT(!change.empty());
    std::string change_prefix = user_name + " " + change + ": ";

    auto response_future_ptr = std::make_shared<future<ResponseType>>(std::move(response_future));
    auto chained_promise     = std::make_shared<promise<ResponseType>>(wago::promise<ResponseType>::create_with_dismiss_notifier([future = response_future_ptr](){
        future->dismiss();
    }));
    response_future_ptr->set_exception_notifier(std::bind(&wago::promise<ResponseType>::set_exception, chained_promise, std::placeholders::_1));
    response_future_ptr->set_notifier([instances, promise = chained_promise, change_prefix = std::move(change_prefix)](ResponseType response){
        WC_ASSERT(response.size() == instances->size());
        for(size_t i = 0; i < response.size(); ++i)
        {
            // if the response is not successful, we do not log it
            if(is_success(response.at(i).status, status_codes_context::parameter_write))
            {
                WC_ASSERT(instances->at(i)             != nullptr);
                WC_ASSERT(instances->at(i)->definition != nullptr);
                auto const class_def = instances->at(i)->definition->class_def.lock();
                std::string const affected_resource = (class_def != nullptr) 
                                                    ? (class_def->base_path + "/*/" + instances->at(i)->definition->path) 
                                                    : instances->at(i)->definition->path;
                wc_log(log_level_t::notice, change_prefix + affected_resource);
            }
        }
        
        promise->set_value(std::move(response));
    });
    
    return chained_promise->get_future();
}

}

}
}
