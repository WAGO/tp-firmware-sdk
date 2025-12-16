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
#include "wago/wdx/parameter_exception.hpp"
#include "model_resolver.hpp"
#include "utils/algo_helpers.hpp"
#include "instances/check_parameter_value.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include "class_instantiation_internal.hpp"
#include "parameter_definition_override.hpp"

#include <wc/log.h>

#include <algorithm>

using namespace std;

namespace wago {
namespace wdx {

void model_resolver::resolve_includes(device_model& model)
{
    // link features and classes after both have been added to the model
    for (auto& feature : model.features)
    {
         if (feature->is_resolved)
        {
            continue;
        }
        for (auto& class_name : feature->classes)
        {
           
            auto class_def = model.find_class_definition(class_name);
            if (class_def != nullptr)
            {
                auto existing_feature_def = class_def->feature_def.lock();
                if (   (existing_feature_def != nullptr) 
                    && (existing_feature_def->name != feature->name))
                {
                    wc_log(log_level_t::warning, "Class '" + class_name + "' already referenced by feature '" + existing_feature_def->name + "', overwriting with feature '" + feature->name + "'.");
                }
                class_def->feature_def = feature;
                class_def->is_beta = feature->is_beta; // FIXME: This is not automatically true
            }
            else
            {
                wc_log(log_level_t::warning, "Class '" + class_name + "' referenced by feature '" + feature->name + "' not found in model.");
            }
        }
    }

    for (auto& cls : model.classes)
    {
        resolve_includes<class_definition>(*cls, model.classes);
    }

    for (auto& feature : model.features)
    {
        for(auto& c : feature->classes)
        {
            auto cls = model.find_class_definition(c);
            if(!cls)
            {
                wc_log(log_level_t::warning, "Unknown Class '" + c + "' in feature '" + feature->name + "'");
                continue;
            }
            cls->instantiations_parameter->feature_def = cls->feature_def;
        }
        resolve_includes<feature_definition>(*feature, model.features);
    }
}

template <typename T>
void model_resolver::resolve_includes(T& base, vector<shared_ptr<T>>& others)
{
    if (base.is_resolved)
    {
        return;
    }
    base.resolved_parameter_definitions = base.parameter_definitions;
    sort(base.resolved_parameter_definitions.begin(), base.resolved_parameter_definitions.end()); // sort is only needed so that insert_unique will work

    // TODO: Exit necessary?
    if (base.includes.empty())
    {
        base.is_resolved = true;
        return;
    }

    // TODO: Use std::set to ensure unique and sorted items in container
    base.resolved_includes = base.includes;
    sort(base.resolved_includes.begin(), base.resolved_includes.end());

    try
    {
        // collect in reverse order for later loop
        std::vector<shared_ptr<T>> reverse_includes;
        reverse_includes.reserve(base.includes.size());
        for (auto& includeName : base.includes)
        {
            auto inclIt = find_if(others.begin(), others.end(), [includeName](shared_ptr<T> pdo)
            {
                return pdo->name == includeName;
            });
            if (inclIt == others.end())
            {
                wc_log(log_level_t::error, "Unknown include " + includeName);
                continue;
            }

            shared_ptr<T> include = *inclIt;
            reverse_includes.insert(reverse_includes.begin(), include);

            resolve_includes(*include, others);

            insert_unique(base.resolved_includes, include->resolved_includes);
            insert_unique(base.resolved_parameter_definitions, include->resolved_parameter_definitions);
            takeover_specific_properties(base, *include);
        }
        // iterate over reverse_includes in reverse order
        // for overrides
        for (auto& include : reverse_includes)
        {
            append_if<std::shared_ptr<parameter_definition_override const>>(base.resolved_overrides, include->resolved_overrides, [&base] (auto const &override_candidate) {
                return false == contains<std::shared_ptr<parameter_definition_override const>>(base.resolved_overrides, [&override_candidate](auto const &override) {
                    return (override->definition_id == override_candidate->definition_id)
                        && (override->owner_name == override_candidate->owner_name);
                });
            });             
        }
        append(base.resolved_overrides, base.overrides);
        base.is_resolved = true;
    }
    catch(const parameter_exception& e)
    {
        wc_log(log_level_t::error, "Exception caught: " + std::string(e.what()));
        base.is_resolved = false;
    }
}

template <>
void model_resolver::takeover_specific_properties<class_definition>(class_definition& base, class_definition& include)
{
    if (base.base_path.empty())
    {
        base.base_path = include.base_path;
    }
    else if (!include.base_path.empty() && base.base_path != include.base_path)
    {
        throw parameter_exception(status_codes::ambiguous_base_path,
                                 "Class '" + base.name + "' defines other basePath that its include '" + include.name +
                                 "'.");
    }
    if(include.is_dynamic)
    {
        base.is_dynamic = true;
    }
    if(include.is_writable)
    {
        base.is_writable = true;
    }
    if(include.base_id > 0)
    {
        base.base_id = include.base_id;
    }
    base.instantiations_parameter = include.instantiations_parameter;
}

template <>
void model_resolver::takeover_specific_properties<feature_definition>(feature_definition& base,
                                                                      feature_definition& include)
{
    if(include.is_beta && !base.is_beta && !base.name.empty()) // base.name.empty() is only for WDD collected_features
    {
        wc_log(log_level_t::warning, "Should not include beta feature '" + include.name + "' without '" + base.name + "' being beta as well");
    }
    if(include.is_deprecated && !base.is_deprecated && !base.name.empty()) // base.name.empty() is only for WDD collected_features
    {
        wc_log(log_level_t::warning, "Should not include deprecated feature '" + include.name + "' without '" + base.name + "' being deprecated as well");
    }
    sort(base.classes.begin(), base.classes.end()); // sort is only needed so that insert_unique will work
    insert_unique(base.classes, include.classes);
}

void model_resolver::prepare_attributes(std::vector<shared_ptr<parameter_definition>>& definitions, device_model& model)
{
    for(auto& pd : definitions)
    {
        if(pd->feature_def.expired()) // this can only happen if class_def is set instead
        {
            if(!pd->class_def.expired())
            {
                pd->feature_def = pd->class_def.lock()->feature_def;
            }
            else // edge case for robustness: if the class was not added to any feature
            {
                wc_log(log_level_t::warning, "Detected instantiations parameter '" + pd->path + "' of class that does not belong to any feature.");
            }
        }
        if(!pd->class_def.expired())
        {
            pd->is_beta       = pd->class_def.lock()->is_beta;
            pd->is_deprecated = pd->class_def.lock()->is_deprecated;
        }
        else if(!pd->feature_def.expired())
        {
            pd->is_beta       = pd->feature_def.lock()->is_beta;
            pd->is_deprecated = pd->feature_def.lock()->is_deprecated;
        }
        else
        {
            wc_log(log_level_t::warning, "Parameter definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") belongs to no feature");
        }
        if(pd->value_type == parameter_value_types::unknown)
        {
            wc_log(log_level_t::warning, "Parameter definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") has unknown value_type. Parameter will not be useable.");
        }
        else if(pd->value_type == parameter_value_types::enum_member)
        {
            if(!pd->enum_name.empty())
            {
                pd->enum_def = model.find_enum_definition(pd->enum_name);
                if(pd->enum_def.expired())
                {
                    wc_log(log_level_t::warning, "Unknown Enum '" + pd->enum_name + "' for parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + "). Parameter will not be usable.");
                }
            }
            else
            {
                wc_log(log_level_t::warning, "Parameter definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") with value_type enum has no source enum. Parameter will not be usable.");
            }
        }
        else if(pd->value_type == parameter_value_types::instance_ref)
        {
            if(!pd->ref_classes.empty() && !pd->ref_classes.at(0).empty())
            {
                pd->ref_classes_def = {model.find_class_definition(pd->ref_classes.at(0))};
                if(pd->ref_classes_def.at(0).expired())
                {
                    wc_log(log_level_t::warning, "Unknown RefClass '" + pd->ref_classes.at(0) + "' for parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + "). Parameter will not be usable.");
                }
            }
            else
            {
                wc_log(log_level_t::warning, "Parameter definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") with InstanceRef has no RefClass. Parameter will not be usable.");
            }
        }
        else if(pd->instance_key)
        {
            auto class_def = pd->class_def.lock();
            if(!class_def)
            {
                wc_log(log_level_t::warning, "Instance key for parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") does not belong to a class instance.");
            }
        }
        else if(pd->value_type == parameter_value_types::instance_identity_ref)
        {
            if(!pd->ref_classes.empty())
            {
                for(auto const &ref_class: pd->ref_classes)
                {
                    std::weak_ptr<class_definition> ref_class_def = model.find_class_definition(ref_class);
                    auto class_def = ref_class_def.lock();
                    if(class_def != nullptr)
                    {
                        pd->ref_classes_def.push_back(ref_class_def);
                        if(!class_def->is_writable && class_def->is_dynamic)
                        {
                            wc_log(log_level_t::warning, "Referenced Class '" + ref_class + "' for parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") is dynamic but not writeable.");
                        }
                        int instance_keys = class_def->is_instance_key ? 1 : 0;
                        auto resolved_parameter_definitions = class_def->resolved_parameter_definitions;
                        for(auto & resolved_parameter_definition: resolved_parameter_definitions)
                        {
                            if(resolved_parameter_definition->instance_key)
                            {
                                instance_keys++;
                            }
                        }
                        if(instance_keys == 0)
                        {
                            wc_log(log_level_t::warning, "Referenced Class '" + ref_class + "' for parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") has no instance_key attribute.");
                        }
                        else if(instance_keys > 1 && !class_def->is_dynamic)
                        {
                            pd->ref_classes_def.pop_back();
                            wc_log(log_level_t::error, "More than one instance_key were found for static class '" + ref_class + "' referenced by parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + "). Loading class was refused.");
                        }
                    }
                    else
                    {
                        wc_log(log_level_t::notice, "Unknown referenced Class '" + ref_class + "' for parameter_definition '" + pd->path + "' (id=" + std::to_string(pd->id) + "). Parameter will not be usable.");
                    }
                }
            }
            else
            {
                wc_log(log_level_t::warning, "Parameter definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ") with InstanceIdentityRef has no RefClasses. Parameter will not be usable.");
            }
        }
        if(pd->overrideables.default_value)
        {
            pd->overrideables.default_value->set_type_internal(pd->value_type, pd->value_rank);
            auto status = check_parameter_value(pd->overrideables.default_value, pd.get());
            if(has_error(status))
            {
                wc_log(log_level_t::warning, "Given default_value did not match parameter definition '" + pd->path + "' (id=" + std::to_string(pd->id) + ")");
            }
        }
        if(pd->value_type == parameter_value_types::method)
        {
            auto md = dynamic_pointer_cast<method_definition>(pd);
            WC_ASSERT(md.get() != nullptr);

            auto set_enum_def = [&pd, &model](method_argument_definition& d) {
                if(d.value_type == parameter_value_types::enum_member)
                {
                    if(!d.enum_name.empty())
                    {
                        d.enum_def = model.find_enum_definition(d.enum_name);
                        if(d.enum_def.expired())
                        {
                            wc_log(log_level_t::warning, "Unknown Enum '" + d.enum_name + "' for Arg of method '" + pd->path + "' (id=" + std::to_string(pd->id) + "). Method will not be usable.");
                        }
                    }
                    else
                    {
                        wc_log(log_level_t::warning, "Arg definition '" + d.name + "' of method '" + pd->path + "' (id=" + std::to_string(pd->id) + ") with value_type enum has no source enum. Method will not be usable.");
                    }
                }
                else if(d.value_type == parameter_value_types::instance_ref)
                {
                    if(!d.ref_classes.empty() && !d.ref_classes.at(0).empty())
                    {
                        d.ref_classes_def = {model.find_class_definition(d.ref_classes.at(0))};
                        if(d.ref_classes_def.at(0).expired())
                        {
                            wc_log(log_level_t::warning, "Unknown RefClass '" + d.ref_classes.at(0) + "' for Arg of method '" + pd->path + "' (id=" + std::to_string(pd->id) + "). Method will not be usable.");
                        }
                    }
                    else
                    {
                        wc_log(log_level_t::warning, "Arg definition '" + d.name + "' of method '" + pd->path + "' (id=" + std::to_string(pd->id) + ") with InstanceRef has no RefClass. Method will not be usable.");
                    }
                }
            };

            for(auto& d : md->in_args)
            {
                set_enum_def(d);
                if(d.default_value)
                {
                    auto status = check_parameter_value(d.default_value, &d);
                    if(has_error(status))
                    {
                        wc_log(log_level_t::warning, "Given default_value of in_args did not match argument definition '"+ pd->path + "' (id=" + std::to_string(pd->id) + ")");
                    }
                }
            }
            for(auto& d : md->out_args)
            {
                set_enum_def(d);
            }
        }
    }
}

void model_resolver::prepare_class_instantiation(class_instantiation_internal& inst, device_model& model, bool only_known_classes)
{
    auto& cls = inst.collected_classes;
    cls.includes = inst.classes;
    if(only_known_classes)
    {
        auto& inc = cls.includes;
        inc.erase(std::remove_if(inc.begin(), inc.end(), [model](auto& c) {
            auto found = model.find_class_definition(c);
            if(!found)
            {
                wc_log(log_level_t::warning, "Unknown instantiation classname " + c);
            }
            return !found;
        }), inc.end());
    }
    model_resolver::resolve_includes(cls, model.classes);
    inst.classes = cls.resolved_includes;
}

}
}
