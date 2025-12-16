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

#ifndef SRC_LIBWDXCORE_PARAMETER_SERVICE_I_HPP_
#define SRC_LIBWDXCORE_PARAMETER_SERVICE_I_HPP_

#include "parameter_service_frontend_extended_i.hpp"
#include "instances/parameter_instance.hpp"
#include "utils/mapped_list.hpp"
#include "monitoring/monitoring_list_collection.hpp"
#include "wago/wdx/parameter_service_backend_extended_i.hpp"
#include "wago/wdx/file_transfer/parameter_service_file_api_i.hpp"
#include "wago/wdx/permissions_i.hpp"

namespace wago {
namespace wdx {

/**
 * A context for file operations, containing the feature name and the path of 
 * the context parameter.
 */
struct file_context
{
    std::string feature;
    std::string path;
};

/**
 * The complete interface of a parameter service, consisting of `parameter_service_frontend_i` and `parameter_service_backend_i`.
 */
class parameter_service_i : public parameter_service_frontend_extended_i, public parameter_service_backend_extended_i, public parameter_service_file_api_i
{
    public:
    
   
    /**
     * Triggers the cleanup routines for resources with timeout.
     */
    virtual void trigger_lapse_checks() = 0;
    
    /**
     * Triggers the cleanup routines for resources regardless of timeout.
     * Can result in calls to providers.
     */
    virtual wago::future<void> cleanup() = 0;

    // FIXME: mutex should not be available outside of the service
    virtual std::mutex &get_internal_param_mutex_that_should_not_be_used_outside() = 0;
    
    /**
     * Internal debugging utility. No guarantees.
     */
    virtual void dump_known_model() = 0;
    /**
     * Internal debugging utility. No guarantees.
     */
    virtual void dump_known_providers() = 0;
    /**
     * Internal debugging utility. No guarantees.
     */
    virtual std::vector<parameter_provider_i*> get_parameter_providers(std::vector<parameter_instance_path> paths) = 0;

    /**
     * Internal debugging utility. No guarantees. No thread-safety.
     */
    int job_count = 0;

    // retrieving parameter_instances
    virtual parameter_instance* get_parameter_instance(parameter_instance_id   const  id,   response *response) const = 0;
    virtual parameter_instance* get_parameter_instance(parameter_instance_path const &path, response *response) const = 0;
    virtual std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_id>& ids,
        std::vector<parameter_response>& responses) const = 0;
    virtual std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        list_i<parameter_instance_id>& ids,
        std::vector<parameter_response>& responses) const = 0;
    virtual std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        list_i<parameter_instance_path>& paths,
        std::vector<parameter_response>&
        responses) const = 0;
    virtual std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_path>& paths,
        std::vector<parameter_response>& responses) const = 0;
    virtual std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_id>& paths,
        std::vector<method_invocation_named_response>& responses) const = 0;
    virtual std::shared_ptr<std::vector<parameter_instance*>> get_parameter_instances(
        const std::vector<parameter_instance_path>& paths,
        std::vector<method_invocation_named_response>& responses) const = 0;

    // permissions
    virtual user_permissions get_full_user_permissions(std::string const &user_name) noexcept = 0;
    virtual std::string get_permission_name(std::string          const &feature,
                                            permissions_i::types const  type) const noexcept = 0;
    // helpers
    virtual future<std::vector<parameter_response>> get_parameters_internal(std::shared_ptr<std::vector<parameter_instance*>> const &instances,
                                                                            std::vector<parameter_response>                          result,
                                                                            bool                                                     onlyDefinitions = false) = 0;

    virtual future<method_invocation_named_response> invoke_method_internal(parameter_instance                                      const *instance,
                                                                            std::map<std::string, std::shared_ptr<parameter_value>> const &in_args,
                                                                            method_invocation_named_response                               result) = 0;
    virtual future<std::vector<set_parameter_response>> set_parameter_values_internal(std::shared_ptr<std::vector<parameter_instance *>> const &instances, 
                                                                                      std::vector<set_parameter_response>                       result,
                                                                                      list_i<std::shared_ptr<parameter_value>>           const &value_requests,
                                                                                      bool                                                      defer_wda_web_connection_changes) = 0;
    virtual std::shared_ptr<monitoring_list_collection::monitoring_list> get_monitoring_list_internal(monitoring_list_id_t  id,
                                                                                                      status_codes         &status) = 0;
    virtual bool has_file_id(file_id id) = 0;
    virtual file_context get_file_context(file_id id) = 0;
};

}
}

#endif // SRC_LIBWDXCORE_PARAMETER_SERVICE_I_HPP_
