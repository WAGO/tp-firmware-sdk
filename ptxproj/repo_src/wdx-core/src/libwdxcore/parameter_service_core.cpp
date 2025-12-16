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
#include "parameter_service_core.hpp"
#include "loader/device_description_loader.hpp"
#include "loader/device_model_loader.hpp"
#include "serial_parameter_provider.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include "model_provider_i.hpp"
#include "device_description_provider_i.hpp"
#include "device_extension_provider_i.hpp"
#include "wago/wdx/file_transfer/file_provider_i.hpp"
#include "wda_ipc/representation.hpp"
#include "wda_ipc/ipc.hpp"
#include "file_transfer/serial_file_provider.hpp"
#include "utils/string_util.hpp"
#include "utils/algo_helpers.hpp"
#include "instances/check_parameter_value.hpp"
#include "wdmm/model_resolver.hpp"
#include "utils/future-helpers.hpp"

#include <wc/log.h>
#include <wc/assertion.h>

#include <memory>
#include <regex>
#include <set>
#include <ctime>
#include <unordered_set>
#include <functional>

using namespace std;
using wago::wdx::method_argument_definition;

namespace wago {
namespace wdx {

using reset_map = map<parameter_instance*,std::shared_ptr<class_definition>>;

namespace {
constexpr char const g_unknown_marker[] = "<unknown>";

bool collect_and_check_parameters(shared_ptr<vector<parameter_instance *>> const &instances,
                                  list_i<shared_ptr<parameter_value>>      const &values,
                                  device_model                             const &model,
                                  reset_map                                      &instance_resets,
                                  vector<parameter_instance*>                    &instance_keys,
                                  vector<value_request>                          &ids_to_write,
                                  vector<size_t>                                 &result_position,
                                  parameter_provider_i                           *provider,
                                  set<parameter_provider_i*>                     &providers_with_invalid_set,
                                  vector<set_parameter_response>                 &result);

bool check_for_same_base_class(device_model                   const &model, 
                               reset_map                      const &instance_resets,
                               vector<value_request>          const &ids_to_write,
                               vector<size_t>                 const &result_position,
                               parameter_provider_i                 *provider,
                               set<parameter_provider_i*>           &providers_with_invalid_set,
                               vector<set_parameter_response>       &result);

bool check_values_for_all_new_instances(device_model                   const &model, 
                                        reset_map                      const &instance_resets,
                                        vector<value_request>          const &ids_to_write,
                                        vector<size_t>                 const &result_position,
                                        parameter_provider_i                 *provider,
                                        set<parameter_provider_i*>           &providers_with_invalid_set,
                                        vector<set_parameter_response>       &result);

bool check_for_instances_extra_values(device_model                   const &model, 
                                      reset_map                      const &instance_resets,
                                      vector<value_request>          const &ids_to_write,
                                      vector<size_t>                 const &result_position,
                                      parameter_provider_i                 *provider,
                                      set<parameter_provider_i*>           &providers_with_invalid_set,
                                      vector<set_parameter_response>       &result);


 std::shared_ptr<class_definition> get_class_def_for_instantiation(parameter_definition                           const &param_def,
                                                                   std::vector<std::shared_ptr<class_definition>> const &classes);


void append_included_features(std::set<std::string> &features, device_model &model);

}

//*************************************************************************
// ******** Implementation of ParameterService here
//*************************************************************************
parameter_service_core::parameter_service_core(std::unique_ptr<permissions_i> permissions)
: m_permissions(std::move(permissions))
{
    initialize_device_collections();
    m_model = make_shared<device_model>();
}

void parameter_service_core::trigger_lapse_checks() {
    expire_lapsed_upload_ids_except("");
    {
        lock_guard<std::mutex> guard(m_param_mutex);
        m_monitoring_lists.cleanup_monitoring_lists();
    }
}

wago::future<void> parameter_service_core::cleanup() {
    // signalling cleanup to parameter provides that might still have file uploads to clean up
    auto p = make_shared<promise<response>>();
    auto jobs = make_shared<vector<shared_ptr<partial_job<response, response>>>>();

    {
        lock_guard<std::mutex> guard(m_upload_ids_mutex);
        auto it = m_upload_ids.begin();
        while(it != m_upload_ids.end()) {
            auto d = it->second;
            it = m_upload_ids.erase(it);
            auto job = make_shared<partial_job<response, response>>(this);
            job->future = d.creator_provider->remove_parameter_upload_id(d.id, d.context);
            job->integrate = ([this](auto r, auto whole_result) {
                (void)(whole_result);
                if(r.is_success()) {
                    wc_log(log_level_t::info, "ParameterProvider responded to remove_parameter_upload_id with success");
                }
                else {
                    wc_log(log_level_t::warning, "ParameterProvider responded to remove_parameter_upload_id with " + to_string(r.status));
                }
            });
            jobs->push_back(job);
        }
    }
    integrate_and_subscribe_next<response, response>(jobs, make_shared<response>(), p);

    return void_future<response>(p->get_future());
}

void parameter_service_core::initialize_device_collections()
{
    // hard coded device collections
    this->m_device_collections = {
        vector<shared_ptr<device>>(1), vector<shared_ptr<device>>(UINT8_MAX), vector<shared_ptr<device>>(UINT8_MAX) // TODO: make unique_ptr
    }; // reserve place for ROOT, KBUS, RLB...
}

status_codes parameter_service_core::add_provider(parameter_provider_i* provider, std::shared_ptr<parameter_provider_i> wrapper_pp, parameter_provider_i* wrapped_pp) {
    if(!m_providers.add(provider, {})) {
        wc_log(log_level_t::error, "ParameterProvider already registered");
        return status_codes::provider_not_operational;
    }
    try
    {
        // no lock can be present here, otherwise deadlock
        auto response = provider->get_provided_parameters();
        if(response.has_error())
        {
            wc_log(log_level_t::error, "ParameterProvider could not initialize: " + to_string(response.status));
            return status_codes::provider_not_operational; // FIXME: Provider already added to m_providers
        }
        else
        {
            if(!m_providers.update_data(provider, pp_data{response.selected_parameters, std::move(wrapper_pp), wrapped_pp}))
            {
                wc_log(log_level_t::error, "Could not update data for unknown ParameterProvider");
                return status_codes::provider_not_operational;
            }

            std::lock_guard<std::mutex> guard(m_param_mutex);
            for (auto& coll : m_device_collections)
            {
                for(auto& device : coll)
                {
                    match_selected_parameters(device, provider, response.selected_parameters);
                }
            }
            return status_codes::success;
        }
    }
    catch (exception& ex)
    {
        wc_log(log_level_t::error, "ParameterProvider could not initialize: " + std::string(ex.what()));
        return status_codes::provider_not_operational; // FIXME: Provider already added to m_providers
    }
    catch (...)
    {
        wc_log(log_level_t::error, "ParameterProvider could not initialize: Unknown exception");
        return status_codes::provider_not_operational; // FIXME: Provider already added to m_providers
    }
}

void parameter_service_core::unprovide(const parameter_provider_i* provider)
{
    bool no_match = true;
    for (auto& dc : m_device_collections)
    {
        for (auto& device : dc)
        {
            if (device == nullptr)
            {
                continue;
            }
            for (auto const & parameterInstance : device->parameter_instances.get_all())
            {
                if (parameterInstance->provider == provider)
                {
                    wc_log(log_level_t::debug, "Unprovide " + to_description(parameterInstance->id));
                    no_match = false;
                    parameterInstance->provider = nullptr;
                }
            }
        }
    }
    if(no_match)
    {
        wc_log(log_level_t::warning, "Found no parameter instance to unprovide");
    }
}

const vector<shared_ptr<device>>* parameter_service_core::get_device_collection(const device_collection_id_t device_collection_id, status_codes& status) const
{
    if (device_collection_id > device_collections::highest) {
        status = status_codes::invalid_device_collection;
        return nullptr;
    }

    auto const *dc = &m_device_collections.at(device_collection_id);
    // FIXME: dc could never be null
    if(dc == nullptr)
    {
        status = status_codes::unknown_device_collection;
    }
    return dc;
}

const vector<shared_ptr<device>>* parameter_service_core::get_device_collection(const device_collection_id_t device_collection_id) const {
    status_codes dummy = status_codes::no_error_yet;
    return get_device_collection(device_collection_id, dummy);
}

shared_ptr<device> parameter_service_core::get_device_internal(const device_id id, status_codes& status) const
{
    auto const *devices = get_device_collection(id.device_collection_id, status);
    if (devices == nullptr)
    {
        return nullptr;
    }
    if (id.slot >= devices->size())
    {
        status = status_codes::invalid_device_slot;
        return nullptr;
    }

    auto dev = devices->at(id.slot);
    if(!dev)
    {
        status = status_codes::unknown_device;
    }
    return dev;
}

shared_ptr<device> parameter_service_core::get_device_internal(const device_id id) const {
    status_codes dummy = status_codes::no_error_yet;
    return get_device_internal(id, dummy);
}

const vector<shared_ptr<device>>* parameter_service_core::get_device_collection(string device_collection_name, status_codes &status) const
{
    string name = device_collection_name;
    std::transform(device_collection_name.begin(), device_collection_name.end(), device_collection_name.begin(), [](unsigned char c){ return std::tolower(c); });
    if (device_collection_name.empty() || device_collection_name == "root")
    {
        return &m_device_collections[device_collections::root];
    }
    else if(device_collection_name == "kbus")
    {
        return &m_device_collections[device_collections::kbus];
    }
    else if(device_collection_name == "rlb")
    {
        return &m_device_collections[device_collections::rlb];
    }
    else
    {
        status = status_codes::unknown_device_collection;
        return nullptr;
    }
}

shared_ptr<device> parameter_service_core::get_device_internal(const device_path_t &device_path, status_codes& status) const
{
    if (device_path.empty())  // empty means headstation
    {
        auto const *dc = get_device_collection(device_collections::root, status);
        if(dc == nullptr)
        {
            return nullptr;
        }
        auto dev = dc->at(0);
        if(dev == nullptr)
        {
            status = status_codes::unknown_device;
        }
        return dev;
    }

    std::regex re("^([0-9]+)(?:-|/)([0-9]+)$");
    std::cmatch m;
    if (!regex_match(device_path.c_str(), m, re))
    {
        status = status_codes::unknown_device;
        return nullptr;
    }

    string prefix = m.str(1);
    string device_str = m.str(2);

    slot_index_t device_id = 0;
    device_collection_id_t deviceCollectionId = 0;
    auto s = wda_ipc::parse_device_collection_and_slot(prefix, device_str, deviceCollectionId, device_id);
    if(has_error(s))
    {
        status = s;
        return nullptr;
    }
    auto const deviceCollection = get_device_collection(deviceCollectionId, status);

    if (deviceCollection == nullptr)
    {
        return nullptr;
    }
    else if (device_id >= deviceCollection->size())
    {
        status = status_codes::invalid_device_slot;
        return nullptr;
    }
    else
    {
        auto dev = deviceCollection->at(device_id);
        if(dev == nullptr)
        {
            status = status_codes::unknown_device;
        }
        return dev;
    }
}

std::shared_ptr<device> parameter_service_core::get_device_internal(device_path_t const &device_path) const
{
    status_codes dummy = status_codes::no_error_yet;
    return get_device_internal(device_path, dummy);
}

void parameter_service_core::load_model(string& wdm_artifact)
{
    device_model_loader dl;
    dl.load(wdm_artifact, *m_model);
    for (auto& coll : m_device_collections)
    {
        for(auto& device : coll)
        {
            if(device != nullptr)
            {
                device->retry_unfinished_device_informations();
            }
        }
    }
}

parameter_instance* parameter_service_core::get_parameter_instance(const parameter_instance_id id,
                                                              response* response) const
{
    auto device = get_device_internal(id.device, response->status);
    if (!device)
    {
        return nullptr;
    }
    auto *instance = device->parameter_instances.get_instance(id);
    if (instance == nullptr)
    {
        response->status = status_codes::unknown_parameter_id;
        return nullptr;
    }
    return instance;
}

parameter_instance* parameter_service_core::get_parameter_instance(parameter_instance_path const &path,
                                                              response                      *response) const
{
    auto device = get_device_internal(path.device_path, response->status);
    if (device == nullptr)
    {
        return nullptr;
    }
    auto *instance = device->parameter_instances.get_instance(path.parameter_path);
    if (instance == nullptr)
    {
        response->status = status_codes::unknown_parameter_path;
        return nullptr;
    }

    return instance;
}

shared_ptr<vector<parameter_instance*>> parameter_service_core::get_parameter_instances(
    const vector<parameter_instance_id>& ids, vector<parameter_response>& responses) const
{
    WC_ASSERT(responses.size() == ids.size());
    auto instances = make_shared<vector<parameter_instance*>>(ids.size());

    for (size_t idx = 0; idx < ids.size(); ++idx)
    {
        auto const &id = ids[idx];

        auto *instance = get_parameter_instance(id, &responses[idx]);
        if (instance == nullptr)
        {
            responses[idx].id = id; // at least mirror the id
            continue;
        }

        (*instances)[idx] = instance;
    }

    return instances;
}

shared_ptr<vector<parameter_instance*>> parameter_service_core::get_parameter_instances(
    list_i<parameter_instance_id>& ids, vector<parameter_response>& responses) const
{
    WC_ASSERT(responses.size() == ids.size());
    auto instances = make_shared<vector<parameter_instance*>>(ids.size());

    for (size_t idx = 0; idx < ids.size(); ++idx)
    {
        auto *instance = get_parameter_instance(ids[idx], &responses[idx]);
        if (instance == nullptr)
        {
            responses[idx].id = ids[idx]; // at least mirror the id
            continue;
        }

        (*instances)[idx] = instance;
    }

    return instances;
}

shared_ptr<vector<parameter_instance*>> parameter_service_core::get_parameter_instances(
    const vector<parameter_instance_path>& paths, vector<parameter_response>& responses) const
{
    WC_ASSERT(responses.size() == paths.size());
    auto instances = make_shared<vector<parameter_instance*>>(paths.size());

    for (size_t idx = 0; idx < paths.size(); ++idx)
    {
        auto *instance = get_parameter_instance(paths[idx], &responses[idx]);
        if (instance == nullptr)
        {
            responses[idx].path = paths[idx]; // at least mirror the path
            continue;
        }

        (*instances)[idx] = instance;
    }

    return instances;
}

shared_ptr<vector<parameter_instance*>> parameter_service_core::get_parameter_instances(
    const vector<parameter_instance_id>& ids, vector<method_invocation_named_response>& responses) const
{
    WC_ASSERT(responses.size() == ids.size());
    auto instances = make_shared<vector<parameter_instance*>>(ids.size());

    for (size_t idx = 0; idx < ids.size(); ++idx)
    {
        auto *instance = get_parameter_instance(ids[idx], &responses[idx]);
        if (instance == nullptr)
        {
            continue;
        }

        (*instances)[idx] = instance;
    }

    return instances;
}

shared_ptr<vector<parameter_instance*>> parameter_service_core::get_parameter_instances(
    const vector<parameter_instance_path>& paths, vector<method_invocation_named_response>& responses) const
{
    WC_ASSERT(responses.size() == paths.size());
    auto instances = make_shared<vector<parameter_instance*>>(paths.size());

    for (size_t idx = 0; idx < paths.size(); ++idx)
    {
        auto *instance = get_parameter_instance(paths[idx], &responses[idx]);
        if (instance == nullptr)
        {
            continue;
        }

        (*instances)[idx] = instance;
    }

    return instances;
}

shared_ptr<vector<parameter_instance*>> parameter_service_core::get_parameter_instances(
    list_i<parameter_instance_path>& paths, vector<parameter_response>& responses) const
{
    WC_ASSERT(responses.size() == paths.size());
    auto instances = make_shared<vector<parameter_instance*>>(paths.size());

    for (size_t idx = 0; idx < paths.size(); ++idx)
    {
        auto *instance = get_parameter_instance(paths[idx], &responses[idx]);
        if (instance == nullptr)
        {
            responses[idx].path = paths[idx]; // at least mirror the path
            continue;
        }

        (*instances)[idx] = instance;
    }

    return instances;
}

//*************************************************************************
// ******** Implementation of parameter_service_frontend_i. here
// All public methods lock the param-mutex on top level for consistent reads on devices and their parameter-instances.
//*************************************************************************
future<vector<parameter_response>> parameter_service_core::get_parameters(vector<parameter_instance_id> ids)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<parameter_response>(ids.size());

