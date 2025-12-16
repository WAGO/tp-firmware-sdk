//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXCORE_PARAMETER_SERVICE_CORE_HPP_
#define SRC_LIBWDXCORE_PARAMETER_SERVICE_CORE_HPP_

#include <unordered_map>
#include <memory>
#include <queue>
#include <iostream>
#include <mutex>
#include "parameter_service_i.hpp"

#include "wdmm/device_model.hpp"
#include "wago/wdx/wdmm/method_argument_definition.hpp"
#include "wago/wdx/permissions_i.hpp"
#include "utils/mapped_list.hpp"
#include "instances/device.hpp"
#include "monitoring/monitoring_list_collection.hpp"
#include "provider_collection.hpp"


namespace wago {
namespace wdx {

class parameter_service_core : public parameter_service_i
{
public:

    parameter_service_core(std::unique_ptr<permissions_i> permissions);

    void trigger_lapse_checks() override;
    wago::future<void> cleanup() override;
    void dump_known_model() override;
    void dump_known_providers() override;
    std::vector<parameter_provider_i*> get_parameter_providers(std::vector<parameter_instance_path> paths) override;
    std::mutex &get_internal_param_mutex_that_should_not_be_used_outside() override {
        return m_param_mutex;
    }

    user_permissions get_full_user_permissions(std::string const &user_name) noexcept override;
    std::string get_permission_name(std::string          const &feature,
                                    permissions_i::types const  type) const noexcept override; 

    // *********************************************
    // parameter_service_frontend_i Methods
    // *********************************************
    // **** Value and Method operations
    future<std::vector<parameter_response>> get_parameters(std::vector<parameter_instance_id> ids) override;
    future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;
    future<std::vector<parameter_response>> get_parameters_by_path(std::vector<parameter_instance_path> paths) override;
    future<std::vector<set_parameter_response>> set_parameter_values_by_path(std::vector<value_path_request> value_path_requests) override;
    future<std::vector<set_parameter_response>> set_parameter_values_by_path_connection_aware(std::vector<value_path_request> value_path_requests, bool defer_wda_web_connection_changes) override;

    future<device_response> get_device(device_id device) override;
    future<device_collection_response> get_all_devices() override;
    future<device_collection_response> get_subdevices(device_collection_id_t device_collection_id) override;
    future<device_collection_response> get_subdevices_by_collection_name(std::string device_collection_name) override;

    future<parameter_response_list_response> get_all_parameters(parameter_filter filter, size_t paging_offset, size_t paging_limit) override;
    future<parameter_response_list_response> get_all_parameter_definitions(parameter_filter filter, size_t paging_offset = 0, size_t paging_limit = SIZE_MAX) override;
    future<parameter_response_list_response> get_all_method_definitions(parameter_filter filter, size_t paging_offset = 0, size_t paging_limit = SIZE_MAX) override;

    future<method_invocation_named_response> invoke_method(parameter_instance_id method_id,
                                                   std::map<std::string, std::shared_ptr<parameter_value>> in_args) override;
    future<method_invocation_named_response> invoke_method_by_path(const parameter_instance_path method_path,
                                                           std::map<std::string, std::shared_ptr<parameter_value>> in_args) override;

    future<monitoring_list_response> create_monitoring_list(std::vector<parameter_instance_id> ids, uint16_t timeout_seconds) override;
    future<monitoring_list_response> create_monitoring_list_with_paths(std::vector<parameter_instance_path> paths, uint16_t timeout_seconds) override;
    future<monitoring_list_values_response> get_values_for_monitoring_list(monitoring_list_id_t id) override;
    future<monitoring_list_response> get_monitoring_list(monitoring_list_id_t id) override;
    future<delete_monitoring_list_response> delete_monitoring_list(monitoring_list_id_t id) override;
    future<monitoring_lists_response> get_all_monitoring_lists() override;

