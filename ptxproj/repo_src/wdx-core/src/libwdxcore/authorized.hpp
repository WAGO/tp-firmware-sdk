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

#ifndef SRC_LIBWDXCORE_AUTHORIZED_HPP_
#define SRC_LIBWDXCORE_AUTHORIZED_HPP_

#include "parameter_service_i.hpp"
#include "wago/wdx/unauthorized.hpp"

namespace wago {
namespace wdx {


class authorized : public parameter_service_frontend_extended_i, public parameter_service_file_api_i
{
private:
    user_data                            user_m;
    user_permissions                     permissions_m;
    std::shared_ptr<parameter_service_i> service_m;

public:
    authorized(user_data                            user, 
               std::shared_ptr<parameter_service_i> service) noexcept
    : user_m(user)
    , permissions_m(service->get_full_user_permissions(user.name))
    , service_m(service)
    { }
    ~authorized() noexcept override = default;

    // frontend
    wago::future<device_collection_response> get_all_devices() override;
    wago::future<parameter_response_list_response> get_all_parameters(parameter_filter filter, size_t paging_offset, size_t paging_limit) override;
    wago::future<std::vector<parameter_response>> get_parameters(std::vector<parameter_instance_id> ids) override;
    wago::future<std::vector<parameter_response>> get_parameters_by_path(std::vector<parameter_instance_path> paths) override;
    wago::future<method_invocation_named_response> invoke_method(parameter_instance_id method_id, std::map<std::string, std::shared_ptr<parameter_value>> in_args) override;
    wago::future<method_invocation_named_response> invoke_method_by_path(parameter_instance_path method_path, std::map<std::string, std::shared_ptr<parameter_value>> in_args) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values_by_path(std::vector<value_path_request> value_path_requests) override;

    // frontend extended
    wago::future<device_response> get_device(device_id device) override;
    wago::future<device_collection_response> get_subdevices(device_collection_id_t device_collection_id) override;
    wago::future<device_collection_response> get_subdevices_by_collection_name(std::string device_collection_name) override;
    wago::future<std::vector<set_parameter_response>> set_parameter_values_by_path_connection_aware(std::vector<value_path_request> value_path_requests, bool defer_wda_web_connection_changes) override;
    wago::future<std::vector<parameter_response>> get_parameter_definitions(std::vector<parameter_instance_id> ids) override;
    wago::future<std::vector<parameter_response>> get_parameter_definitions_by_path(std::vector<parameter_instance_path> paths) override;
    wago::future<parameter_response_list_response> get_all_parameter_definitions(parameter_filter filter, size_t paging_offset, size_t paging_limit) override;
    wago::future<parameter_response_list_response> get_all_method_definitions(parameter_filter filter, size_t paging_offset, size_t paging_limit) override;
    wago::future<std::vector<feature_list_response>> get_features(std::vector<device_path_t> device_paths) override;
    wago::future<std::vector<feature_list_response>> get_features_of_all_devices() override;
    wago::future<feature_response> get_feature_definition(device_path_t device, name_t feature_name) override;
    wago::future<monitoring_list_response> create_monitoring_list(std::vector<parameter_instance_id> ids, uint16_t timeout_seconds) override;
    wago::future<monitoring_list_response> create_monitoring_list_with_paths(std::vector<parameter_instance_path> paths, uint16_t timeout_seconds) override;
    wago::future<monitoring_list_values_response> get_values_for_monitoring_list(monitoring_list_id_t id) override;
    wago::future<monitoring_list_response> get_monitoring_list(monitoring_list_id_t id) override;
    wago::future<monitoring_lists_response> get_all_monitoring_lists() override;
    wago::future<enum_definition_response> get_enum_definition(name_t enum_name) override;
    wago::future<std::vector<enum_definition_response>> get_all_enum_definitions() override;
    wago::future<delete_monitoring_list_response> delete_monitoring_list(monitoring_list_id_t id) override;

    // file api
    wago::future<file_read_response> file_read(file_id id, uint64_t offset, size_t length) override;
    wago::future<response> file_write(file_id id, uint64_t offset, std::vector<uint8_t> data) override;
    wago::future<file_info_response> file_get_info(file_id id) override;
    wago::future<response> file_create(file_id id, uint64_t capacity) override;
    wago::future<file_id_response> create_parameter_upload_id(wago::wdx::parameter_instance_path context, uint16_t timeout_seconds) override;
};

}
}

#endif // SRC_LIBWDXCORE_AUTHORIZED_HPP_