    auto instances = get_parameter_instances(ids, result);

    return get_parameters_internal(instances, std::move(result));
}

future<vector<parameter_response>> parameter_service_core::get_parameters_by_path(vector<parameter_instance_path> paths)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<parameter_response>(paths.size());

    auto instances = get_parameter_instances(paths, result);

    return get_parameters_internal(instances, std::move(result));
}

future<std::vector<parameter_response>> parameter_service_core::get_parameter_definitions(std::vector<parameter_instance_id> ids)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<parameter_response>(ids.size());

    auto instances = get_parameter_instances(ids, result);

    return get_parameters_internal(instances, std::move(result), true);
}

future<std::vector<parameter_response>> parameter_service_core::get_parameter_definitions_by_path(std::vector<parameter_instance_path> paths)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<parameter_response>(paths.size());

    auto instances = get_parameter_instances(paths, result);

    return get_parameters_internal(instances, std::move(result), true);
}

void parameter_service_core::prepare_get_parameters(
        std::vector<provider_read_portion>                      &portions,
        std::shared_ptr<std::vector<parameter_instance*>> const &instances,
        std::vector<parameter_response>                         &responses,
        bool                                                     onlyDefinitions
    )
    {
    set<parameter_provider_i*> providers;
    WC_ASSERT(instances->size() == responses.size());

    WC_DEBUG_LOG("READ: Preparing " + std::to_string(instances->size()) + " requested parameters");

    // determine the providers and save it to the local providers list
    // collect the values and assign it to the response if they are defined in the device description 
    for (size_t idx = 0, e = instances->size(); idx < e; ++idx)
    {
        auto& res = responses[idx];
        auto *instance = (*instances)[idx];

        if (instance == nullptr)
        {
            WC_DEBUG_LOG("Instance null for parameter [" + std::to_string(idx) + "]");
            continue;
        }
        
        // set id and path (if possible)
        res.id = instance->id;
        if (instance->definition != nullptr) {
            res.path = parameter_instance_path(parameter_instance_collection::build_parameter_instance_path(instance).m_path, wda_ipc::to_string(instance->id.device));
        }
        
        if (res.has_error())
        {
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "]: " + to_string(res.status));
            continue;
        }

        if (instance->definition != nullptr) {
            if(instance->definition->value_type != parameter_value_types::method)
            {
                res.definition = make_shared<parameter_definition>(*instance->definition);
            }
            else
            {
                res.definition = make_shared<method_definition>(*(static_cast<method_definition const *>(instance->definition))); // parasoft-suppress CERT_C-EXP39-b-3 "Type is checked as parameter_value_types::method before"
            }
        }

        if(onlyDefinitions)
        {
            res.status = status_codes::success;
            continue;
        }

        if((instance->definition != nullptr) && (instance->definition->value_type == parameter_value_types::method))
        {
            res.status = status_codes::methods_do_not_have_value;
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": " + to_string(res.status));
            continue;
        }

        // TODO: authorization

        // if there is a value defined in the device description (=cacheValue) then return it within the response if the parameter is readonly anyway
        if (instance->fixed_value)
        {
            res.status = status_codes::success;
            res.value = instance->fixed_value;
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": fixed value " + instance->fixed_value->get_json());
            continue;
        }

        // if there is no valid provider defined for this parameter instance then return the appropriate error response
        if (instance->provider == nullptr)
        {
            if(instance->status_unavailable_if_not_provided)
            {
                res.status = status_codes::status_value_unavailable;
            }
            else
            {
                res.status = status_codes::parameter_not_provided;
            }
            res.message = "No parameter provider is associated with this parameter.";
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": " + to_string(res.status));
            continue;
        }

        // if there ist a valid provider then save it in the local providers list
        providers.insert(instance->provider);
        WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": -> " + instance->provider->display_name());
    }

    // TODO: To improve performance may use a mapping (provider -> instance) to avoid second loop over instances
    // now collect the values from the provider and assign it to the response
    for (auto *provider : providers)
    {
        provider_read_portion p;
        p.provider = provider;
        WC_DEBUG_LOG("Collecting parameters for provider " + provider->display_name());
        // collect the parameter instances for the current provider
        for (size_t idx = 0, e = instances->size(); idx < e; ++idx)
        {
            if (responses[idx].is_determined())
            {
                continue;
            }
            auto& instance = instances->at(idx);
            if (instance->provider == provider)
            {
                p.ids_to_ask.push_back(instance->id);
                WC_DEBUG_LOG("Added [" + std::to_string(idx) + "] " + to_short_description(instance->id));
                p.definitions_for_validation.push_back(instance->definition);
                // this is why we need a complete parameter_id with device_id - otherwise the information would not be available for get_parameters_by_path
                p.result_position.push_back(idx);
            }
        }
        portions.push_back(p);
    }
}

future<std::vector<parameter_response>> parameter_service_core::get_parameters_internal(std::vector<provider_read_portion> &provider_portions,
                                                                                   std::vector<parameter_response>     result)
{
    auto p = make_shared<promise<vector<parameter_response>>>();
    auto jobs = make_shared<vector<shared_ptr<partial_job<vector<value_response>, vector<parameter_response>>>>>();
    for (auto portion : provider_portions)
    {
        try
        {
            // TODO: make sure only one call at a time comes through
            // TODO: make sure no call to SetParameterValues/InvokeMethod is pending
            // TODO: handle timeouts
            // now get the values from the provider

            for (size_t idx = 0, e = portion.ids_to_ask.size(); idx < e; ++idx)
            {
                size_t pos = portion.result_position[idx];
                result[pos].status = status_codes::internal_error; // this status will be overwritten by the integration of results, except when an exception occurs.
            }

            auto job = make_shared<partial_job<vector<value_response>, vector<parameter_response>>>(this);
            WC_DEBUG_LOG("Calling ParameterProvider " + portion.provider->display_name() + " with " + std::to_string(portion.ids_to_ask.size()) + " parameters");
            job->future = portion.provider->get_parameter_values(portion.ids_to_ask);
            job->integrate = ([this, portion](auto providerResult, auto whole_result) { // TODO: move portion?
                // copy the values to the result list at the correct position
                for (size_t idx = 0, e = providerResult.size(); idx < e; ++idx)
                {
                    size_t pos = portion.result_position[idx];
                    auto& response = providerResult[idx];
                    auto const& definition = portion.definitions_for_validation.at(idx);
                    auto& single_result = whole_result->at(pos);
                    
                    if (response.is_success())
                    {
                        auto validation = check_parameter_value(response.value, definition);
                        if(has_error(validation))
                        {
                            std::string const parameter_value = response.value ? response.value->to_string() : "null";
                            wc_log(log_level_t::error, "ParameterProvider returned invalid value (" + parameter_value + ") for parameter " + to_short_description(portion.ids_to_ask.at(idx)) + ": " + to_string(validation));
                            single_result.status = status_codes::internal_error;
                        }
                        else
                        {
                            this->process_parameter_response(response);
                            single_result.value = response.value;
                            single_result.status = status_codes::success;
                        }
                    }
                    else if(response.status == status_codes::status_value_unavailable && definition->user_setting)
                    {
                        wc_log(log_level_t::error, "ParameterProvider returned invalid status (" + to_string(response.status) + ") for user setting " + to_short_description(portion.ids_to_ask.at(idx)));
                        single_result.status = status_codes::internal_error;
                    }
                    else if (response.is_determined())
                    {
                        single_result.status = response.status;
                        single_result.domain_specific_status_code = response.domain_specific_status_code;
                        single_result.message = response.message;
                    }
                    else // the parameter_provider didn't do anything with the parameter
                    {
                        single_result.status = status_codes::parameter_not_provided;
                        single_result.message = "Associated parameter provider did not handle this parameter.";
                    }
                    WC_DEBUG_LOG("Response [" + std::to_string(idx) + "] from provider was " + to_string(response.status) + " -> Response [" + std::to_string(pos) + "] from core is " + to_string(single_result.status));
                }
            });
            jobs->push_back(job);
        }
        catch(exception& ex)
        {
            wc_log(log_level_t::error, "Exception during getting parameters: " + std::string(ex.what()));
        }
        catch (...)
        {
            wc_log(log_level_t::error, "Exception during getting parameters");
        }
    }

    integrate_and_subscribe_next<vector<value_response>, vector<parameter_response>>(jobs, make_shared<vector<parameter_response>>(std::move(result)), p);

    return p->get_future();
}

future<vector<parameter_response>> parameter_service_core::get_parameters_internal(shared_ptr<vector<parameter_instance*>> const &instances,
                                                                              vector<parameter_response>                     result,
                                                                              bool                                           onlyDefinitions)
{
    std::vector<provider_read_portion> provider_portions;
    prepare_get_parameters(provider_portions, instances, result, onlyDefinitions);

    return get_parameters_internal(provider_portions, std::move(result));
}

template<typename T, typename TResult>
void parameter_service_core::integrate_and_subscribe_next(
                                std::shared_ptr<std::vector<std::shared_ptr<partial_job<T, TResult>>>> remaining_jobs,
                                std::shared_ptr<TResult> whole_result,
                                std::shared_ptr<wago::promise<TResult>> promise, size_t idx)
{
    if(idx < remaining_jobs->size())
    {
        auto j = remaining_jobs->at(idx); 
        
        // set_notifier also nicht fuer alle futures gleichzeitig, sondern nacheinander und erst nachdem der vorgaenger integriert wurde
        // das funktioniert, weil man einem wago::future auch nach Eintreffen des Wertes einen notifier setzen kann
        // der notifier wird also ggf. direkt ausgeloest - d.h. auch, dasss diese Anweisung die letzte fuer diesen index sein muss
        j->future.set_notifier(([this, idx, whole_result, remaining_jobs, promise](T &&partial_result) mutable {
            remaining_jobs->at(idx)->integrate(std::move(partial_result), whole_result);
            integrate_and_subscribe_next(remaining_jobs, whole_result, promise, idx+1);
        }));
        j->future.set_exception_notifier([this, idx, whole_result, remaining_jobs, promise](std::exception_ptr eptr) mutable {
            try { std::rethrow_exception(std::move(eptr)); }
            catch(exception& ex)
            {
                wc_log(log_level_t::error, "ParameterProvider Exception: " + std::string(ex.what()));
                integrate_and_subscribe_next(remaining_jobs, whole_result, promise, idx+1);
            }
        });
    }
    else
    {
        // Gesamtresultat zurueckgeben
        promise->set_value(std::move(*whole_result));
        
        // release cyclic references (remaining_jobs -> job -> future -> notifier -> remaining_jobs), otherwise jobs won't be cleaned up
        for(auto& j : *remaining_jobs)
        {
            j.reset();
        }
        remaining_jobs.reset();
    }
}

