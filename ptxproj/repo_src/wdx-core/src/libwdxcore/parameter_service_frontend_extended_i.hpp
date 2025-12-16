//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------

#ifndef SRC_LIBWDXCORE_PARAMETER_SERVICE_FRONTEND_EXTENDED_I_HPP_
#define SRC_LIBWDXCORE_PARAMETER_SERVICE_FRONTEND_EXTENDED_I_HPP_

#include <vector>
#include <map>
#include "wago/wdx/parameter_service_frontend_i.hpp"
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/parameter_instance_path.hpp"
#include "wago/wdx/responses.hpp"
#include "wago/wdx/requests.hpp"
#include "wago/future.hpp"

namespace wago {
namespace wdx {

/**
This interface contains extended methods used by the REST-API implementation. It is not meant for widespread use.
Methods might change and/or be removed in the future.
 */

class parameter_service_frontend_extended_i : public parameter_service_frontend_i
{
public:

    /**
    Returns device information.
    */
    virtual wago::future<device_response> get_device(device_id device) = 0;

    /**
    Returns all present devices of the specified deviceCollection.
    */
    virtual wago::future<device_collection_response> get_subdevices(device_collection_id_t device_collection_id) = 0;

    /**
    Returns all present devices of the specified deviceCollection.
    The deviceCollectionName is case-insensitive.
    */
    virtual wago::future<device_collection_response> get_subdevices_by_collection_name(std::string device_collection_name) = 0;

    /**
    Same as `set_parameter_values_by_path` from the `parameter_service_frontend_i`, but with `defer_wda_web_connection_changes` which can be used to prevent premature connection failure.
     */
    virtual wago::future<std::vector<set_parameter_response>> set_parameter_values_by_path_connection_aware(std::vector<value_path_request> value_path_requests, bool defer_wda_web_connection_changes) = 0;

    /**
    Returns the definition for every requested parameter instance, addressed by its numerical ID. The value will be null. No parameter providers will be involved.
    */
    virtual wago::future<std::vector<parameter_response>> get_parameter_definitions(std::vector<parameter_instance_id> ids) = 0;

    /**
    Returns the definition for every requested parameter instance, addressed by its path. The value will be null. No parameter providers will be involved.
    Paths are case-insensitive.
    */
    virtual wago::future<std::vector<parameter_response>> get_parameter_definitions_by_path(std::vector<parameter_instance_path> paths) = 0;

    virtual wago::future<parameter_response_list_response> get_all_parameter_definitions(parameter_filter filter, size_t paging_offset = 0, size_t paging_limit = SIZE_MAX) = 0;

    virtual wago::future<parameter_response_list_response> get_all_method_definitions(parameter_filter filter, size_t paging_offset = 0, size_t paging_limit = SIZE_MAX) = 0;

    /**
    For every requested device, returns the list of features each device has.
    */
    virtual wago::future<std::vector<feature_list_response>> get_features(std::vector<device_path_t> device_paths) = 0;

    /**
    Returns the list of features for all registered devices.
    */
    virtual wago::future<std::vector<feature_list_response>> get_features_of_all_devices() = 0;

    /**
    Returns feature information for requested `device` and `feature_name`.
    */
    virtual wago::future<feature_response> get_feature_definition(device_path_t device, name_t feature_name) = 0;

    /**
    Creates a monitor list that consists of given parameter instances, addressed by their numerical ID.
    Requested parameter instances that are unknown or not provided will not be attempted again later, but stay this way throughout the lifetime of this monitor list.
    */
    virtual wago::future<monitoring_list_response> create_monitoring_list(std::vector<parameter_instance_id> ids, uint16_t timeout_seconds) = 0;

    /**
    Creates a monitor list that consists of given parameter instances, addressed by their paths.
    Requested parameter instances that are unknown or not provided will not be attempted again later, but stay this way throughout the lifetime of this monitor list.
    */
    virtual wago::future<monitoring_list_response> create_monitoring_list_with_paths(std::vector<parameter_instance_path> paths, uint16_t timeout_seconds) = 0;
    
    /**
    Retrieves the parameter values for the monitor list, and refreshes its timeout.
     */
    virtual wago::future<monitoring_list_values_response> get_values_for_monitoring_list(monitoring_list_id_t id) = 0;

    /**
    Gets the monitor list for given ID, and refreshes its timeout.
    */
    virtual wago::future<monitoring_list_response> get_monitoring_list(monitoring_list_id_t id) = 0;

    /**
    Gets all active monitor lists.
    NOTE: Does not refresh the timeouts of the monitor lists.
    */
    virtual wago::future<monitoring_lists_response> get_all_monitoring_lists() = 0;

    /**
    Gets the enum definition for requested `enum_name`.
     */
    virtual wago::future<enum_definition_response> get_enum_definition(name_t enum_name) = 0;
    /**
    Gets all known enum definitions.
     */
    virtual wago::future<std::vector<enum_definition_response>> get_all_enum_definitions() = 0;

    /**
    Deletes the monitor list for given ID.
    */
    virtual wago::future<delete_monitoring_list_response> delete_monitoring_list(monitoring_list_id_t id) = 0;

    virtual ~parameter_service_frontend_extended_i() noexcept = default;
};

}
}
#endif // SRC_LIBWDXCORE_PARAMETER_SERVICE_FRONTEND_EXTENDED_I_HPP_
