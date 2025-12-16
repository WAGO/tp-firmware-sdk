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
#include "parameter_instance_collection.hpp"
#include "check_parameter_value.hpp"
#include "utils/string_util.hpp"

#include <wc/log.h>

#include <algorithm>
#include <regex>

using namespace std;

namespace wago {
namespace wdx {

namespace {
std::regex const path_instance_number_pattern(".*\\" + std::string(1, path::DELIMITER) + "([0-9]+)\\" + std::string(1, path::DELIMITER) + ".*"); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "bad_alloc exception is not thrown on Linux, regex constructor does only throw if pattern is invalid but the pattern here is build from constant values, regex destructor is not known to throw"
}
        
parameter_instance_collection::parameter_instance_collection()
= default;

parameter_instance_collection::~parameter_instance_collection() noexcept
= default;

void parameter_instance_collection::add_instances(device_id                                       device,
                                                  instance_id_t                                   instance_id,
                                                  vector<shared_ptr<parameter_definition>> const &definitions,
                                                  vector<value_declaration>                const &values,
                                                  bool                                            force_status_unavailable_if_not_provided)
{
    std::vector<unique_ptr<parameter_instance>> prepared_parameter_instances;
    prepared_parameter_instances.reserve(definitions.size());

    bool instance_ok = true;
    for (const auto& definition : definitions)
    {
        auto id = parameter_instance_id{definition->id, instance_id, device};
        auto *existing = get_instance(id);
        if(existing != nullptr)
        {
            if(id.instance_id != DYNAMIC_PLACEHOLDER_INSTANCE_ID)
            {
                wc_log(log_level_t::warning, "Parameter instance " + to_short_description(id) + " already exists, ignoring");
            }
            continue; // ignore duplicates
        }

        auto instance = std::make_unique<parameter_instance>(id, definition.get());

        auto found_val = definition->overrideables.default_value;
        value_declaration val;
        if (find_value_declaration(values, id, val))
        {
            if(val.value != nullptr)
            {
                auto status = check_parameter_value(val.value, instance->definition);
                if(has_error(status))
                {
                    wc_log(log_level_t::warning, "Given " + instance->definition->path + " did not match model definition: " + status_code_strings[(int)status]);
                }
                else
                {
                    found_val = val.value;
                }
            }
            instance->status_unavailable_if_not_provided = val.status_unavailable_if_not_provided;
        }
        else if((definition->instance_key) && (id.instance_id != DYNAMIC_PLACEHOLDER_INSTANCE_ID))
        {
            instance_ok = false;
            wc_log(log_level_t::error, "No value for instance_key was found for static class '" + definition->class_def.lock()->base_path + "' in parameter_definition '" + definition->path + "' (id=" + std::to_string(definition->id) + "). Loading class instance was refused.");
            break;
        }
        if(force_status_unavailable_if_not_provided)
        {
            instance->status_unavailable_if_not_provided = true;
        }
        if(!definition->writeable || definition->overrideables.inactive)
        {
            instance->fixed_value = found_val;
        }
        prepared_parameter_instances.push_back(std::move(instance));
    }
    if(instance_ok)
    {
        m_parameter_instances.reserve(m_parameter_instances.size() + definitions.size());
        m_parameter_instance_ptrs.reserve(m_parameter_instance_ptrs.size() + definitions.size());

        for(size_t i = 0; i < prepared_parameter_instances.size(); ++i)
        {
            // TODO: insert sorted by id/instanceId, so we can later do binary search
            m_parameter_instances.push_back(std::move(prepared_parameter_instances.at(i)));
            auto * instance_ptr = m_parameter_instances.back().get();
            m_parameter_instance_ptrs.push_back(instance_ptr);
            m_paths.add_value(build_internal_path(instance_ptr), instance_ptr);
        }
    }
}

void parameter_instance_collection::add_class_instance(device_id                   device,
                                                       path                        base_path,
                                                       parameter_definition const &definition,
                                                       value_declaration    const &force_value_decl)
{
    auto id = parameter_instance_id{definition.id, 0, device};

    // NOTE: Since this method is called multiple times with instantiation pseudo-parameters of non-dynamic classes (all with base_id == 0),
    // these will lead to duplicate ids. That's ok because we need them in here for get_all_parameters.
    // Addressing instantiation parameter of non-dynamic classes without base_id is not supported anyway, so no problem should occur from this.
    if(get_instance(id) == nullptr) // This relies on the fact that it will return nullptr for id 0-0 
    {
        auto instance = std::make_unique<parameter_instance>(id, &definition);
        auto * instance_ptr = instance.get();
        instance->fixed_value = force_value_decl.value;
        instance->status_unavailable_if_not_provided = force_value_decl.status_unavailable_if_not_provided;

        m_parameter_instances.push_back(std::move(instance));  // TODO: insert sorted by id/instanceId, so we can later do binary search
        m_parameter_instance_ptrs.push_back(instance_ptr);
        m_paths.add_value(std::move(path(std::move(base_path))), instance_ptr);
    }
}

parameter_instance* parameter_instance_collection::get_instance(parameter_instance_id id)
{
    // TODO: use binary search instead

    // we might have instances in here with id 0-0, but those should only be the instantiation parameters of classes that have no base_id. They should not be addressable numerically, only by path.
    if(id.id == 0 && id.instance_id == 0)
    {
        return nullptr;
    }
    for (auto& inst : this->m_parameter_instances)
    {
        if(inst->id.id == id.id)
        {
            if(inst->id.instance_id == id.instance_id)
            {
                return inst.get();
            }
            else if(id.instance_id > 0 && inst->id.instance_id == DYNAMIC_PLACEHOLDER_INSTANCE_ID) // special case for placeholder parameter_instances of dynamic classes
            {
                return get_dynamic_instance(id, inst.get());
            }
        }
    }
    return nullptr;
}

parameter_instance* parameter_instance_collection::get_instance(path const &parameter_path)
{
    auto *inst = this->m_paths.get_value(parameter_path);
    if(inst == nullptr)
    {
        return nullptr;
    }
    if(inst->id.instance_id == DYNAMIC_PLACEHOLDER_INSTANCE_ID) // special case for placeholder parameter_instances of dynamic classes
    {
        return get_dynamic_instance(parameter_path, inst);
    }
    return inst;
}

parameter_instance* parameter_instance_collection::get_dynamic_instance(parameter_instance_id id, parameter_instance* placeholder_instance)
{
    for (auto& inst : this->m_dynamic_parameter_instances)
    {
        if(inst->id.id == id.id)
        {
            if(inst->id.instance_id == id.instance_id)
            {
                inst->provider = placeholder_instance->provider;
                return inst.get();
            }
        }
    }
    // create an instance on the fly with the correct instance_id
    auto instance = std::make_unique<parameter_instance>(*placeholder_instance);
    instance->id.instance_id = id.instance_id;
    // Since we have to store the new instance somewhere, just insert it in our instances vector
    // This will leave instances in the vector that might not be correct later (e.g. when a class instance is removed)
    // but should be no problem since the parameter_provider handling the requests has to handle unknown instance-ids for dynamic classes anyway
    auto * instance_ptr = instance.get();
    m_dynamic_parameter_instances.push_back(std::move(instance));
    return instance_ptr;
}

parameter_instance* parameter_instance_collection::get_dynamic_instance(path const &parameter_path, parameter_instance* placeholder_instance)
{
    auto *inst = this->m_dynamic_paths.get_value(parameter_path);
    if(inst != nullptr)
    {
        inst->provider = placeholder_instance->provider;
        return inst;
    }

    std::smatch m;
    if (!regex_match(parameter_path.m_path, m, path_instance_number_pattern))
    {
        return nullptr;
    }
    string s = m.str(1);
    int inst_id = 0;
    if(!convert_to_int(s, inst_id))
    {
        return nullptr;
    }
    // create an instance on the fly with the correct instance_id
    auto instance = std::make_unique<parameter_instance>(*placeholder_instance);
    instance->id.instance_id = inst_id;
    // Since we have to store the new instance somewhere, just insert it in our instances vector
    // This will leave instances in the vector that might not be correct later (e.g. when a class instance is removed)
    // but should be no problem since the parameter_provider handling the requests has to handle unknown instance-ids for dynamic classes anyway
    auto * instance_ptr = instance.get();
    m_dynamic_parameter_instances.push_back(std::move(instance));
    return instance_ptr;
}

vector<parameter_instance*> const & parameter_instance_collection::get_all()
{
    return m_parameter_instance_ptrs;
}

std::vector<parameter_instance*> parameter_instance_collection::get_all_under_subpath(path subpath)
{
    std::vector<parameter_instance*> visited_insts;
    auto *n = m_paths.get_node_under_path(subpath, visited_insts, true);
    if(n == nullptr)
    {
        return {};
    }
    std::vector<parameter_instance*> result;
    m_paths.gather_values(n, result);
    for(auto& visited : visited_insts)
    {
        if(visited->definition->value_type == parameter_value_types::instantiations && !visited->fixed_value)
        {
            result.push_back(visited);
        }
    }
    return result;
}

path parameter_instance_collection::build_parameter_instance_path(parameter_instance* inst)
{
    auto base_path = inst->definition->class_def.expired() ? path("") : path(inst->definition->class_def.lock()->base_path);
    auto iid = inst->id.instance_id;
    return path(base_path, path(iid > 0 ? std::to_string(static_cast<unsigned>(iid)) : ""), path(inst->definition->path));
}

path parameter_instance_collection::build_internal_path(parameter_instance* inst)
{
    /**
        For parameters of class_instantiations, the paths are composed differently in the search tree than in the requests.
        Request path: ClassDefinition.basePath + instanceID + ParameterDefinition.path
        Search path: ClassDefinition.basePath + ParameterDefinition.path + instanceID
        This is done to minimize memory usage by reusing the paths of the ParameterDefinitions as much as possible.
        NOTE: Changes to this policy have to be reflected in PathTreeNode.hpp!
    */

    auto base_path = inst->definition->class_def.expired() ? path("") : path(inst->definition->class_def.lock()->base_path);
    auto iid = inst->id.instance_id;
    return path(base_path, path(inst->definition->path), path(iid > 0 ? std::to_string(static_cast<unsigned>(iid)) : ""));
}

bool find_value_declaration(const std::vector<value_declaration>& values, parameter_instance_id id, value_declaration& out)
{
    auto val = find_if(values.begin(), values.end(), [id](auto& v)
    {
        return v.id.equals_ignoring_device(id);
    });
    if (val != values.end())
    {
        out = *val;
        return true;
    }
    return false;
}

}
}