future<device_response> parameter_service_core::get_device(device_id device)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    device_response r;
    auto dev = get_device_internal(device, r.status);
    if(r.has_error())
    {
        return resolved_future(std::move(r));
    }
    if(dev != nullptr)
    {
        r.status = status_codes::success;
        r.id = dev->id;
        r.order_number = dev->get_order_number();
        r.firmware_version = dev->get_firmware_version();
        r.has_beta_parameters = dev->has_beta_parameters;
        r.has_deprecated_parameters = dev->has_deprecated_parameters;
    }
    else
    {
        r.status = status_codes::unknown_device;
    }
    return resolved_future(std::move(r));
}

future<device_collection_response> parameter_service_core::get_all_devices()
{
    lock_guard<std::mutex> guard(m_param_mutex);
    device_collection_response result;

    for(auto const &dc : m_device_collections)
    {
        auto ds = get_subdevices_internal(&dc);
        append(result.devices, ds.devices);
    }
    result.status = status_codes::success;

    return resolved_future(std::move(result));
}

future<device_collection_response> parameter_service_core::get_subdevices(const device_collection_id_t device_collection_id)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto const *deviceCollection = get_device_collection(device_collection_id);
    if (deviceCollection == nullptr)
    {
        return resolved_future(device_collection_response(status_codes::unknown_device_collection));
    }
    return resolved_future(get_subdevices_internal(deviceCollection));
}

future<device_collection_response> parameter_service_core::get_subdevices_by_collection_name(const string device_collection_name)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    status_codes status = status_codes::internal_error;
    auto const *deviceCollection = get_device_collection(device_collection_name, status);
    if (deviceCollection == nullptr)
    {
        return resolved_future(device_collection_response(status));
    }
    return resolved_future(get_subdevices_internal(deviceCollection));
}

device_collection_response parameter_service_core::get_subdevices_internal(
    const vector<shared_ptr<device>>* device_collection)
{
    device_collection_response result;
    for (auto const &device : *device_collection)
    {
        if (device)
        {
            result.devices.push_back(device_response(device->id,
                                                     device->get_order_number(),
                                                     device->get_firmware_version(),
                                                     device->has_beta_parameters,
                                                     device->has_deprecated_parameters));
        }
    }
    result.status = status_codes::success;
    return result;
}

future<vector<feature_list_response>> parameter_service_core::get_features(vector<device_path_t> device_paths)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<feature_list_response>(device_paths.size());

    for (size_t idx = 0, e = device_paths.size(); idx != e; ++idx)
    {
        try
        {
            auto& device_path = device_paths.at(idx);
            auto& res = result.at(idx);

            auto device = get_device_internal(device_path, res.status);
            if (!device)
            {
                continue;
            }
            res.device_path = wago::wda_ipc::to_string(device->id);
            res.features = build_feature_infos(device->collected_features.resolved_includes, device.get());
            res.status = status_codes::success;
        }
        catch (...)
        {
            wc_log(log_level_t::error, "Unexpected exception during getting features");
            result.at(idx).status = status_codes::internal_error;
        }
    }

    return resolved_future(std::move(result));
}

future<vector<feature_list_response>> parameter_service_core::get_features_of_all_devices()
{
    vector<device_path_t> dpaths;
    for(auto const &d : get_all_devices().get().devices)
    {
        dpaths.push_back(wago::wda_ipc::to_string(d.id));
    }
    return get_features(dpaths);
}

future<feature_response> parameter_service_core::get_feature_definition(device_path_t device_path, name_t feature_name)
{
    feature_response res;
    res.device_path = device_path;
    auto device = get_device_internal(device_path, res.status);
    if (device)
    {
        auto         feature_infos   = build_feature_infos({feature_name}, device.get());
        auto const & device_features = device->collected_features.resolved_includes;
        if(feature_infos.empty())
        {
            res.status = status_codes::unknown_feature_name;
        }
        else if (std::find_if(device_features.begin(), device_features.end(), [feature_name] (auto const &device_feature) {
            return to_lower_copy(feature_name) == to_lower_copy(device_feature);
        }) == device_features.end()) {
            res.status = status_codes::feature_not_available;
        }
        else
        {
            res.status = status_codes::success;
            res.feature = feature_infos[0];
        }
    }
    return resolved_future(std::move(res));   
}

vector<device_feature_information> parameter_service_core::build_feature_infos(vector<name_t> const &feature_names,
                                                                          device               *device)
{
    vector<device_feature_information> res;
    for(auto const &fn : feature_names)
    {
        device_feature_information di;
        auto fd = m_model->find_feature_definition(fn);
        if(!fd)
        {
            continue;
        }
        di.name = fd->name;
        di.is_beta = fd->is_beta;
        di.is_deprecated = fd->is_deprecated;
        for(auto const &pd : fd->parameter_definitions)
        {
            auto dds = device->collected_features.resolved_parameter_definitions;
            auto dd = std::find_if(dds.begin(), dds.end(), [pd](auto d) {
                return d->id == pd->id;
            });
            if(dd != dds.end())
            {
                di.parameter_definitions.push_back(*dd);
            }
        }
        di.includes = build_feature_infos(fd->includes, device);
        res.push_back(di);
    }
    return res;
}

future<enum_definition_response> parameter_service_core::get_enum_definition(name_t enum_name)
{
    enum_definition_response r;
    r.definition = m_model->find_enum_definition(enum_name);
    if(r.definition != nullptr)
    {
        r.status = status_codes::success;
    }
    else
    {
        r.status = status_codes::unknown_enum_name;
    }
    return resolved_future(std::move(r));
}

future<vector<enum_definition_response>> parameter_service_core::get_all_enum_definitions()
{
    vector<enum_definition_response> r;
    for(auto const &e : m_model->enums)
    {
        enum_definition_response er;
        er.status = status_codes::success;
        er.definition = e;
        r.push_back(er);
    }
    return resolved_future(std::move(r));
}

future<method_invocation_named_response> parameter_service_core::invoke_method(parameter_instance_id method_id,
                                                                  map<string, shared_ptr<parameter_value>> in_args)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    method_invocation_named_response result;
    
    auto *instance = get_parameter_instance(method_id, &result);

    if (!result.has_error())
    {
        return invoke_method_internal(instance, in_args, std::move(result));
    }
    else
    {
        return resolved_future(std::move(result));
    }
}

future<method_invocation_named_response> parameter_service_core::invoke_method_by_path(
    parameter_instance_path method_path, map<string, shared_ptr<parameter_value>> in_args)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    method_invocation_named_response result;

    auto *instance = get_parameter_instance(method_path, &result);

    if (!result.has_error())
    {
        WC_DEBUG_LOG("INVOKE: Preparing requested method " + method_path.parameter_path);
        return invoke_method_internal(instance, in_args, std::move(result));
    }
    else
    {
        WC_DEBUG_LOG("INVOKE: Requested method " + to_string(result.status));
        return resolved_future(std::move(result));
    }
}

future<method_invocation_named_response> parameter_service_core::invoke_method_internal(const parameter_instance* instance,
                                                                                   const map<string, shared_ptr<parameter_value>>& in_args,
                                                                                   method_invocation_named_response result)
{
    if (result.has_error())
    {
        return resolved_future(std::move(result));
    }

    // TODO: handle exceptions
    if (instance->definition->value_type != parameter_value_types::method)
    {
        result.status = status_codes::not_a_method;
        WC_DEBUG_LOG("Method " + to_short_description(instance->id) + ": " + to_string(result.status));
        return resolved_future(std::move(result));
    }

    if (instance->provider == nullptr)
    {
        result.status = status_codes::parameter_not_provided;
        WC_DEBUG_LOG("Method " + to_short_description(instance->id) + ": " + to_string(result.status));
        return resolved_future(std::move(result));
    }

    auto *method_def = static_cast<method_definition const *>(instance->definition); // parasoft-suppress CERT_C-EXP39-b-3 "Type is checked as parameter_value_types::method before"

    // TODO: check if method is ready (similar to DAM.isExecutable)

    auto positional_args = process_method_arguments(result, in_args, method_def->in_args);
    if (result.has_error())
    {
        result.message = "Method arguments are invalid according to their definition.";
        WC_DEBUG_LOG("Method " + to_short_description(instance->id) + ": " + to_string(result.status));
        return resolved_future(std::move(result));
    }

    auto *provider = instance->provider;

    auto p = make_shared<promise<method_invocation_named_response>>();

    // TODO: make sure only one call at a time comes through
    // TODO: make sure no call to GetParameterValues/SetParameterValues is pending
    try
    {
        WC_DEBUG_LOG("Calling " + provider->display_name() + " with Method " + to_short_description(instance->id));
        auto f = provider->invoke_method(instance->id, std::move(positional_args));
        f.set_exception_notifier([this, p, provider, method_def](std::exception_ptr eptr) {
            try { std::rethrow_exception(std::move(eptr)); }
            catch(const std::exception& ex)
            {
                wc_log(log_level_t::error, "Exception in ParameterProvider invokeMethod '" + method_def->path + "': " + std::string(ex.what()));
            }
            p->set_value(method_invocation_named_response(status_codes::internal_error));
        });
        f.set_notifier([this, p, method_def](auto &&r) {
            method_invocation_named_response res;
            this->process_method_response(res, r, method_def->out_args);
            WC_DEBUG_LOG("Provider responded with " + to_string(r.status) + " -> Core result: " + to_string(res.status));
            p->set_value(std::move(res));
        });
    }
    catch(const std::exception& ex)
    {
        wc_log(log_level_t::error, "Exception in ParameterProvider invokeMethod '" + method_def->path + "': " + std::string(ex.what()));
        p->set_value(method_invocation_named_response(status_codes::internal_error));
    }
    catch(...)
    {
        wc_log(log_level_t::error, "Unknown exception in ParameterProvider invokeMethod '" + method_def->path + "'");
        p->set_value(method_invocation_named_response(status_codes::internal_error));
    }
    return p->get_future();
}

vector<shared_ptr<parameter_value>> parameter_service_core::process_method_arguments(
    method_invocation_named_response& result, const map<string, shared_ptr<parameter_value>>& in_args,
    const vector<method_argument_definition>& in_arg_definitions) const
{
    auto args = vector<shared_ptr<parameter_value>>();

    for (auto const & arg_definition : in_arg_definitions)
    {
        shared_ptr<parameter_value> in_arg;
        if (in_args.find(arg_definition.name) != in_args.end())
        {
            in_arg = in_args.at(arg_definition.name);
        }
        else if(arg_definition.default_value)
        {
            in_arg = arg_definition.default_value;
        }
        else
        {
            result.status = status_codes::missing_argument;
            return args;
        }
            
        result.status = check_parameter_value(in_arg, &arg_definition);
        if (result.has_error())
        {
            std::string const in_arg_value = in_arg ? in_arg->to_string() : "null";
            wc_log(log_level_t::notice, "Rejected invalid value (" + in_arg_value + ") for InArg (" + arg_definition.name + "): " + to_string(result.status));
            return args;
        }
        WC_DEBUG_LOG("InArg '" + arg_definition.name + "' value: " + in_arg->get_json());
        args.push_back(in_arg);
    }

    return args;
}

void parameter_service_core::process_parameter_response(value_response& result) const
{
    if(!result.value)
    {
        return;
    }
    if(result.value->get_type() == parameter_value_types::instantiations)
    {
        // we need to re-create the instantiations list, 
        // as the provider does not know about the model and may not
        // provide a resolved class list for each instance
        std::vector<class_instantiation> val;
        for(class_instantiation_internal inst :  result.value->get_instantiations())
        {
            model_resolver::prepare_class_instantiation(inst, *m_model, false);
            val.push_back(inst);
        }
        result.value = parameter_value::create_instantiations(val);
    }
}

void parameter_service_core::process_method_response(method_invocation_named_response& result,
                                                const method_invocation_response& response,
                                                const vector<method_argument_definition>& out_arg_definitions) const
{
    result.status = response.status;
    result.domain_specific_status_code = response.domain_specific_status_code;
    result.message = response.message;

    if (result.has_error())
    {
        return;
    }
    if(!result.is_determined()) // the parameter_provider didn't do anything with the parameter
    {
        result.status = status_codes::parameter_not_provided;
        result.message = "Associated parameter provider did not handle this method.";
        return;
    }

    if (response.out_args.size() != out_arg_definitions.size())
    {
        wc_log(log_level_t::error, "ParameterProvider returned wrong number (" + std::to_string(response.out_args.size()) + ") of OutArgs (expected " + std::to_string(out_arg_definitions.size()) + ")");
        result.status = status_codes::internal_error;
        return;
    }

    size_t idx = 0;
    for (auto const &argDefinition : out_arg_definitions)
    {
        auto const &out_arg = response.out_args[idx];
        auto validation = check_parameter_value(out_arg, &argDefinition);
        if(has_error(validation))
        {
            std::string const out_arg_value = out_arg ? out_arg->to_string() : "null";
            wc_log(log_level_t::error, "ParameterProvider returned invalid value (" + out_arg_value + ") for OutArg " + argDefinition.name + ": " + to_string(validation));
            result.status = status_codes::internal_error;
        } // this means one invalid out_arg will define the result.status, but still all other out_args will be part of the response
        else
        {
            result.out_args[argDefinition.name] = out_arg;
            WC_DEBUG_LOG("OutArg '" + argDefinition.name + "' value: " + out_arg->get_json());
        }
        idx++;
    }
}

