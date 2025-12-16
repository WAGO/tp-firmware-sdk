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
#include "device_description_loader.hpp"
#include "wago/wdx/status_codes.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "wdmm/model_resolver.hpp"
#include "wago/wdx/parameter_value.hpp"
#include "utils/algo_helpers.hpp"
#include "loader_common.hpp"
#include "wdmm/parameter_definition_override.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include "instances/device.hpp"

#include <wc/log.h>

#include <regex>
#include <sstream>
#include <set>

using namespace std;
using json = nlohmann::json;

namespace wago {
namespace wdx {

bool device_description_loader::parse_device_description(json &node)
{
    try
    {
        if (node.contains("WDMMVersion"))
        {
            m_description.wdmm_version = node.at("WDMMVersion").get<string>();
        }
        else
        {
            throw parameter_exception(status_codes::wdmm_version_not_supported, "\"WDMMVersion\" not found in device description");
        }
        if (node.contains("ModelReference"))
        {
            m_description.model_name = node.at("ModelReference").get<string>();
        }
        else
        {
            throw parameter_exception(status_codes::wdmm_version_not_supported, "\"ModelReference\" not found in device description");
        }
        device_model::check_model_version(m_description.wdmm_version);

        if (node.contains("Features"))
        {
            m_description.features = node.at("Features").get<vector<string>>();
        }

        if (node.contains("Instantiations"))
        {
            for (auto &inst_node : node.at("Instantiations"))
            {
                vector<string> classes;
                if (inst_node.contains("Class"))
                {
                    classes.push_back(inst_node.at("Class").get<string>());
                }
                if (inst_node.contains("AdditionalClasses"))
                {
                    for(auto const &cls : inst_node.at("AdditionalClasses"))
                    {
                        classes.push_back(cls.get<string>());
                    }
                }
                if (inst_node.contains("Instances"))
                {
                    for (auto &inst : inst_node.at("Instances"))
                    {
                        if(inst.contains("ID"))
                        {
                            class_instantiation_internal instance;
                            instance.id = inst.at("ID");
                            instance.classes = classes;
                            if(inst.contains("Overrides"))
                            {
                                for(auto &inst_ovr_node : inst.at("Overrides"))
                                {
                                    if(inst_ovr_node.contains("ID"))
                                    {
                                        auto inst_ovr = std::make_shared<parameter_definition_override>();
                                        inst_ovr->owner_name = inst.at("ID").dump(); // Instance ID as string
                                        inst_ovr->definition_id = inst_ovr_node.at("ID").get<parameter_id_t>();
                                        inst_ovr->attributes = parse_overrideable_attributes(inst_ovr_node);
                                        instance.overrides.push_back(std::move(inst_ovr));
                                    }
                                }
                            }
                            m_description.instantiations.push_back(std::move(instance));
                        }
                    }
                }
            }
        }

        if (node.contains("Overrides"))
        {
            for(auto const &ovr_node : node.at("Overrides"))
            {
                if(ovr_node.contains("ID"))
                {
                    auto ovr = std::make_shared<parameter_definition_override>();
                    // should be something unique as soon as mustiple WDDs per device is supported
                    // order number + fw version?
                    ovr->owner_name = "WDD";
                    ovr->definition_id = ovr_node.at("ID").get<parameter_id_t>();
                    ovr->attributes = parse_overrideable_attributes(ovr_node);
                    m_description.overrides.push_back(ovr);
                }
            }
        }

        return parse_device_description_values(node);
    }
    catch (exception& ex)
    {
        wc_log(log_level_t::error, "Could not parse device description: " + std::string(ex.what()));
        return false;
    }
    catch (...)
    {
        wc_log(log_level_t::error, "Could not parse device description: unknown exception");
        return false;
    }
}

bool device_description_loader::parse_device_description_values(json &node)
{
    try
    {
        if(node.contains("ParameterValues"))
        {
            for (auto &val_node : node.at("ParameterValues"))
            {
                value_declaration valDecl{parameter_instance_id(0), nullptr};
                if (parse_value_declaration(val_node, valDecl))
                {
                    m_description.values.push_back(valDecl);
                }
            }
        }
        if (node.contains("Instantiations"))
        {
            for (auto &inst_node : node.at("Instantiations"))
            {
                if(inst_node.contains("Instances"))
                {
                    for (auto &inst : inst_node.at("Instances"))
                    {
                        if(inst.contains("ParameterValues"))
                        {
                            for (auto &val_node : inst.at("ParameterValues"))
                            {
                                value_declaration val_decl{parameter_instance_id(0), nullptr};
                                if (parse_value_declaration(val_node, val_decl))
                                {
                                    if(inst.contains("ID"))
                                    {
                                        val_decl.id.instance_id = inst.at("ID");
                                        m_description.values.push_back(val_decl);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return true;
    }
    catch (exception& ex)
    {
        wc_log(log_level_t::error, "Exception loading device description: " + std::string(ex.what()));
        return false;
    }
    catch (...)
    {
        wc_log(log_level_t::error, "Unknown exception loading device description.");
        return false;
    }
}

bool device_description_loader::parse_value_declaration(json &node, value_declaration &val_decl) const
{
    parameter_id_t definition_id;
    instance_id_t instance_id = 0;
    if(node.contains("ID"))
    {
        auto &id_node = node.at("ID");
        if (id_node.is_number())
        {
            definition_id = id_node.get<parameter_id_t>();
        }
        else
        {
            definition_id = id_node.at("ID").get<parameter_id_t>();
            instance_id = id_node.contains("InstanceID") ? id_node.at("InstanceID").get<uint16_t>() : 0;
        }
    }
    else
    {
        return false;
    }
    parameter_instance_id id{definition_id, instance_id};

    if (node.contains("Value"))
    {
        auto &val_json = node.at("Value");
        val_decl.id = id;
        val_decl.value = parameter_value::create_with_unknown_type(val_json.dump());
        return true;
    }
    else if (node.contains("StatusUnavailableIfNotProvided"))
    {
        val_decl.id = id;
        val_decl.status_unavailable_if_not_provided = node.at("StatusUnavailableIfNotProvided").get<bool>();
        return true;
    }
    else
    {
        return false;
    }
}

void device_description_loader::instantiate(parameter_definition_owner      &definition_owner,
                                            instance_id_t                    instance_id,
                                            vector<value_declaration> const &values,
                                            bool                             force_status_unavailable_if_not_provided)
{
    if (!definition_owner.is_resolved)
    {
        throw parameter_exception(status_codes::internal_error, "Can only instantiate resolved Definition.");
    }
    m_device.parameter_instances.add_instances(m_device.id, instance_id, definition_owner.resolved_parameter_definitions, values, force_status_unavailable_if_not_provided);
}

device_description_loader::device_description_loader(device_model &device_model_, device &device_)
    : m_device(device_)
    , m_device_model(device_model_)
{ }

void device_description_loader::load_description(std::string &wdd_content)
{
    if (wdd_content.empty())
    {
        throw parameter_exception(status_codes::device_description_inaccessible);
    }
    auto device_description_node = json::parse(wdd_content); // TODO: switch to SAX-style parsing for less memory consumption

    bool success = parse_device_description(device_description_node);
    if (!success)
    {
        throw parameter_exception(status_codes::device_description_parse_error);
    }
}

void device_description_loader::process_description()
{
    m_model_incomplete = false;

    for(auto &val : m_description.values)
    {
        auto d = m_device_model.find_definition(val.id.id);
        if(d)
        {
            if(val.value)
            {
                val.value->set_type_internal(d->value_type, d->value_rank);
            }
        }
        else
        {
            m_model_incomplete = true;
        }
    }

    feature_definition added;
    added.includes = m_description.features;

    // only use the features that are known in the model
    {
        auto &inc = added.includes;
        inc.erase(std::remove_if(inc.begin(), inc.end(), [this](auto &f) {
            return !m_device_model.find_feature_definition(f);
        }), inc.end());
    }
    if(added.includes.size() < m_description.features.size())
    {
        m_model_incomplete = true;
    }
    model_resolver::resolve_includes(added, m_device_model.features);

    append(added.resolved_overrides, m_description.overrides); // add device description specific overrides to the overrides defined by the model
    apply_overrides(&added);
    model_resolver::prepare_attributes(added.resolved_parameter_definitions, m_device_model);
    if(contains<shared_ptr<parameter_definition>>(added.resolved_parameter_definitions, [](auto &def) { return def->is_beta; }))
    {
        m_device.has_beta_parameters = true;
    }
    if(contains<shared_ptr<parameter_definition>>(added.resolved_parameter_definitions, [](auto &def) { return def->is_deprecated; }))
    {
        m_device.has_deprecated_parameters = true;
    }
    // will ignore duplicates
    instantiate(added, 0, m_description.values);

    // append uniquely into m_device.collected_features
    append_unique<string, string>(m_device.collected_features.resolved_includes, added.resolved_includes, [](auto &x) { return x; }, [](auto &x) { wc_log(log_level_t::warning, "Feature '" + x + "' already declared for device, skipping"); });
    append_unique<string, string>(m_device.collected_features.classes, added.classes, [](auto &x) { return x; });
    append_unique<shared_ptr<parameter_definition>, parameter_id_t>(m_device.collected_features.resolved_parameter_definitions, added.resolved_parameter_definitions, [](auto &x) { return x->id; });
    append_unique<shared_ptr<parameter_definition_override const>, parameter_id_t>(m_device.collected_features.resolved_overrides, added.resolved_overrides, [](auto &x) { return x->definition_id; });

    for (auto &cls_name : m_device.collected_features.classes)
    {
        auto cls = m_device_model.find_class_definition(cls_name);
        if(cls == nullptr)
        {
            continue;
        }
        if(cls->is_dynamic) {
            append(cls->resolved_overrides, m_description.overrides);
            apply_overrides(cls.get());
        }
    }
    
    set<name_t> overridden_dynamic_base_paths;
    for (auto &class_instance : m_description.instantiations)
    {
        auto new_instance = class_instantiation_internal(class_instance.id, class_instance.classes);
        model_resolver::prepare_class_instantiation(new_instance, m_device_model, true);
        auto &cls = new_instance.collected_classes;
        if(cls.includes.size() < class_instance.classes.size())
        {
            m_model_incomplete = true;
        }
        append(cls.resolved_overrides, m_description.overrides); // add device description specific overrides to the overrides defined by the model

        if(cls.is_dynamic)
        {
            overridden_dynamic_base_paths.insert(cls.base_path);
        }
        apply_overrides(&cls);

        // If overrides present this instance is its own definition owner
        if(!class_instance.overrides.empty())
        {
            // Create instance specific definitions
            wc_log(log_level_t::info, "Apply instance specific attributes for "
                                      "" + cls.base_path + "/" + std::to_string(class_instance.id) + " "
                                      "(" + class_instance.classes.at(0) + ")");
            append(cls.resolved_overrides, class_instance.overrides);
            apply_overrides(&cls, true);
        }
        model_resolver::prepare_attributes(cls.resolved_parameter_definitions, m_device_model);
        instantiate(cls, new_instance.id, m_description.values); // will ignore duplicates

        auto existing = std::find_if(m_device.class_instantiations.begin(), m_device.class_instantiations.end(), [&](auto &x) {
            return new_instance.id == x.id && new_instance.collected_classes.base_path == x.collected_classes.base_path;
        });
        if(existing != m_device.class_instantiations.end())
        {
            append_unique<string, string>(existing->collected_classes.resolved_includes, cls.resolved_includes, [](auto &x) { return x; });
            existing->classes = existing->collected_classes.resolved_includes;
            append_unique<shared_ptr<parameter_definition>, parameter_id_t>(existing->collected_classes.resolved_parameter_definitions, cls.resolved_parameter_definitions, [](auto &x) { return x->id; });
            append_unique<shared_ptr<parameter_definition_override const>, parameter_id_t>(existing->collected_classes.resolved_overrides, cls.resolved_overrides, [](auto &x) { return x->definition_id; });
        }
        else
        {
            m_device.class_instantiations.push_back(new_instance);
        }
    }

    set<name_t> base_paths;
    for (auto &cls_name : m_device.collected_features.classes)
    {
        auto cls = m_device_model.find_class_definition(cls_name);
        if(cls == nullptr)
        {
            continue;
        }
        auto base_path = cls->base_path;
        auto base_path_done = base_paths.find(base_path) != base_paths.end();
        bool dynamic = cls->is_dynamic && overridden_dynamic_base_paths.find(base_path) == overridden_dynamic_base_paths.end();
        
        if(!dynamic)
        {
            if(base_path_done)
            {
                continue;
            }
            vector<class_instantiation> instantiations;
            for(auto &inst : m_device.class_instantiations)
            {
                if(inst.collected_classes.base_path == base_path)
                {
                    instantiations.push_back(inst);
                }
            }
            m_device.parameter_instances.add_class_instance(m_device.id, base_path, *cls->instantiations_parameter, {parameter_instance_id(), parameter_value::create_instantiations(std::move(instantiations)), false});
        }
        else
        {
            value_declaration val;
            find_value_declaration(m_description.values, parameter_instance_id(cls->instantiations_parameter->id,0), val);
            // instantiate placeholder parameter_instances for each dynamic class, with the fake instance_id
            instantiate(*cls, DYNAMIC_PLACEHOLDER_INSTANCE_ID, {}, val.status_unavailable_if_not_provided);
            if(base_path_done)
            {
                continue;
            }
            m_device.parameter_instances.add_class_instance(m_device.id, base_path, *cls->instantiations_parameter, {parameter_instance_id(), nullptr, val.status_unavailable_if_not_provided});
        }
        base_paths.insert(base_path);
    }

    // TODO: local ParameterDefinitions
}

void device_description_loader::apply_overrides(parameter_definition_owner* base, bool warn)
{
    for(auto &override : base->resolved_overrides)
    {
        if(apply_override(*override, base->resolved_parameter_definitions))
        {
            wc_log(log_level_t::info, "Attribute overriden for ID " + std::to_string(override->definition_id));
        }
        else if(warn)
        {
            wc_log(log_level_t::warning, "Failed to override attribute for ID " + std::to_string(override->definition_id));
        }
    }
}

bool device_description_loader::apply_override(parameter_definition_override const& override, std::vector<shared_ptr<parameter_definition>>& definitions)
{
    size_t idx = 0;
    for(auto &def : definitions)
    {
        if(def->id == override.definition_id)
        {
            std::shared_ptr<parameter_definition> new_def;
            if(def->value_type == parameter_value_types::method)
            {
                new_def = static_pointer_cast<parameter_definition>(make_shared<method_definition>(*(dynamic_pointer_cast<method_definition>(def))));
            }
            else
            {
                new_def = make_shared<parameter_definition>(*def);
            }
            new_def->overrideables.override_with(override.attributes);
            definitions.at(idx) = new_def;
            return true;
        }
        idx++;
    }
    return false;
}

void device_description_loader::load(string &wdd_content)
{
    load_description(wdd_content);
    process_description();
}

void device_description_loader::add_description(device_description const &extension_description)
{
    m_description = extension_description;
    process_description();
}

bool device_description_loader::was_model_incomplete() const
{
    return m_model_incomplete;
}

void device_description_loader::retry_with_updated_model()
{
    if(m_model_incomplete)
    {
        process_description();
    }
}

}
}