    // *** Model operations
    future<std::vector<parameter_response>> get_parameter_definitions(std::vector<parameter_instance_id> ids) override;
    future<std::vector<parameter_response>> get_parameter_definitions_by_path(std::vector<parameter_instance_path> paths) override;
    future<std::vector<feature_list_response>> get_features(std::vector<device_path_t> device_paths) override;
    future<std::vector<feature_list_response>> get_features_of_all_devices() override;
    future<feature_response> get_feature_definition(device_path_t device, name_t feature_name) override;
    future<enum_definition_response> get_enum_definition(name_t enum_name) override;
    future<std::vector<enum_definition_response>> get_all_enum_definitions() override;

    // *********************************************
    // IParameterServiceBackend Methods
    // *********************************************
    wago::future<std::vector<response>> register_devices(std::vector<register_device_request> requests) override;
    wago::future<std::vector<response>> unregister_devices(std::vector<device_id> device_ids) override;
    wago::future<response> unregister_all_devices(device_collection_id_t device_collection) override;
    wago::future<std::vector<response>> register_parameter_providers(std::vector<parameter_provider_i*> providers, parameter_provider_call_mode mode) override;
    void unregister_parameter_providers(std::vector<parameter_provider_i*> providers) override;
    wago::future<std::vector<response>> register_model_providers(std::vector<model_provider_i*> providers) override;
    void unregister_model_providers(std::vector<model_provider_i*> providers) override;
    wago::future<std::vector<response>> register_device_description_providers(std::vector<device_description_provider_i*> providers) override;
    void unregister_device_description_providers(std::vector<device_description_provider_i*> providers) override;
    wago::future<std::vector<response>> register_device_extension_providers(std::vector<device_extension_provider_i*> providers) override;
    void unregister_device_extension_providers(std::vector<device_extension_provider_i*> providers) override;
    wago::future<std::vector<register_file_provider_response>> register_file_providers(std::vector<register_file_provider_request> requests, provider_call_mode mode) override;
    void unregister_file_providers(std::vector<file_transfer::file_provider_i*> providers) override;
    wago::future<std::vector<register_file_provider_response>> reregister_file_providers(std::vector<reregister_file_provider_request> requests, provider_call_mode mode) override;

    // *********************************************
    // File API Methods
    // *********************************************
    wago::future<file_read_response> file_read(file_id id, uint64_t offset, size_t length) override;
    wago::future<response> file_write(file_id id, uint64_t offset, std::vector<uint8_t> data) override;
    wago::future<file_info_response> file_get_info(file_id id) override;
    wago::future<response> file_create(file_id id, uint64_t capacity) override;
    wago::future<file_id_response> create_parameter_upload_id(parameter_instance_path context, uint16_t timeout_seconds) override;

private:

    template <typename T, typename TResult>
    struct partial_job
    {
        partial_job(partial_job const &) noexcept = default;
        partial_job &operator=(partial_job const &) noexcept = default;
        wago::future<T> future;
        std::function<void(T value, std::shared_ptr<TResult> result)> integrate;
        parameter_service_core* service;
        partial_job(parameter_service_core* service_)
        {
            service = service_;
            service->job_count++;
        }
        ~partial_job() noexcept
        {
            service->job_count--;
        }
    };

    struct pp_data
    {
        std::vector<parameter_selector> selectors;
        std::shared_ptr<parameter_provider_i> wrapper_provider; // needed so wrapper doesn't get deleted
        parameter_provider_i* wrapped_provider;
    };

    struct fp_data
    {
        file_id id;
        parameter_id_t context;
        std::shared_ptr<file_provider_i> wrapper_provider; // needed so wrapper doesn't get deleted
        file_provider_i* wrapped_provider;
    };

    struct upload_id_data
    {
        file_id id;
        parameter_id_t context;
        uint16_t timeout_seconds;
        std::chrono::time_point<std::chrono::system_clock> last_access;
        parameter_provider_i* creator_provider;
    };