future<vector<set_parameter_response>> parameter_service_core::set_parameter_values_internal(
    shared_ptr<vector<parameter_instance *>> const &instances, vector<set_parameter_response> result,
    const list_i<shared_ptr<parameter_value>>& values, bool defer_wda_web_connection_changes)
{
    set<parameter_provider_i*> providers;
    set<parameter_provider_i*> providers_with_invalid_set;

    WC_DEBUG_LOG("WRITE: Preparing " + std::to_string(instances->size()) + " requested parameters");

    for (size_t idx = 0, e = instances->size(); idx < e; ++idx)
    {
        auto& res = result[idx];
        if (res.has_error())
        {
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "]: " + to_string(res.status));
            continue;
        }

        auto const *instance = (*instances)[idx];

        if (instance->definition->overrideables.inactive)
        {
            res.status = status_codes::ignored;
            res.message = "Parameter is inactive according to the device description.";
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": " + to_string(res.status));
            continue;
        }
        if (!instance->definition->writeable)
        {
            res.status = status_codes::parameter_not_writeable;
            res.message = "Parameter is not writeable according to the parameter definition.";
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": " + to_string(res.status));
            continue;
        }

        if (instance->provider == nullptr)
        {
            res.status = status_codes::parameter_not_provided;
            res.message = "No parameter provider is associated with this parameter.";
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": " + to_string(res.status));
            continue;
        }

        const auto& val = values[idx];
        res.status = check_parameter_value(val, instance->definition);
        if (res.has_error())
        {
            std::string const parameter_value = val ? val->to_string() : "null";
            res.message = "Value is invalid according to the parameter definition.";
            wc_log(log_level_t::notice, "Rejected invalid value (" + parameter_value + ") for parameter (" + to_short_description(instance->id) + "): " + to_string(res.status));
            WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": " + to_string(res.status));
            providers_with_invalid_set.insert(instance->provider);
            continue;
        }
        providers.insert(instance->provider);
        WC_DEBUG_LOG("Parameter [" + std::to_string(idx) + "] " + to_short_description(instance->id) + ": -> " + instance->provider->display_name());
    }

    auto p = make_shared<promise<vector<set_parameter_response>>>();
    auto jobs = make_shared<vector<shared_ptr<partial_job<vector<set_parameter_response>, vector<set_parameter_response>>>>>();

    // TODO: To improve performance may use a mapping (provider -> instance) to avoid second loop over instances
    for (auto *provider : providers)
    {
        vector<value_request> ids_to_write;
        vector<size_t> result_position;
        reset_map instance_resets;
        vector<parameter_instance*> instance_keys;
        if(!contains(providers_with_invalid_set, provider))
        {
            WC_DEBUG_LOG("Collecting parameters for provider " + provider->display_name());
            bool write_set_valid = collect_and_check_parameters(instances, values, *m_model,
                                                                instance_resets, instance_keys, ids_to_write, result_position,
                                                                provider, providers_with_invalid_set, result);
            if(write_set_valid)                                                 
            {
                // Ensure instance keys are only writeable in reset case
                for(auto &key :instance_keys)
                {
                    auto find_result = std::find_if(instance_resets.begin(), instance_resets.end(), [&key](auto const &reset_instance) {
                        return    (reset_instance.first->id.device == key->id.device)
                               && (reset_instance.first->definition->class_def.lock()->base_path == key->definition->class_def.lock()->base_path);
                    });
                    if(find_result == instance_resets.end())
                    {
                        for(size_t i = 0; i < ids_to_write.size(); ++i)
                        {
                            if(ids_to_write[i].param_id == key->id)
                            {
                                auto result_idx = result_position[i];
                                result[result_idx].status = status_codes::instance_key_not_writeable;
                                result[result_idx].message = "Parameter is not writeable because it is an instance key.";
                                WC_DEBUG_LOG("Parameter [" + std::to_string(result_idx) + "] " + to_short_description(key->id) + ": " + to_string(result[result_idx].status));
                                ids_to_write.erase(std::next(ids_to_write.begin(), i));
                                result_position.erase(std::next(result_position.begin(), i));
                                write_set_valid = false;
                                break;
                            }
                        }
                    }
                }
            }
            if(write_set_valid)
            {
                write_set_valid = check_for_same_base_class(*m_model, instance_resets, ids_to_write, result_position,
                                                            provider, providers_with_invalid_set, result);
            }
            if(write_set_valid)
            {
                write_set_valid = check_values_for_all_new_instances(*m_model, instance_resets, ids_to_write, result_position,
                                                                     provider, providers_with_invalid_set, result);
            }
            if(write_set_valid)
            {
                write_set_valid = check_for_instances_extra_values(*m_model, instance_resets, ids_to_write, result_position,
                                                                   provider, providers_with_invalid_set, result);
            }
        }
        // If a parameterset contains some valid values and an invalid one, how should we proceed?
        // We don't want to leave the invalid value out, because then the parameter_provider has no chance of detecting the inconsistency, possibly making changes that the client did not want.
        // We also don't want to just give the whole set to the parameter_provider, because then parameter_providers can't rely on a valid model and have to make much more checks.
        // Instead, we reject the whole set meant for this parameter_provider and give the client a chance to formulate a consistent, valid set again.
        if(contains(providers_with_invalid_set, provider)) // Do NOT use `else` here: New check is required!
        {
            WC_DEBUG_LOG("Some invalid parameters for provider " + provider->display_name() + ". Rejecting the whole set.");
            for (size_t idx = 0; idx < instances->size(); ++idx)
            {
                if (result[idx].is_determined())
                {
                    continue;
                }
                auto& instance = instances->at(idx);
                if (instance->provider == provider)
                {
                    result[idx].status = status_codes::other_invalid_value_in_set;
                    result[idx].message = "Other parameter values are invalid that might have to be consistent with this value.";
                }
            }
            continue;
        }
        try
        {
            // TODO: make sure only one call at a time comes through
            // TODO: make sure no call to GetParameterValues/InvokeMethod is pending

            for (size_t idx = 0; idx < instances->size(); ++idx)
            {
                if (result[idx].is_determined())
                {
                    continue;
                }
                auto& instance = instances->at(idx);
                if (instance->provider == provider)
                {
                    result[idx].status = status_codes::internal_error; // this status will be overwritten by the integration of results, except when an exception occurs.
                    result[idx].message = "Request was not handled.";
                }
            }
            auto job = make_shared<partial_job<vector<set_parameter_response>, vector<set_parameter_response>>>(this);
            WC_DEBUG_LOG("Calling ParameterProvider " + provider->display_name() + " with " + std::to_string(ids_to_write.size()) + " parameters");
            job->future = provider->set_parameter_values_connection_aware(std::move(ids_to_write), defer_wda_web_connection_changes);
            job->integrate = ([result_position, this](auto provider_result, auto whole_result) {
                for (size_t idx = 0, e = provider_result.size(); idx < e; ++idx)
                {
                    size_t pos = result_position[idx];
                    auto& response = provider_result[idx];
                    auto& single_result = whole_result->at(pos);

                    // FIXME: Copy whole response instead of single members?
                    if (response.has_error())
                    {
                        single_result.status = response.status;
                        single_result.domain_specific_status_code = response.domain_specific_status_code;
                        single_result.message = response.message;
                    }
                    else if(response.is_determined())
                    {
                        single_result.status = response.status;
                        single_result.value  = response.value;
                    }
                    else { // the parameter_provider didn't do anything with the parameter
                        single_result.status = status_codes::parameter_not_provided;
                        single_result.message = "Associated parameter provider did not handle this parameter.";
                    }
                    WC_DEBUG_LOG("Response [" + std::to_string(idx) + "] from provider was " + to_string(response.status) + " -> Response [" + std::to_string(pos) + "] from core is " + to_string(single_result.status));
                }
            });
            jobs->push_back(job);
        }
        catch(exception& ex) {
            wc_log(log_level_t::error, "Exception during setting parameters: " + std::string(ex.what()));
        }
        catch (...)
        {
            wc_log(log_level_t::error, "Exception during setting parameters");
        }
    }

    integrate_and_subscribe_next<vector<set_parameter_response>, vector<set_parameter_response>>(jobs, make_shared<vector<set_parameter_response>>(std::move(result)), p);

    return p->get_future();
}

future<vector<set_parameter_response>> parameter_service_core::set_parameter_values(vector<value_request> value_requests)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto param_resps = vector<parameter_response>(value_requests.size());
    auto param_ids = mapped_list<value_request, parameter_instance_id>(
        value_requests, [](const value_request& vr) { return vr.param_id; });

    auto instances = get_parameter_instances(*param_ids.get_iterator(), param_resps);

    vector<set_parameter_response> result;
    result.reserve(param_resps.size());
    for(auto& r : param_resps)
    {
        result.push_back(set_parameter_response(r.status));
    }
    auto param_values = mapped_list<value_request, shared_ptr<parameter_value>>(
        value_requests, [](const value_request& vr) { return vr.value; });

    return set_parameter_values_internal(instances, std::move(result), *param_values.get_iterator(), false);
}

future<vector<set_parameter_response>> parameter_service_core::set_parameter_values_by_path(vector<value_path_request> value_path_requests) {
    return set_parameter_values_by_path_connection_aware(std::move(value_path_requests), false);
}

future<vector<set_parameter_response>> parameter_service_core::set_parameter_values_by_path_connection_aware(vector<value_path_request> value_path_requests, bool defer_wda_web_connection_changes)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto param_resps = vector<parameter_response>(value_path_requests.size());
    auto param_paths = mapped_list<value_path_request, parameter_instance_path>(
        value_path_requests, [](const value_path_request& vr) { return vr.param_path; });

    auto instances = get_parameter_instances(*param_paths.get_iterator(), param_resps);

    vector<set_parameter_response> result;
    result.reserve(param_resps.size());
    for(auto& r : param_resps)
    {
        result.push_back(set_parameter_response(r.status));
    }
    auto param_values = mapped_list<value_path_request, shared_ptr<parameter_value>>(
        value_path_requests, [](const value_path_request& vr) { return vr.value; });

    return set_parameter_values_internal(instances, std::move(result), *param_values.get_iterator(), defer_wda_web_connection_changes);
}