    std::mutex m_param_mutex;
    std::mutex m_file_mutex;

    std::unique_ptr<permissions_i> m_permissions;

    // device collection management
    std::vector<std::vector<std::shared_ptr<device>>> m_device_collections;
    void initialize_device_collections();
    const std::vector<std::shared_ptr<device>>* get_device_collection(const device_collection_id_t device_collection_id) const;
    const std::vector<std::shared_ptr<device>>* get_device_collection(const device_collection_id_t device_collection_id, status_codes& result) const;
    const std::vector<std::shared_ptr<device>>* get_device_collection(std::string device_collection_name, status_codes &status) const;
    std::shared_ptr<device> get_device_internal(device_id const id) const;
    std::shared_ptr<device> get_device_internal(device_id const id, status_codes& result) const;
    std::shared_ptr<device> get_device_internal(device_path_t const &device_path) const;
    std::shared_ptr<device> get_device_internal(device_path_t const &device_path, status_codes& status) const;

    // provider

    provider_collection<parameter_provider_i, pp_data> m_providers;
    provider_collection<model_provider_i, std::vector<device_selector>> m_model_providers;
    provider_collection<device_description_provider_i, std::vector<device_selector>> m_device_description_providers;
    provider_collection<device_extension_provider_i, device_extension_response> m_device_extension_providers;
    provider_collection<file_provider_i, fp_data> m_file_providers;
    std::map<file_id, upload_id_data> m_upload_ids;
    std::mutex m_upload_ids_mutex;

    void unprovide(const parameter_provider_i* provider);

    status_codes add_provider(parameter_provider_i* provider, std::shared_ptr<parameter_provider_i> wrapper_pp, parameter_provider_i* wrapped_pp);
    status_codes add_provider(model_provider_i* provider);
    status_codes add_provider(device_description_provider_i* provider);
    status_codes add_provider(device_extension_provider_i* provider);

    bool remove_provider(parameter_provider_i* provider);
    bool remove_provider(model_provider_i* provider);
    bool remove_provider(device_description_provider_i* provider);
    bool remove_provider(device_extension_provider_i* provider);

    template <typename T>
    wago::future<std::vector<response>> register_providers(std::vector<T*> providers);
    template <typename T>
    void unregister_providers(std::vector<T*> providers);

    // DeviceModel
    std::shared_ptr<device_model> m_model;
    void load_model(std::string& wdm_artifact);

    // registering
    bool is_match(std::shared_ptr<device> const &device,
                  device_selector         const &selector);
    void match_selected_parameters(std::shared_ptr<device>         const &device,
                                   parameter_provider_i                  *provider,
                                   std::vector<parameter_selector>       &selectors);
    void load_matched_device_informations(std::shared_ptr<device>        const &device,
                                          device_description_provider_i        *provider,
                                          std::vector<device_selector>         &selectors);
    void load_matched_device_informations(std::shared_ptr<device>   const &device,
                                          device_extension_response       &response);

    // Monitoring
    monitoring_list_collection m_monitoring_lists;

    // retrieving parameter_instances
    parameter_instance* get_parameter_instance(parameter_instance_id   const  id,   response *response) const override;
    parameter_instance* get_parameter_instance(parameter_instance_path const &path, response *response) const override;
    std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_id>& ids,
        std::vector<parameter_response>& responses) const override;
    std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        list_i<parameter_instance_id>& ids,
        std::vector<parameter_response>& responses) const override;
    std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_path>& paths,
        std::vector<parameter_response>& responses) const override;
    std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_id>& paths,
        std::vector<method_invocation_named_response>& responses) const override;
    std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_path>& paths,
        std::vector<method_invocation_named_response>& responses) const override;
    std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(list_i<parameter_instance_path>& paths,
                                                                              std::vector<parameter_response>&
                                                                              responses) const override;

    // preparing requests to parameter_providers
    struct provider_read_portion
    {
        parameter_provider_i* provider;
        std::vector<parameter_instance_id> ids_to_ask;
        std::vector<parameter_definition const *> definitions_for_validation;
        std::vector<std::size_t> result_position;
    };
    
    void prepare_get_parameters(std::vector<provider_read_portion>                      &portions,
                                std::shared_ptr<std::vector<parameter_instance*>> const &instances,
                                std::vector<parameter_response>                         &responses,
                                bool                                                     onlyDefinitions = false);

    // Frontend Implementation helpers
    future<std::vector<parameter_response>> get_parameters_internal(std::vector<provider_read_portion>& provider_portions,
                                std::vector<parameter_response> result);

    future<std::vector<parameter_response>> get_parameters_internal(std::shared_ptr<std::vector<parameter_instance*>> const &instances,
                                                                                    std::vector<parameter_response>          result,
                                                                                    bool                                     onlyDefinitions = false) override;
    template<typename T, typename TResult>
    void integrate_and_subscribe_next(std::shared_ptr<std::vector<std::shared_ptr<partial_job<T, TResult>>>> remaining_jobs,
                                std::shared_ptr<TResult> whole_result,
                                std::shared_ptr<wago::promise<TResult>> promise, size_t idx = 0);
    future<method_invocation_named_response> invoke_method_internal(const parameter_instance* instance,
                                const std::map<std::string, std::shared_ptr<parameter_value>>& in_args,
                                method_invocation_named_response result) override;
    std::vector<std::shared_ptr<parameter_value>> process_method_arguments(method_invocation_named_response& result,
                                                                           const std::map<
                                                                               std::string, std::shared_ptr<
                                                                                   parameter_value>>&
                                                                           in_args,
                                                                           const std::vector<
                                                                               method_argument_definition>&
                                                                           in_arg_definitions) const;
    void process_method_response(method_invocation_named_response& result, const method_invocation_response& response,
                                 const std::vector<method_argument_definition>& out_arg_definitions) const;
    void process_parameter_response(value_response& result) const;
    future<std::vector<set_parameter_response>> set_parameter_values_internal(
        std::shared_ptr<std::vector<parameter_instance *>>const &instances, std::vector<set_parameter_response> result,
        const list_i<std::shared_ptr<parameter_value>>& value_requests,
        bool defer_wda_web_connection_changes) override;
    device_collection_response get_subdevices_internal(const std::vector<std::shared_ptr<device>>* device_collection);
    void set_builtin_value(std::shared_ptr<parameter_value> const &val,
                           parameter_id_t                          param_id,
                           std::shared_ptr<device>          const &device);

    future<parameter_response_list_response> get_all_parameters_internal(parameter_filter                const &filter,
                                                                         size_t                                 paging_offset,
                                                                         size_t                                 paging_limit,
                                                                         bool                                   first_phase,
                                                                         std::vector<parameter_response> const &dyn_instantiation_responses,
                                                                         bool                                   only_definitions);

    std::shared_ptr<monitoring_list_collection::monitoring_list>  get_monitoring_list_internal(monitoring_list_id_t id,
                                                                                               status_codes &status) override;

    // File API helpers
    file_id generate_file_id();
    register_file_provider_response register_file_provider_internal(
        file_transfer::file_provider_i        *provider,
        file_id                         const &file_id_,
        parameter_id_t                         context,
        provider_call_mode                     mode
    );
    bool has_file_id(file_id id) override;
    file_provider_i* get_file_provider(file_id id);
    file_context get_file_context(file_id id) override;
    void expire_lapsed_upload_ids_except(file_id const &id_with_heartbeat);

    // feature information helpers
    std::vector<device_feature_information> build_feature_infos(std::vector<name_t> const &feature_names,
                                                                device                    *device);
};
}
}
#endif // SRC_LIBWDXCORE_PARAMETER_SERVICE_CORE_HPP_