future<parameter_response_list_response> parameter_service_core::get_all_parameters_internal(parameter_filter                const &filter,
                                                                                        size_t                                 paging_offset,
                                                                                        size_t                                 paging_limit,
                                                                                        bool                                   first_phase,
                                                                                        std::vector<parameter_response> const &dyn_instantiation_responses,
                                                                                        bool                                   only_definitions)
{
    // either _only_methods or _without_methods is expected to be set by (internal) caller
    WC_ASSERT(filter._only_methods || filter._without_methods);

    bool filter_path = !filter._only_subpath.empty();
    path subpath(filter._only_subpath);
    bool apply_paging = paging_offset > 0 || paging_limit != SIZE_MAX;
    bool ask_dynamic_instances = false;
    std::vector<provider_read_portion> provider_portions;
    std::vector<parameter_response> responses;
    size_t entries_count = 0;

    {
        lock_guard<std::mutex> guard(m_param_mutex);

        // Gather parameter_instances
        auto instances = make_shared<vector<parameter_instance*>>();
        for (auto& coll : m_device_collections)
        {
            for(auto& device : coll)
            {
                if(device && is_match(device, filter._device))
                {
                    if(filter_path)
                    {
                        // if dynamic: param(s) with PLACEHOLDER will be present; and instantation parameters too
                        append(*instances, device->parameter_instances.get_all_under_subpath(subpath));
                    }
                    else
                    {
                        append(*instances, device->parameter_instances.get_all());
                    }
                }
            }
        }

        auto filter_fn = [filter, this](parameter_instance* const& inst)
        {
            if(inst->id.instance_id == DYNAMIC_PLACEHOLDER_INSTANCE_ID)
            {
                return false;
            }
            if(filter._without_methods && inst->definition->value_type == parameter_value_types::method)
            {
                return false;
            }
            if(filter._only_methods && inst->definition->value_type != parameter_value_types::method)
            {
                return false;
            }
            if(filter._without_file_ids && inst->definition->value_type == parameter_value_types::file_id)
            {
                return false;
            }
            if(filter._only_file_ids && inst->definition->value_type != parameter_value_types::file_id)
            {
                return false;
            }
            if(filter._without_beta && inst->definition->is_beta)
            {
                return false;
            }
            if(filter._only_beta && !inst->definition->is_beta)
            {
                return false;
            }
            if(filter._without_deprecated && inst->definition->is_deprecated)
            {
                return false;
            }
            if(filter._only_deprecated && !inst->definition->is_deprecated)
            {
                return false;
            }
            if(filter._without_usersettings && (inst->definition->user_setting && !inst->definition->overrideables.inactive))
            {
                return false;
            }
            if(filter._only_usersettings && (!inst->definition->user_setting || inst->definition->overrideables.inactive))
            {
                return false;
            }
            if(filter._without_writeable && (inst->definition->writeable && !inst->definition->overrideables.inactive))
            {
                return false;
            }
            if(filter._only_writeable && (!inst->definition->writeable || inst->definition->overrideables.inactive))
            {
                return false;
            }
            if(!filter._only_features.empty())
            {
                // feature of instance currently looked at
                auto& instance_feature = inst->definition->feature_def;
                if(instance_feature.expired())
                {
                    wc_log(log_level_t::warning, "Parameter definition '" + inst->definition->path + "' (id=" + std::to_string(inst->definition->id) + ") belongs to no feature");
                    return false;
                }

                // check if instance feature is contained in feature list of filter
                auto instance_feature_name = instance_feature.lock()->name;
                auto instance_feature_found_in_filter = 
                    filter._only_features.end() != std::find_if(
                        filter._only_features.begin(), filter._only_features.end(), 
                        [instance_feature_name](auto const &filter_feature_name) {
                            return device_model::names_equal(instance_feature_name, filter_feature_name);
                        });

                if(!instance_feature_found_in_filter)
                {
                    return false;
                }
            }
            return true;
        };

        auto second_phase_filter_fn = [&filter_path, &subpath, &filter_fn](parameter_instance* const& inst) {
            if(!filter_fn(inst))
            {
                return false;
            }
            if(!filter_path)
            {
                return true;
            }
            auto original_path = to_lower_copy(parameter_instance_collection::build_parameter_instance_path(inst).m_path);
            auto sub_path      = to_lower_copy(subpath.m_path);
            return (original_path == sub_path) || starts_with(original_path, sub_path + '/');
        };

        // Apply filter and gather dynamic_instantiation_params
        auto dynamic_instantiation_params = make_shared<vector<parameter_instance*>>();
        instances->erase(std::remove_if(instances->begin(), instances->end(), [filter_fn, dynamic_instantiation_params, this](auto& inst) {
            if(inst->definition->value_type == parameter_value_types::instantiations && !inst->fixed_value)
            {
                dynamic_instantiation_params->push_back(inst);
                return true;
            }
            return !filter_fn(inst);
        }), instances->end());

        bool found_dyn_instantiations = !dynamic_instantiation_params->empty();
        auto first_dyn_instantiations_idx = instances->size();

        if(first_phase)
        {
            append(*instances, *dynamic_instantiation_params); // now all the dynamic stuff is at the end
        }
        else
        {
            append_if<parameter_instance*>(*instances, *dynamic_instantiation_params, second_phase_filter_fn);

            // generate the parameter_instances corresponding to the dyn_instantiations
            // this is also the reason we later have to use the original dyn_instantiations in the response, because they are consistent with the rest
            for(auto const &di : dyn_instantiation_responses)
            {
                if(di.status != status_codes::success || !di.value)
                {
                    continue;
                }
                auto device = get_device_internal(di.id.device);
                if(!device)
                {
                    continue;
                }
                auto instantiations = di.value->get_instantiations();
                for(class_instantiation_internal instantiation : instantiations)
                {
                    model_resolver::prepare_class_instantiation(instantiation, *m_model, false);
                    for(auto const &def : instantiation.collected_classes.resolved_parameter_definitions)
                    {
                        auto *inst = device->parameter_instances.get_instance(parameter_instance_id(def->id, instantiation.id));
                        if(inst != nullptr && second_phase_filter_fn(inst))
                        {
                            instances->push_back(inst);
                        }
                    }
                }
            }
        }

        entries_count = instances->size();
        auto instances_to_ask = instances;

        // paging
        if(apply_paging)
        {
            if((!found_dyn_instantiations || !first_phase) && (paging_offset >= entries_count))
            {
                parameter_response_list_response resp(status_codes::success);
                resp.total_entries = entries_count;
                return resolved_future(std::move(resp));
            }
            auto max_to_get = min(paging_limit, entries_count);
            auto lastIdxExclusive = min(paging_offset + max_to_get, entries_count);
            ask_dynamic_instances = first_phase && found_dyn_instantiations && first_dyn_instantiations_idx < lastIdxExclusive;
            if(!ask_dynamic_instances)
            {
                instances_to_ask = make_shared<vector<parameter_instance*>>(instances->begin() + paging_offset, instances->begin() + lastIdxExclusive);
            }
        }
        else
        {
            ask_dynamic_instances = first_phase && found_dyn_instantiations;
        }

        if(ask_dynamic_instances)
        {
            responses.resize(dynamic_instantiation_params->size());
            prepare_get_parameters(provider_portions, dynamic_instantiation_params, responses);
        }
        else
        {
            responses.resize(instances_to_ask->size());
            prepare_get_parameters(provider_portions, instances_to_ask, responses, only_definitions);
        }
    }

    auto p = make_shared<promise<parameter_response_list_response>>();

    if(ask_dynamic_instances)
    {
        auto f = get_parameters_internal(provider_portions, std::move(responses));
        f.set_exception_notifier([this, p](std::exception_ptr eptr) {
            p->set_exception(std::move(eptr));
        });
        f.set_notifier([this, p, filter, paging_offset, paging_limit, only_definitions](auto &&dyn_instantiations) {
            auto second = this->get_all_parameters_internal(filter, paging_offset, paging_limit, false, std::move(dyn_instantiations), only_definitions);
            second.set_exception_notifier([this, p](std::exception_ptr eptr) {
                p->set_exception(std::move(eptr));
            });
            second.set_notifier([p, dyn_instantiations](auto &&resp) {
                // replace new instantiations values with old values for consistency
                for(auto& dyn_inst : dyn_instantiations)
                {
                    if(dyn_inst.status != status_codes::success)
                    {
                        continue;
                    }
                    for(auto& r : resp.param_responses)
                    {
                        if(r.id.id == dyn_inst.id.id)
                        {
                            r.value = dyn_inst.value;
                            break;
                        }
                    }
                }
                p->set_value(std::move(resp));
            });
        });
    }
    else if(only_definitions)
    {
        parameter_response_list_response resp(status_codes::success);
        resp.param_responses = std::move(responses);
        resp.total_entries = entries_count;
        p->set_value(std::move(resp));
    }
    else
    {
        auto f = get_parameters_internal(provider_portions, std::move(responses));
        f.set_exception_notifier([this, p](std::exception_ptr eptr) {
            p->set_exception(std::move(eptr));
        });
        f.set_notifier([this, p, entries_count](auto &&r) {
            parameter_response_list_response resp(status_codes::success);
            resp.param_responses = std::move(r);
            resp.total_entries = entries_count;
            p->set_value(std::move(resp));
        });
    }
    return p->get_future();
}

std::shared_ptr<monitoring_list_collection::monitoring_list> parameter_service_core::get_monitoring_list_internal(monitoring_list_id_t id, status_codes &status)
{
    return m_monitoring_lists.get_monitoring_list(id, status); // parasoft-suppress CERT_C-CON43-a "Mutex is used by caller."
}

future<parameter_response_list_response> parameter_service_core::get_all_parameters(parameter_filter filter, size_t paging_offset, size_t paging_limit)
{
    // Strategy:
    // 1. Gather (filtered) parameter_instances, and sort the dynamic instantiation parameter_instances last
    // 2. Apply paging
    // 3. If no dynamic instantiation parameter_instances exist or current paging window hasn't reached dynamic stuff yet -> Ask parameter_providers for parameter_instances -> done
    // 4. Else: 
    // 5. Find out all dynamic class_instantiations (not only those inside the current paging window!) from parameter_providers
    // 6. Second pass of the above steps, but this time append the dynamic parameter_instances before paging
    // 7. For consistency of the dynamic class_instantiations with their associated parameter_instances,
    //    discard the new values for dynamic class_instantiations (of Step 6) and replace them with the old responses (of Step 5).
    // NOTE: Before the paging window reaches the dynamic stuff, the total_entries number is not accurate.
    // Otherwise, we would need to always ask the parameter_providers for their dynamic instantiations.

    return get_all_parameters_internal(parameter_filter::without_methods() | filter, paging_offset, paging_limit, true, {}, false);
}

future<parameter_response_list_response> parameter_service_core::get_all_parameter_definitions(parameter_filter filter, size_t paging_offset, size_t paging_limit)
{
    return get_all_parameters_internal(parameter_filter::without_methods() | filter, paging_offset, paging_limit, true, {}, true);
}

future<parameter_response_list_response> parameter_service_core::get_all_method_definitions(parameter_filter filter, size_t paging_offset, size_t paging_limit)
{
    return get_all_parameters_internal(parameter_filter::only_methods() | filter, paging_offset, paging_limit, true, {}, true);
}

future<monitoring_list_response> parameter_service_core::create_monitoring_list(std::vector<parameter_instance_id> ids, uint16_t timeout_seconds)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<parameter_response>(ids.size());
    auto instances = get_parameter_instances(ids, result);

    status_codes status = status_codes::internal_error;
    auto monitoring_list_info = m_monitoring_lists.create_monitoring_list(instances, result, timeout_seconds, status);
    monitoring_list_response response(status);
    response.monitoring_list = monitoring_list_info;
    return resolved_future(std::move(response));
}

future<monitoring_list_response> parameter_service_core::create_monitoring_list_with_paths(std::vector<parameter_instance_path> paths, uint16_t timeout_seconds)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto result = vector<parameter_response>(paths.size());
    auto instances = get_parameter_instances(paths, result);

    status_codes status = status_codes::internal_error;
    auto monitoring_list_info = m_monitoring_lists.create_monitoring_list(instances, result, timeout_seconds, status);

    monitoring_list_response response(status);
    response.monitoring_list = monitoring_list_info;
    return resolved_future(std::move(response));
}

future<monitoring_list_values_response> parameter_service_core::get_values_for_monitoring_list(monitoring_list_id_t id)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    status_codes status = status_codes::internal_error;
    auto monitoring_list = m_monitoring_lists.get_monitoring_list(id, status);
    if(status == status_codes::success)
    {
        auto p = make_shared<promise<monitoring_list_values_response>>();
        auto& results = monitoring_list->results;
        for(size_t idx=0, e=results.size(); idx < e; idx++)
        {
            if(monitoring_list->parameter_instances->at(idx) != nullptr)
            {
                results[idx].status = status_codes::no_error_yet;
                results[idx].value = nullptr;
            }
        }
        auto f = get_parameters_internal(monitoring_list->parameter_instances, results);
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

future<monitoring_list_response> parameter_service_core::get_monitoring_list(monitoring_list_id_t id)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    monitoring_list_response response;
    response.monitoring_list = m_monitoring_lists.get_monitoring_list_info(id, response.status);
    return resolved_future(std::move(response));
}

future<delete_monitoring_list_response> parameter_service_core::delete_monitoring_list(const monitoring_list_id_t id)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    auto status = m_monitoring_lists.delete_monitoring_list(id);
    return resolved_future(delete_monitoring_list_response{status});
}

future<monitoring_lists_response> parameter_service_core::get_all_monitoring_lists()
{
    lock_guard<std::mutex> guard(m_param_mutex);
    monitoring_lists_response response;
    response.monitoring_lists = m_monitoring_lists.get_monitoring_list_infos();
    response.status = status_codes::success;
    return resolved_future(std::move(response));
}

//*************************************************************************
// ******** Implementation of IParameterServiceBackend here
// All register-provider methods lock the providers-mutex on top level for consistent changes on providers.
// Information provided might change devices and their parameter-instances. In those cases, the param-mutex is acquired when needed.
// The register_devices method locks nothing on top level, but locks are acquired inside when needed.
//*************************************************************************
wago::future<vector<response>> parameter_service_core::register_devices(vector<register_device_request> requests)
{
    auto result = vector<response>();

    for (auto& request : requests)
    {
        auto& device_id = request.device_id;
        status_codes status = status_codes::no_error_yet;

        {
            // TODO: Check why param mutex is necessary here
            lock_guard<std::mutex> guard(m_param_mutex);
            auto dev = get_device_internal(device_id, status);

            if(has_error(status) && status != status_codes::unknown_device)
            {
                result.push_back(response{status});
                continue;
            }

            if (dev != nullptr)
            {
                result.push_back(response{status_codes::device_already_exists});
                continue;
            }
        }

        try
        {
            auto dev = make_shared<device>(device_id, request.order_number, request.firmware_version);

            m_device_description_providers.for_each([this, dev](auto provider, auto& selectors) {
                this->load_matched_device_informations(dev, provider, selectors);
            });
            m_device_extension_providers.for_each([this, dev](auto provider, auto& response) {
                (void)(provider);
                this->load_matched_device_informations(dev, response);
            });

            {
                lock_guard<std::mutex> guard(m_param_mutex);

                this->m_device_collections[device_id.device_collection_id][device_id.slot] = dev;

                // special parameters in the WAGO-Model. Set them if available.
                // TODO: Replace magic numbers
                set_builtin_value(parameter_value::create(request.order_number), 1, dev); // FIXME: May throw parameter_exception!
                set_builtin_value(parameter_value::create(request.firmware_version), 6, dev); // FIXME: May throw parameter_exception!
            }

            result.push_back(response{status_codes::success});
            wc_log(log_level_t::info, "Loaded device " + wda_ipc::to_string(device_id) + " with orderNumber '" + request.order_number + "'");
        }
        catch (parameter_exception& ex)
        {
            wc_log(log_level_t::warning, "Could not register device with orderNumber '" + request.order_number + "':  " + std::string(ex.what()));
            result.push_back(response{ex.get_error_code()});
        }
        catch (exception& ex)
        {
            wc_log(log_level_t::error, "Could not register device: " + std::string(ex.what()));
            result.push_back(response{status_codes::internal_error});
        }
        catch (...)
        {
            wc_log(log_level_t::error, "Could not register device: Unknown exception");
            result.push_back(response{status_codes::internal_error});
        }
    }

    return resolved_future(std::move(result));
}

wago::future<std::vector<response>> parameter_service_core::unregister_devices(std::vector<device_id> device_ids)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    vector<response> result(device_ids.size());
    for(size_t idx = 0, e = device_ids.size(); idx < e; idx++)
    {
        auto id = device_ids[idx];
        auto dev = get_device_internal(id, result[idx].status);
        if(result[idx].has_error())
        {
            continue;
        }
        if(dev != nullptr)
        {
            this->m_device_collections[id.device_collection_id][id.slot] = nullptr;
            result[idx].status = status_codes::success;
            wc_log(log_level_t::info, "Unloaded device " + wda_ipc::to_string(id));
        }
        else
        {
            result[idx].status = status_codes::unknown_device;
        }
    }
    return resolved_future(std::move(result));
}

wago::future<response> parameter_service_core::unregister_all_devices(device_collection_id_t device_collection)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    response result;

    auto const *devices = get_device_collection(device_collection);
    if (devices == nullptr)
    {
        result.status = status_codes::unknown_device_collection;
        return resolved_future(std::move(result));
    }

    for (size_t idx = 0, l = devices->size(); idx < l; idx++)
    {
        this->m_device_collections[device_collection][idx] = nullptr;
    }

    result.status = status_codes::success;
    wc_log(log_level_t::info, "Unloaded devices for collection " + std::to_string(device_collection));
    return resolved_future(std::move(result));
}

bool parameter_service_core::is_match(std::shared_ptr<device> const &device,
                                 device_selector         const &selector)
{
    if(!selector.is_any_selector())
    {
        if(selector.is_collection_selector())
        {
            if(selector.get_selected_device().device_collection_id != device->id.device_collection_id)
            {
                return false;
            }
        }
        else if(selector.get_selected_device() != device->id)
        {
            return false;
        }
    }
    return true;
}

void parameter_service_core::match_selected_parameters(std::shared_ptr<device>         const &device,
                                                  parameter_provider_i                  *provider,
                                                  std::vector<parameter_selector>       &selectors)
{
    if(!device)
    {
        return;
    }
    for(auto& selector : selectors)
    {
        if(!is_match(device, selector.get_selected_devices()))
        {
            continue;
        }
        feature_definition* feature = nullptr;
        class_definition* cls = nullptr;
        if(selector.get_selector_type() == parameter_selector_type::feature)
        {
            // ignore if device doesn't have the selected feature at all
            auto& features = device->collected_features.resolved_includes;
            if(std::find(features.begin(), features.end(), selector.get_selected_name()) == features.end())
            {
                continue;
            }
            auto& ff = m_model->features;
            auto r = std::find_if(ff.begin(), ff.end(), [&](shared_ptr<feature_definition> const &def) { return def->name == selector.get_selected_name(); });
            if(r != ff.end())
            {
                feature = r->get();
            }
        }
        else if (selector.get_selector_type() == parameter_selector_type::cls)
        {
            auto& ff = m_model->classes;
            auto r = std::find_if(ff.begin(), ff.end(), [&](shared_ptr<class_definition> const &def) { return def->name == selector.get_selected_name(); });
            if(r != ff.end())
            {
                cls = r->get();
            }
        }
        
        auto belongs_to = [](parameter_definition_owner* po, parameter_instance* inst) {
            auto& params = po->resolved_parameter_definitions;
            return std::find_if(params.begin(), params.end(), [inst, &params](shared_ptr<parameter_definition> const &def){ return def->id == inst->id.id; }) != params.end(); // parasoft-suppress CERT_CPP-EXP61-b-2 "Inner lambda is executed synchronously, lifetime will not be longer than variabes from outer lambda"
        };
        auto belongs_to_class = [&](class_definition* po, parameter_instance* inst) {
            return belongs_to(po, inst) || 
            (po->is_dynamic && po->base_id == inst->id.id);
        };
        auto belongs_to_feature = [&](feature_definition* po, parameter_instance* inst) {
            if(belongs_to(po, inst)) { return true; }

            if((inst->definition != nullptr) && (!inst->definition->feature_def.expired()))
            {
                auto const feature_candidate = inst->definition->feature_def.lock();
                return feature_candidate->is_part_of(*po);
            }

            if(!po->classes.empty())
            {
                // FIXME: Is this case possible?
                // If not, remove fallback
                wc_log(log_level_t::debug, "Searching classes caused due to missing feature definition reference for ID " + std::to_string(inst->id.id));
            }
            for(auto& c : po->classes)
            {
                auto cc = m_model->find_class_definition(c);
                if(!cc) {continue;}
                if(belongs_to_class(cc.get(), inst)) { return true; }
            }

            return false;
        };
        for (auto const &instance : device->parameter_instances.get_all())
        {
            if(instance->fixed_value)
            {
                continue;
            }
            if(    selector.get_selector_type() == parameter_selector_type::parameter_definition
                && selector.get_selected_id() != instance->id.id)
            {
                continue;
            }
            if(     selector.get_selector_type() == parameter_selector_type::feature
                && (    feature == nullptr
                    || !belongs_to_feature(feature, instance)))
            {
                continue;
            }
            if(     selector.get_selector_type() == parameter_selector_type::cls
                && (    cls == nullptr
                    || !belongs_to_class(cls, instance)))
            {
                continue;
            }
            if (instance->provider != nullptr && instance->provider != provider)
            {
                wc_log(log_level_t::warning, "ParameterInstance " + to_description(instance->id) + " was already provided, overwriting");
                // it is not fatal to 'overprovide' parameters.
            }
            instance->provider = provider;
        }
    }
}
bool parameter_service_core::remove_provider(parameter_provider_i* provider)
{
    parameter_provider_i *removed_provider = nullptr;
    if(m_providers.remove(provider))
    {
        std::lock_guard<std::mutex> guard(m_param_mutex);
        unprovide(provider);
        removed_provider = provider;
    }
    else
    {
        auto* wrapper_pp = m_providers.find([&provider](auto pp, auto data) { return data.wrapped_provider == provider; });
        if(wrapper_pp != nullptr && m_providers.remove(wrapper_pp))
        {
            std::lock_guard<std::mutex> guard(m_param_mutex);
            unprovide(wrapper_pp);
            removed_provider = wrapper_pp;
        }
    }
    std::lock_guard<std::mutex> guard(m_upload_ids_mutex);
    for(auto it = m_upload_ids.begin(); it != m_upload_ids.end();)
    {
        if(it->second.creator_provider == removed_provider)
        {
            it = m_upload_ids.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return removed_provider != nullptr;
}

bool parameter_service_core::remove_provider(model_provider_i* provider)
{
    return m_model_providers.remove(provider);
}

bool parameter_service_core::remove_provider(device_description_provider_i* provider)
{
    return m_device_description_providers.remove(provider);
}

bool parameter_service_core::remove_provider(device_extension_provider_i* provider)
{
    // FIXME: retract information from device_extension_provider_i and remove extension functionality
    return m_device_extension_providers.remove(provider);
}

template <typename T>
wago::future<std::vector<response>> parameter_service_core::register_providers(std::vector<T*> providers)
{
    auto result = vector<response>(providers.size());
    size_t idx = 0;
    for(auto *p : providers)
    {
        result[idx].status = add_provider(p);
        idx++;
    }
    return resolved_future(std::move(result));
}

template <typename T>
void parameter_service_core::unregister_providers(std::vector<T*> providers)
{
    for (auto *provider : providers)
    {
        if (!remove_provider(provider))
        {
            wc_log(log_level_t::warning, "Could not unregister provider, was not found");
        }
    }
}

wago::future<vector<response>> parameter_service_core::register_parameter_providers(vector<parameter_provider_i*> providers, parameter_provider_call_mode mode)
{
    auto result = vector<response>(providers.size());
    size_t idx = 0;
    for(auto *p : providers)
    {
        if(mode == parameter_provider_call_mode::serialized)
        {
            auto wrapper = make_shared<serial_parameter_provider>(p);
            result[idx].status = add_provider(wrapper.get(), wrapper, p);
        }
        else
        {
            result[idx].status = add_provider(p, nullptr, nullptr);
        }
        idx++;
    }
    return resolved_future(std::move(result));
}

void parameter_service_core::unregister_parameter_providers(vector<parameter_provider_i*> providers)
{
    unregister_providers(providers);
}

wago::future<vector<response>> parameter_service_core::register_model_providers(vector<model_provider_i*> providers)
{
    return register_providers(providers);
}

void parameter_service_core::unregister_model_providers(vector<model_provider_i*> providers)
{
    unregister_providers(providers);
}

wago::future<vector<response>> parameter_service_core::register_device_description_providers(vector<device_description_provider_i*> providers)
{
    return register_providers(providers);
}

void parameter_service_core::unregister_device_description_providers(vector<device_description_provider_i*> providers)
{
    unregister_providers(providers);
}

wago::future<vector<response>> parameter_service_core::register_device_extension_providers(vector<device_extension_provider_i*> providers)
{
    return register_providers(providers);
}

void parameter_service_core::unregister_device_extension_providers(vector<device_extension_provider_i*> providers)
{
    unregister_providers(providers);
}

void parameter_service_core::set_builtin_value(std::shared_ptr<parameter_value> const &val,
                                          parameter_id_t                          param_id,
                                          std::shared_ptr<device>          const &dev)
{
    parameter_instance* instance = dev->parameter_instances.get_instance({parameter_instance_id(param_id)});
    if(instance == nullptr)
    {
        auto def = m_model->find_definition(param_id);
        if(!def)
        {
            return;
        }
        dev->parameter_instances.add_instances(dev->id, 0, {def}, {});
        instance = dev->parameter_instances.get_instance({parameter_instance_id(param_id)});
    }
    if(instance != nullptr)
    {
        auto status = check_parameter_value(val, instance->definition);
        if(has_error(status))
        {
            throw parameter_exception(status, instance->definition->path + " did not match model definition: " + status_code_strings[(int)status]);
        }
        instance->fixed_value = val;
    }
}

status_codes parameter_service_core::add_provider(model_provider_i* provider)
{
    if(!m_model_providers.add(provider, {device_selector::any}))
    {
        wc_log(log_level_t::error, "ModelProvider already registered");
        return status_codes::provider_not_operational;
    }

    auto f = provider->get_model_information();
    f.set_exception_notifier([this](std::exception_ptr eptr){
        try { std::rethrow_exception(std::move(eptr)); }
        catch(exception& ex)
        {
            wc_log(log_level_t::error, "ModelProvider could not initialize: " + std::string(ex.what()));
        }
    });
    f.set_notifier([this](wdm_response&& response) {
        if(response.has_error())
        {
            wc_log(log_level_t::error, "ModelProvider could not initialize: " + to_string(response.status));
            return;
        }
        if(!response.wdm_content.empty())
        {
            try
            {
                std::lock_guard<std::mutex> guard(m_param_mutex); // TODO: Mutex for model?
                load_model(response.wdm_content);
                for (auto& coll : m_device_collections)
                {
                    for(auto& device : coll)
                    {
                        if(device == nullptr) {continue;}
                        m_providers.for_each([this, device](auto prov, auto& data) {
                            this->match_selected_parameters(device, prov, data.selectors);
                        });
                    }
                }
            }
            catch (exception& ex)
            {
                wc_log(log_level_t::error, "Could not load model from ModelProvider: " + std::string(ex.what()));
            }
        }
    });
    return status_codes::success;
}

status_codes parameter_service_core::add_provider(device_extension_provider_i* provider)
{
    if(!m_device_extension_providers.add(provider, {}))
    {
        wc_log(log_level_t::error, "DeviceExtensionProvider already registered");
        return status_codes::provider_not_operational;
    }
    try
    {
        // no lock can be present here, otherwise deadlock
        auto response = provider->get_device_extensions();
        if(response.has_error()) 
        {
            wc_log(log_level_t::error, "DeviceExtensionProvider could not initialize: " + to_string(response.status));
            return status_codes::provider_not_operational;
        }
        else
        {
            if(!m_device_extension_providers.update_data(provider, response))
            {
                wc_log(log_level_t::error, "Could update data for unknown DeviceExtensionProvider");
                return status_codes::provider_not_operational;
            }

            for (auto& coll : m_device_collections)
            {
                for(auto& device : coll)
                {
                    load_matched_device_informations(device, response);
                }
            }
            return status_codes::success;
        }
    }
    catch (exception& ex)
    {
        wc_log(log_level_t::error, "DeviceExtensionProvider could not initialize: " + std::string(ex.what()));
        return status_codes::provider_not_operational;
    }
    catch (...)
    {
        wc_log(log_level_t::error, "DeviceExtensionProvider could not initialize: Unknown exception");
        return status_codes::provider_not_operational;
    }
}

status_codes parameter_service_core::add_provider(device_description_provider_i* provider)
{
    // TODO redundant code
    if(!m_device_description_providers.add(provider, {}))
    {
        wc_log(log_level_t::error, "DeviceDescriptionProvider already registered");
        return status_codes::provider_not_operational;
    }
    try
    {
        // no lock can be present here, otherwise deadlock
        auto response = provider->get_provided_devices();
        if(response.has_error())
        {
            wc_log(log_level_t::error, "DeviceDescriptionProvider could not initialize: " + to_string(response.status));
            return status_codes::provider_not_operational; // FIXME: Provider is registered and has to be removed
        }
        else
        {
            if(!m_device_description_providers.update_data(provider, response.selected_devices))
            {
                wc_log(log_level_t::error, "Could update data for unknown DeviceDescriptionProvider");
                return status_codes::provider_not_operational; // FIXME: Provider is registered and has to be removed
            }

            for (auto& coll : m_device_collections)
            {
                for(auto& device : coll)
                {
                    load_matched_device_informations(device, provider, response.selected_devices);
                }
            }
            return status_codes::success;
        }
    }
    catch (exception& ex)
    {
        wc_log(log_level_t::error, "DeviceDescriptionProvider could not initialize: " + std::string(ex.what()));
        return status_codes::provider_not_operational; // FIXME: Provider is registered and has to be removed
    }
    catch (...)
    {
        wc_log(log_level_t::error, "DeviceDescriptionProvider could not initialize: Unknown exception");
        return status_codes::provider_not_operational; // FIXME: Provider is registered and has to be removed
    }
}

// mutex is locked in the notifiers
void parameter_service_core::load_matched_device_informations(std::shared_ptr<device>       const &device,
                                                         device_description_provider_i       *provider,
                                                         std::vector<device_selector>        &selectors)
{
    string orderNumber;
    string fwVersion;
    bool matched = false;
    {
        std::lock_guard<std::mutex> guard(m_param_mutex);
        if(!device)
        {
            return;
        }
        orderNumber = device->get_order_number();
        fwVersion = device->get_firmware_version();
        for(auto& selector : selectors)
        {
            if(is_match(device, selector))
            {
                matched = true;
                break;
            }
        }
    }

    if(matched)
    {    
        auto f = provider->get_device_information(orderNumber, fwVersion);
        f.set_exception_notifier([this, orderNumber, fwVersion](std::exception_ptr eptr) {
            try { std::rethrow_exception(std::move(eptr)); }
            catch(exception& ex)
            {
                wc_log(log_level_t::error, "Could not get information (" + orderNumber + ", " + fwVersion + ") from DeviceInformationProvider: " + std::string(ex.what()));
            }
        });
        f.set_notifier([this, orderNumber, fwVersion, device](wdd_response&& response) {
            try
            {
                if(!response.is_determined())
                {
                    return;
                }
                if(response.has_error())
                {
                    wc_log(log_level_t::warning, "Device description provider has reported problem with (" + orderNumber + ", " + fwVersion + ", " + to_short_description(device->id) + "): " + status_code_strings[(int)response.status]);
                    return;
                }

                std::lock_guard<std::mutex> guard(m_param_mutex);

                // TODO: separate bundle
                wdm_content_t wdm_content;
                wdd_content_t wdd_content = response.content;

                if(!wdm_content.empty())
                {
                    load_model(wdm_content);
                }

                if(!device)
                {
                    return;
                }
                if(!wdd_content.empty())
                {
                        device->add_wdd(std::move(wdd_content), *m_model);
                }
                m_providers.for_each([this, device](auto prov, auto& data)
                {
                    this->match_selected_parameters(device, prov, data.selectors);
                });
            }
            catch (exception& ex)
            {
                wc_log(log_level_t::error, "Could not load information (" + orderNumber + ", " + fwVersion + ") from DeviceInformationProvider: " + std::string(ex.what()));
            }
        });
    }
}

void parameter_service_core::load_matched_device_informations(std::shared_ptr<device>   const &device,
                                                         device_extension_response       &response)
{
    lock_guard<std::mutex> guard(m_param_mutex);
    if(!device)
    {
        return;
    }
    if(is_match(device, response.selected_devices))
    {
        device_description description;
        description.features = response.extension_features;
        device->add_description(description, *m_model);
    }
    // TODO: prepare for retraction of information provided by device_extension_provider_i

    m_providers.for_each([this, device](auto prov, auto& data)
    {
        this->match_selected_parameters(device, prov, data.selectors);
    });
}

// *********************************************
// File API
// All public methods lock the file-mutex on top level for consistency
// *********************************************

wago::future<std::vector<register_file_provider_response>> parameter_service_core::register_file_providers(std::vector<register_file_provider_request> requests, provider_call_mode mode)
{
    lock_guard<std::mutex> guard(m_file_mutex);
    std::vector<register_file_provider_response> result;
    
    for(auto& request : requests)
    {
        file_id id = generate_file_id();
        result.push_back(register_file_provider_internal(request.provider, id, request.context, mode));
    }
    return resolved_future(std::move(result));
}

void parameter_service_core::unregister_file_providers(std::vector<file_transfer::file_provider_i*> providers)
{
    lock_guard<std::mutex> guard(m_file_mutex);
    for(auto& provider : providers)
    {
        auto *found = m_file_providers.find([provider](auto p, auto& data) {
            return p == provider || data.wrapped_provider == provider;
        });
        if (found == nullptr || !m_file_providers.remove(found))
        {
            wc_log(log_level_t::warning, "Could not unregister file provider, was not found");
        }
    }
}

wago::future<std::vector<register_file_provider_response>> parameter_service_core::reregister_file_providers(std::vector<reregister_file_provider_request> requests, provider_call_mode mode) 
{
    lock_guard<std::mutex> guard(m_file_mutex);
    std::vector<register_file_provider_response> result;
    result.reserve(requests.size());
    for(auto& request : requests)
    {
        result.push_back(register_file_provider_internal(request.provider, request.id, request.context, mode));
    }
    return resolved_future(std::move(result));
}

register_file_provider_response parameter_service_core::register_file_provider_internal(
    file_transfer::file_provider_i        *provider,
    file_id                         const &file_id_,
    parameter_id_t                         context,
    provider_call_mode                     mode
    )
{
    auto* existing = get_file_provider(file_id_);
    if(existing != nullptr)
    {
        wc_log(log_level_t::warning, "Attempting to register file_provider for existing file_id " + file_id_);
        return register_file_provider_response(status_codes::provider_not_operational);
    }

    if(m_model->find_definition(context) == nullptr)
    {
        wc_log(log_level_t::warning, "Attempting to register file_provider with unknown context (" + std::to_string(context) + ")");
        return register_file_provider_response(status_codes::provider_not_operational);
    }

    bool added = false;
    if(mode == provider_call_mode::serialized)
    {
        auto wrapper = make_shared<serial_file_provider>(provider);
        added = m_file_providers.add(wrapper.get(), {file_id_, context, wrapper, provider });
    }
    else
    {
        added = m_file_providers.add(provider, {file_id_, context, nullptr, nullptr, });
    }

    if(!added)
    {
        return register_file_provider_response(status_codes::provider_not_operational);
    }
    else
    {
        return register_file_provider_response(file_id_);
    }
}

wago::future<file_read_response> parameter_service_core::file_read(file_id id, uint64_t offset, size_t length)
{
    expire_lapsed_upload_ids_except(id);
    lock_guard<std::mutex> guard(m_file_mutex);
    auto *provider = get_file_provider(id);
    if(provider == nullptr)
    {
        return resolved_future(file_read_response(status_codes::unknown_file_id));
    }
    return provider->read(offset, length);
}
wago::future<response> parameter_service_core::file_write(file_id id, uint64_t offset, std::vector<uint8_t> data)
{
    expire_lapsed_upload_ids_except(id);
    lock_guard<std::mutex> guard(m_file_mutex);
    auto *provider = get_file_provider(id);
    if(provider == nullptr)
    {
        return resolved_future(response{status_codes::unknown_file_id});
    }
    return provider->write(offset, std::move(data));
}

wago::future<response> parameter_service_core::file_create(file_id id, uint64_t capacity)
{
    expire_lapsed_upload_ids_except(id);
    lock_guard<std::mutex> guard(m_file_mutex);
    auto *provider = get_file_provider(id);
    if(provider == nullptr)
    {
        return resolved_future(response{status_codes::unknown_file_id});
    }
    return provider->create(capacity);
}

wago::future<file_info_response> parameter_service_core::file_get_info(file_id id)
{
    expire_lapsed_upload_ids_except(id);
    lock_guard<std::mutex> guard(m_file_mutex);
    auto *provider = get_file_provider(id);
    if(provider == nullptr)
    {
        return resolved_future(file_info_response(status_codes::unknown_file_id));
    }
    return provider->get_file_info();
}

wago::future<file_id_response> parameter_service_core::create_parameter_upload_id(parameter_instance_path context, uint16_t timeout_seconds)
{
    expire_lapsed_upload_ids_except("");
    {
        lock_guard<std::mutex> guard(m_upload_ids_mutex);
        if(m_upload_ids.size() >= 100)
        {
            return resolved_future(file_id_response(status_codes::upload_id_max_exceeded));
        }
    }
    lock_guard<std::mutex> guard(m_param_mutex);
    file_id_response result;
    
    try
    {
        auto* instance = get_parameter_instance(context, &result);
        if (!result.has_error())
        {
            if (instance->provider == nullptr)
            {
                return resolved_future(file_id_response(status_codes::parameter_not_provided));
            }
            if(instance->definition->value_type != parameter_value_types::file_id)
            {
                return resolved_future(file_id_response(status_codes::not_a_file_id));
            }
            auto cid = instance->definition->id;
            return mapped_future<file_id_response, file_id_response>(instance->provider->create_parameter_upload_id(cid), [this, cid, timeout_seconds, instance](auto r) {
                if(r.is_success())
                {
                    lock_guard<std::mutex> upload_id_guard(m_upload_ids_mutex);
                    file_id fid = r.registered_file_id;
                    m_upload_ids.insert({fid, upload_id_data{fid, cid, timeout_seconds, std::chrono::system_clock::now(), instance->provider}});
                    return r;
                }
                else
                {
                    wc_log(log_level_t::warning, "ParameterProvider responded to create_parameter_upload_id with " + to_string(r.status));
                    return file_id_response(status_codes::parameter_not_provided);
                }
            });
        }
        else
        {
            return resolved_future(std::move(result));
        }
    }
    catch(const std::exception& ex)
    {
        wc_log(log_level_t::error, "Exception in ParameterProvider create_parameter_upload_id: " + std::string(ex.what()));
        return resolved_future(file_id_response(status_codes::internal_error));
    }
    catch(...)
    {
        wc_log(log_level_t::error, "Unknown exception in ParameterProvider create_parameter_upload_id");
        return resolved_future(file_id_response(status_codes::internal_error));
    }
}

void parameter_service_core::expire_lapsed_upload_ids_except(file_id const &id_with_heartbeat)
{
    lock_guard<std::mutex> guard(m_upload_ids_mutex);
    auto it = m_upload_ids.begin();
    while(it != m_upload_ids.end())
    {
        auto& data = it->second;
        if(!id_with_heartbeat.empty() && data.id == id_with_heartbeat)
        {
            data.last_access = std::chrono::system_clock::now();
        }
        else
        {
            auto delta = std::chrono::system_clock::now() - data.last_access;
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(delta).count();
            if(elapsed_seconds > data.timeout_seconds)
            {
                wc_log(log_level_t::warning, "Upload-ID " + data.id + " has lapsed, attempting to clean up");
                auto d = data;
                it = m_upload_ids.erase(it); // data is discarded here
                auto f = d.creator_provider->remove_parameter_upload_id(d.id, d.context);
                f.set_exception_notifier([this](auto e_ptr) {
                    (void)(e_ptr);
                    wc_log(log_level_t::error, "ParameterProvider encountered an exception during remove_parameter_upload_id");
                });
                f.set_notifier([this](auto &&r) {
                    if(r.is_success())
                    {
                        wc_log(log_level_t::info, "ParameterProvider responded to remove_parameter_upload_id with success");
                    }
                    else
                    {
                        wc_log(log_level_t::warning, "ParameterProvider responded to remove_parameter_upload_id with " + to_string(r.status));
                    }
                });
                continue;
            }
        }
        it++;
    }
}

file_id parameter_service_core::generate_file_id()
{
    // see https://en.wikipedia.org/wiki/Birthday_problem#Approximations
    // 36^8 possibilities give a collision probability of ~0.1% for 100000 simultaneously used file-ids.
    return "file"+random_alpha(8);
}

bool parameter_service_core::has_file_id(file_id id) 
{
    return m_file_providers.find([id](auto, auto& data) {
        return data.id == id;
    }) != nullptr;
}

file_provider_i* parameter_service_core::get_file_provider(file_id id)
{
    return m_file_providers.find([id](auto provider, auto& data) {
        (void)(provider);
        return data.id == id;
    });
}

file_context parameter_service_core::get_file_context(file_id id)
{
    auto data = m_file_providers.find_data([id](auto provider, auto& data) {
        (void)(provider);
        return data.id == id;
    });
    if(data == nullptr) return {g_unknown_marker, g_unknown_marker};
    
    auto parameter_def = m_model->find_definition(data->context);
    WC_ASSERT_RETURN(parameter_def.get() != nullptr,( file_context {g_unknown_marker, g_unknown_marker}));

    auto feature_def = parameter_def->feature_def.lock();
    auto class_def = parameter_def->class_def.lock();
    return { 
        (feature_def != nullptr) ? feature_def->name : g_unknown_marker,
        (class_def != nullptr) ? (class_def->base_path + "/*/" + parameter_def->path) : parameter_def->path
    };
}

void parameter_service_core::dump_known_model()
{
    WC_DEBUG_LOG("Known Model: " +
                 std::to_string(m_model->features.size())    + " Features, " +
                 std::to_string(m_model->classes.size())     + " Classes, " +
                 std::to_string(m_model->definitions.size()) + " ParameterDefinitions");
    for(auto& f : m_model->features)
    {
        WC_DEBUG_LOG("Feature " + f->name);
        for(auto& n : f->includes)
        {
            WC_DEBUG_LOG("Includes " + n);
        }
        for(auto& n : f->classes)
        {
            WC_DEBUG_LOG("Contains Class " + n);
        }
        for(auto& d : f->parameter_definitions)
        {
            WC_DEBUG_LOG("Contains ParameterDefinition " + std::to_string(d->id));
        }
    }
    for(auto& c : m_model->classes)
    {
        WC_DEBUG_LOG("Class " + c->name);
        for(auto& n : c->includes)
        {
            WC_DEBUG_LOG("Includes " + n);
        }
        for(auto& d : c->parameter_definitions)
        {
            WC_DEBUG_LOG("Contains ParameterDefinition " + std::to_string(d->id));
        }
    }
}

void parameter_service_core::dump_known_providers()
{
    WC_DEBUG_LOG("Known Providers: " + 
                 std::to_string(m_providers.size())                    + " ParameterProviders, " +
                 std::to_string(m_device_description_providers.size()) + " DeviceDescriptionProviders, " +
                 std::to_string(m_device_extension_providers.size())   + " DeviceExtensionProviders, " +
                 std::to_string(m_model_providers.size())              + " ModelProviders");
    m_providers.for_each([this](auto p, auto& data)
    {
        WC_DEBUG_LOG("ParameterProvider " + p->display_name() + " with selector " + wda_ipc::to_ipc_string(data.selectors));
    });
    m_device_description_providers.for_each([this](auto p, auto& data)
    {
        (void)(p);
        WC_DEBUG_LOG("DeviceDescriptionProvider with selectors " + wda_ipc::to_ipc_string(data));
    });
    m_device_extension_providers.for_each([this](auto p, auto& data)
    {
        (void)(p);
        WC_DEBUG_LOG("DeviceExtensionProvider with selectors " + wda_ipc::to_ipc_string(data));
    });
}

std::vector<parameter_provider_i*> parameter_service_core::get_parameter_providers(std::vector<parameter_instance_path> paths)
{
    auto result = vector<parameter_provider_i*>(paths.size());
    auto res = vector<parameter_response>(paths.size());

    auto instances = get_parameter_instances(paths, res);

    for(size_t i=0;i<paths.size();i++)
    {
        if(res[i].has_error())
        {
            continue;
        }
        result[i] = instances->at(i)->provider;
    }

    return result;
}


user_permissions parameter_service_core::get_full_user_permissions(std::string const &user_name) noexcept
{
    // expand permissions to include all features that are contained implicitly
    auto permissions = m_permissions->get_user_permissions(user_name);

    append_included_features(permissions.read_permissions, *m_model);
    append_included_features(permissions.write_permissions, *m_model);
    return permissions;
}

std::string parameter_service_core::get_permission_name(std::string          const &feature,
                                                        permissions_i::types const  type) const noexcept
{
    return m_permissions->get_permission_name(feature, type);
}

namespace{
bool collect_and_check_parameters(shared_ptr<vector<parameter_instance *>> const &instances,
                                  list_i<shared_ptr<parameter_value>>      const &values,
                                  device_model                             const &model,
                                  reset_map                                      &instance_resets,
                                  vector<parameter_instance*>                    &instance_keys,
                                  vector<value_request>                          &ids_to_write,
                                  vector<size_t>                                 &result_position,
                                  parameter_provider_i                           *provider,
                                  set<parameter_provider_i*>                     &providers_with_invalid_set,
                                  vector<set_parameter_response>                 &result)
{
    for (size_t idx = 0, e = instances->size(); idx < e; ++idx)
    {
        if (result[idx].is_determined())
        {
            continue;
        }
        auto& instance = instances->at(idx);
        if (instance->provider == provider)
        {
            auto current_request = value_request(instance->id, values[idx]);
            if(instance->definition->value_type == parameter_value_types::instantiations)
            {
                ids_to_write.insert(ids_to_write.begin(), current_request); // instantiations come first
                auto const base_class_def = get_class_def_for_instantiation(*instance->definition, model.classes);
                if(base_class_def == nullptr)
                {
                    result[result_position[idx]].status = status_codes::internal_error;
                    result[result_position[idx]].message = "Unable to find base class definition for instantiation.";
                    providers_with_invalid_set.insert(provider);
                    return false;
                }
                instance_resets.emplace(instance, base_class_def);
                result_position.insert(result_position.begin(), idx);
            }
            else if(instance->definition->instance_key)
            {
                instance_keys.push_back(instance);
                ids_to_write.push_back(current_request);
                result_position.push_back(idx);
            }
            else
            {
                ids_to_write.push_back(current_request);
                result_position.push_back(idx);
            }
            WC_DEBUG_LOG("Added [" + std::to_string(idx) + "] " + to_short_description(instance->id));
        }
    }
    return true;
}

bool check_for_same_base_class(device_model                   const &model, 
                               reset_map                      const &instance_resets,
                               vector<value_request>          const &ids_to_write,
                               vector<size_t>                 const &result_position,
                               parameter_provider_i                 *provider,
                               set<parameter_provider_i*>           &providers_with_invalid_set,
                               vector<set_parameter_response>       &result)
{
    for(auto const &instance :instance_resets)
    {
        for(size_t i = 0; i < ids_to_write.size(); ++i)
        {
            auto const &request = ids_to_write[i];
            if(request.param_id == instance.first->id)
            {
                auto instantiations = request.value->get_instantiations();
                for(auto const &instantiation :instantiations)
                {
                    for(auto const &class_name :instantiation.classes)
                    {
                        if(class_name != instance.second->name)
                        {
                            auto class_def = model.find_class_definition(class_name);
                            if(   (class_def == nullptr)
                               || (std::find(class_def->resolved_includes.begin(),
                                             class_def->resolved_includes.end(),
                                             instance.second->name)  == class_def->resolved_includes.end()))
                            {
                                // Not same base class
                                auto idx = result_position[i];
                                result[idx].status = status_codes::invalid_value;
                                providers_with_invalid_set.insert(provider);
                                result[idx].message = "At least one new class instance has a different base class. First invalid instance found: " + class_name;
                                wc_log(log_level_t::notice, "Rejected invalid reset for instantiations parameter " + instance.first->definition->path + ": " + result[i].message);
                                return false;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool check_values_for_all_new_instances(device_model                   const &model, 
                                        reset_map                      const &instance_resets,
                                        vector<value_request>          const &ids_to_write,
                                        vector<size_t>                 const &result_position,
                                        parameter_provider_i                 *provider,
                                        set<parameter_provider_i*>           &providers_with_invalid_set,
                                        vector<set_parameter_response>       &result)
{
    // Ensure values for all new instances in reset case
    for(auto const &instance :instance_resets)
    {
        // Extract new instance numbers and collect parameter IDs needed for an instance
        // Check for present write values
        std::map<parameter_instance_id, std::vector<std::shared_ptr<parameter_definition>>> necessary_parameter_definitions;
        std::vector<parameter_instance_id> new_instances;
        for(size_t i = 0; i < ids_to_write.size();  ++i)
        {
            auto const &request = ids_to_write[i];
            if(request.param_id == instance.first->id)
            {
                auto instantiations = request.value->get_instantiations();
                new_instances.clear();
                new_instances.reserve(instantiations.size());
                for(auto const &instantiation :instantiations)
                {
                    auto instance_id = instance.first->id;
                    instance_id.instance_id = instantiation.id;
                    new_instances.push_back(instance_id);
                    necessary_parameter_definitions.emplace(instance_id,
                                                            std::vector<std::shared_ptr<parameter_definition>>());
                    for(auto const &class_def: model.classes)
                    {
                        std::set<std::string> class_names(instantiation.classes.begin(), instantiation.classes.end());
                        class_names.insert(instance.second->name);
                        for(auto const &class_name: class_names)
                        {
                            if(class_name == class_def->name)
                            {
                                auto &param_vec = necessary_parameter_definitions[instance_id];
                                std::copy_if(class_def->parameter_definitions.begin(),
                                                class_def->parameter_definitions.end(),
                                                std::back_inserter(param_vec),
                                                [](auto const &param_def) {
                                                return param_def->writeable && !param_def->overrideables.inactive;
                                                });
                            }
                        }
                    }
                }
                // Do not break loop because last reset wins (in case of multiple reset requests)
            }
        }
        std::set<parameter_instance_id> new_value_ids;
        for(auto const &request :ids_to_write)
        {
            for(auto const &new_instance: new_instances)
            {
                auto const &necessary_for_instance = necessary_parameter_definitions.at(new_instance);
                auto found_necessary = std::find_if(necessary_for_instance.begin(), necessary_for_instance.end(), [&request, &new_instance](auto const &item){
                    return parameter_instance_id(item->id, new_instance.instance_id, new_instance.device) == request.param_id;
                });
                if(found_necessary != necessary_for_instance.end())
                {
                    // Collect parameter instance ID as satisfied value
                    new_value_ids.insert(request.param_id);
                }
            }
        }
        // Check if values for all new instances are present
        for(auto const instance_id :new_instances)
        {
            for(auto const &necessary_parameter_definition :necessary_parameter_definitions.at(instance_id))
            {
                auto const parameter_id = necessary_parameter_definition->id;
                auto param_instance_id_to_search = parameter_instance_id(parameter_id, instance_id.instance_id, instance.first->id.device);
                if(new_value_ids.count(param_instance_id_to_search) == 0)
                {
                    // Necessary value is missing
                    for(size_t i = 0; i < ids_to_write.size(); ++i)
                    {
                        if(ids_to_write[i].param_id == instance.first->id)
                        {
                            auto idx = result_position[i];
                            result[idx].status = status_codes::missing_parameter_for_instantiation;
                            providers_with_invalid_set.insert(provider);
                            result[idx].message = "At least one parameter value is missing for a new class instance. First missing parameter found: " + necessary_parameter_definition->path;
                            wc_log(log_level_t::notice, "Rejected invalid reset for instantiations parameter " + instance.first->definition->path + ": " + result[i].message);
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool check_for_instances_extra_values(device_model                   const &model, 
                                      reset_map                      const &instance_resets,
                                      vector<value_request>          const &ids_to_write,
                                      vector<size_t>                 const &result_position,
                                      parameter_provider_i                 *provider,
                                      set<parameter_provider_i*>           &providers_with_invalid_set,
                                      vector<set_parameter_response>       &result)
{
    WC_ASSERT(result.size() >= ids_to_write.size());
    WC_ASSERT(result.size() >= result_position.size());

    bool check_result = true;
    for(auto const &instance :instance_resets)
    {
        // Collect parameter IDs not needed for an instance
        // Check for present write values
        std::set<parameter_instance_id> unnecessary_parameters;
        for(size_t i = 0; i < ids_to_write.size();  ++i)
        {
            auto const &request = ids_to_write[i];
            if(request.param_id == instance.first->id)
            {
                unnecessary_parameters.clear();
                auto instantiations = request.value->get_instantiations();
                for(auto const &class_def: model.classes)
                {
                    if(std::find(class_def->resolved_includes.begin(),
                                 class_def->resolved_includes.end(),
                                 instance.second->name) != class_def->resolved_includes.end())
                    {
                        for(auto const &instantiation: instantiations)
                        {
                            bool unused_class = true;
                            for(auto &class_name: instantiation.classes)
                            {
                                auto const instance_class_def = model.find_class_definition(class_name);
                                if(instance_class_def != nullptr)
                                {
                                    unused_class =     unused_class
                                                   && (instance_class_def->name != class_def->name) 
                                                   && (std::find(instance_class_def->resolved_includes.begin(),
                                                                 instance_class_def->resolved_includes.end(),
                                                                 class_def->name) == instance_class_def->resolved_includes.end());
                                }
                            }
                            if(unused_class)
                            {
                                for(auto &param_def: class_def->parameter_definitions)
                                {
                                    unnecessary_parameters.emplace(parameter_instance_id(param_def->id, instantiation.id, instance.first->id.device));
                                }
                            }
                        }
                    }
                }
                // Do not break loop because last reset wins (in case of multiple reset requests)
            }
        }
        for(size_t i = 0; i < ids_to_write.size(); ++i)
        {
            auto const &request = ids_to_write[i];
            if(unnecessary_parameters.count(request.param_id) > 0)
            {
                auto idx = result_position[i];
                result[idx].status = status_codes::not_existing_for_instance; // parasoft-suppress CERT_CPP-CTR50-a-2 "result position 'idx' was determined internally and will not be out of bounds of result vector"
                providers_with_invalid_set.insert(provider);
                result[idx].message = "This parameter value does not exist for target class instance."; // parasoft-suppress CERT_CPP-CTR50-a-2 "result position 'idx' was determined internally and will not be out of bounds of result vector"
                check_result = false;
            }
        }
    }
    return check_result;
}

std::shared_ptr<class_definition> get_class_def_for_instantiation(parameter_definition                           const &param_def,
                                                                  std::vector<std::shared_ptr<class_definition>> const &classes)
{
    if(    (param_def.value_type == parameter_value_types::instantiations)
        && (param_def.value_rank == parameter_value_rank::scalar))
    {
        for(auto const &class_def: classes)
        {
            if(class_def->base_path == param_def.path)
            {
                return class_def;
            }
        }
    }
    return nullptr;
}


void append_included_features(std::set<std::string> &features, device_model &model) {
    auto original_feature = features;
    for (auto const &feature_name : original_feature)
    {
        auto feature = model.find_feature_definition(feature_name);
        if (feature != nullptr)
        {
            // append all features included by this feature directly or indirectly
            for (auto const &included_feature : feature->resolved_includes)
            {
                features.insert(included_feature);
            }
        }
    }
}

}}}
