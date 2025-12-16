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
#include "device_model_loader.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "wago/wdx/status_codes.hpp"
#include "wdmm/model_resolver.hpp"

#include "utils/algo_helpers.hpp"
#include "json_helpers.hpp"
#include "loader_common.hpp"
#include "wdmm/parameter_definition_override.hpp"

#include <wc/log.h>

#include <sstream>

using namespace std;
using namespace wago::wdx::wdmm;

namespace wago {
namespace wdx {
namespace wdmm {

template <typename T>
void fill_type_and_rank(T *p, json const &node)
{
    if(node.contains("Type"))
    {
        string const &type = node.at("Type").get<string>();
        if(type == "Boolean")
        {
            p->value_type = parameter_value_types::boolean;
        }
        else if(type == "String")
        {
            p->value_type = parameter_value_types::string;
        }
        else if(type == "Float32")
        {
            p->value_type = parameter_value_types::float32;
        }
        else if(type == "Float64")
        {
            p->value_type = parameter_value_types::float64;
        }
        else if(type == "Bytes")
        {
            p->value_type = parameter_value_types::bytes;
        }
        else if(type == "UInt8")
        {
            p->value_type = parameter_value_types::uint8;
        }
        else if(type == "UInt16")
        {
            p->value_type = parameter_value_types::uint16;
        }
        else if(type == "UInt32")
        {
            p->value_type = parameter_value_types::uint32;
        }
        else if(type == "UInt64")
        {
            p->value_type = parameter_value_types::uint64;
        }
        else if(type == "Int8")
        {
            p->value_type = parameter_value_types::int8;
        }
        else if(type == "Int16")
        {
            p->value_type = parameter_value_types::int16;
        }
        else if(type == "Int32")
        {
            p->value_type = parameter_value_types::int32;
        }
        else if(type == "Int64")
        {
            p->value_type = parameter_value_types::int64;
        }
        else if(type == "InstanceRef")
        {
            p->value_type = parameter_value_types::instance_ref;
        }
        else if(type == "InstanceIdentityRef")
        {
            p->value_type = parameter_value_types::instance_identity_ref;
        }
        else if(type == "Enum")
        {
            p->value_type = parameter_value_types::enum_member;
        }
        else if(type == "Method")
        {
            p->value_type = parameter_value_types::method;
        }
        else if(type == "FileID")
        {
            p->value_type = parameter_value_types::file_id;
        }
        else if(type == "IPv4Address")
        {
            p->value_type = parameter_value_types::ipv4address;
        }
        else if(type == "Instantiations")
        {
            throw parameter_exception(status_codes::device_model_parse_error, "parameter_value_types::instantiations should not be used directly in definitions.");
        }
        else
        {
            p->value_type = parameter_value_types::unknown;
        }
    }
    if(node.contains("Rank"))
    {
        auto const &r = node.at("Rank");
        if(r.is_string() && r == "Array")
        {
            p->value_rank = parameter_value_rank::array;
        }
    }
}

method_argument_definition device_model_loader::parse_method_argument_definition(json const &node)
{
    method_argument_definition m;
    if(node.contains("Name"))
    {
        m.name = node.at("Name").get<string>();
    }
    fill_type_and_rank<method_argument_definition>(&m, node);
    if(node.contains("DefaultValue"))
    {
        m.default_value = parameter_value::create_with_unknown_type(node.at("DefaultValue").dump());
    }
    if(node.contains("Enum"))
    {
        m.enum_name = node.at("Enum").get<string>();
    }
    if(node.contains("RefClass"))
    {
        m.ref_classes = {node.at("RefClass").get<string>()};
    }
    return m;
}

shared_ptr<parameter_definition> device_model_loader::create_parameter_definition(
                                    json                                const &node,
                                    std::shared_ptr<feature_definition> const &owner_feature,
                                    std::shared_ptr<class_definition>   const &owner_class)
{
    auto p = make_shared<parameter_definition>();

    if(node.contains("Path"))
    {
        p->path = node.at("Path").get<string>();
    }
    if(node.contains("ID"))
    {
        p->id = node.at("ID");
    }
    p->feature_def = owner_feature;
    p->class_def = owner_class;

    fill_type_and_rank<parameter_definition>(p.get(), node);

    if(p->value_type == parameter_value_types::method)
    {
        auto md = make_shared<method_definition>();
        md->id = p->id;
        md->path = p->path;
        md->feature_def = p->feature_def;
        md->class_def = p->class_def;
        md->value_type = p->value_type;
        if(node.contains("InArgs"))
        {
            md->in_args  = json_to_vector<method_argument_definition>(node.at("InArgs"), parse_method_argument_definition);
        }
        if(node.contains("OutArgs"))
        {
            md->out_args = json_to_vector<method_argument_definition>(node.at("OutArgs"), parse_method_argument_definition);
        }
        // TODO: domain specific statuscodes
        return md;
    }
    else
    {
        if(node.contains("OnlyOnline"))
        {
            p->only_online = node.at("OnlyOnline").get<bool>();
        }
        if(node.contains("UserSetting"))
        {
            p->user_setting = node.at("UserSetting").get<bool>();
        }
        if(node.contains("Writeable"))
        {
            p->writeable = node.at("Writeable").get<bool>();
        }
        if(node.contains("InstanceKey"))
        {
            p->instance_key = node.at("InstanceKey").get<bool>();
        }
        if(node.contains("Enum"))
        {
            p->enum_name = node.at("Enum").get<string>();
        }
        if(node.contains("RefClass"))
        {
            p->ref_classes = {node.at("RefClass").get<string>()};
        }
        if(node.contains("RefClasses"))
        {
            p->ref_classes = node.at("RefClasses").get<vector<string>>();
        }
            
        p->overrideables = parse_overrideable_attributes(node);
    }

    return p;
}

void device_model_loader::fill_definition(shared_ptr<parameter_definition_owner> const &definition_owner,
                                          json                                   const &node,
                                          std::shared_ptr<feature_definition>    const &owner_feature,
                                          std::shared_ptr<class_definition>      const &owner_class)
{
    definition_owner->name = node.at("ID").get<string>();
    if(node.contains("Includes"))
    {
        definition_owner->includes = node.at("Includes").get<vector<string>>();
    }

    if(node.contains("Parameters"))
    {
        for (auto& p : node.at("Parameters"))
        {
            definition_owner->parameter_definitions.push_back(create_parameter_definition(p, owner_feature, owner_class));
        }
    }

    if(node.contains("Overrides"))
    {
        for (auto& p : node.at("Overrides"))
        {
            auto ovr = std::make_shared<parameter_definition_override>();
            ovr->owner_name = node.at("ID").get<string>();
            ovr->definition_id = p.at("ID").get<uint16_t>();
            ovr->attributes = parse_overrideable_attributes(p);
            definition_owner->overrides.push_back(ovr);
        }
    }
}

shared_ptr<class_definition> device_model_loader::create_class_definition(json const &node)
{
    auto c = make_shared<class_definition>();

    fill_definition(c, node, nullptr, c);
    if(node.contains("BasePath"))
    {
        c->base_path = node.at("BasePath").get<string>();
    }
    if(node.contains("BaseID"))
    {
        c->base_id = node.at("BaseID").get<parameter_id_t>();
    }
    if(node.contains("Dynamic"))
    {
        c->is_dynamic = node.at("Dynamic").get<bool>();
    }
    if(node.contains("Writeable"))
    {
        c->is_writable = node.at("Writeable").get<bool>();
    }
    if(node.contains("UserSetting"))
    {
        c->is_user_setting = node.at("UserSetting").get<bool>();
    }
    if(node.contains("InstanceKey"))
    {
        c->is_instance_key = node.at("InstanceKey").get<bool>();
    }

    if(!c->base_path.empty() && c->base_id != 0)
    {
        class_definition::build_instantiations_parameter(c);
    }

    return c;
}

shared_ptr<feature_definition> device_model_loader::create_feature_definition(json const &node)
{
    auto f = make_shared<feature_definition>();
    fill_definition(f, node, f, nullptr);

    if(node.contains("Classes"))
    {
        f->classes = node.at("Classes").get<vector<string>>();
    }
    if(node.contains("Beta"))
    {
        f->is_beta = node.at("Beta").get<bool>();
    }
    if(node.contains("Deprecated"))
    {
        f->is_deprecated = node.at("Deprecated").get<bool>();
    }
    return f;
}


shared_ptr<enum_definition> device_model_loader::create_enum_definition(json const &node)
{
    auto e = make_shared<enum_definition>();

    e->name = node.at("Name").get<string>();
    
    if(node.contains("Members"))
    {
        for (auto& m : node.at("Members"))
        {
            enum_member member;
            member.id = m.at("ID").get<uint16_t>();
            member.name = m.at("Name").get<string>();
            e->members.push_back(member);
        }
    }

    return e;
}

bool device_model_loader::parse_device_model(json const &node, device_model& model, vector<shared_ptr<parameter_definition>>& added_definitions)
{
    try
    {
        string version;
        if(node.contains("WDMMVersion"))
        {
            version = node.at("WDMMVersion").get<string>();
        }
        else
        {
            throw parameter_exception(status_codes::wdmm_version_not_supported, "\"WDMMVersion\" not found in device model");
        }

        if(node.contains("Name"))
        {
            model.name = node.at("Name").get<string>();
        }
        else
        {
            throw parameter_exception(status_codes::wdmm_version_not_supported, "\"Name\" not found in device model");
        }
        device_model::check_model_version(version);

        if(node.contains("Features"))
        {
            for (auto& featureNode : node.at("Features"))
            {
                auto def = create_feature_definition(featureNode);
                if(push_back_unique<shared_ptr<feature_definition>, name_t>(model.features, def, [](auto& x) { return x->name; }))
                {
                    append(added_definitions, def->parameter_definitions);
                }
                else
                {
                    wc_log(log_level_t::warning, "Feature '" + def->name + "' already in model, skipping");
                }
            }
        }

        if(node.contains("Classes"))
        {
            for (auto& classNode : node.at("Classes"))
            {
                auto def = create_class_definition(classNode);
                if(push_back_unique<shared_ptr<class_definition>, name_t>(model.classes, def, [](auto& x) { return x->name; }))
                {
                    append(added_definitions, def->parameter_definitions);
                    if(def->instantiations_parameter)
                    {
                        added_definitions.push_back(def->instantiations_parameter);
                    }
                }
                else
                {
                    wc_log(log_level_t::warning, "Class '" + def->name + "' already in model, skipping");
                }
            }
        }

        if(node.contains("Enums"))
        {
            for (auto& enumNode : node.at("Enums"))
            {
                auto def = create_enum_definition(enumNode);
                if(push_back_unique<shared_ptr<enum_definition>, name_t>(model.enums, def, [](auto& x) { return x->name; }))
                {  }
                else
                {
                    wc_log(log_level_t::warning, "Enum '" + def->name + "' already in model, skipping");
                }
            }
        }
    }
    catch (exception& ex)
    {
        wc_log(log_level_t::error, "Could not parse device model: " + std::string(ex.what()));
        return false;
    }
    catch (...)
    {
        wc_log(log_level_t::error, "Could not parse device model: unknown exception.");
        return false;
    }

    return true;
}

void device_model_loader::load(string& wdm_artifact, device_model& model)
{
    if(wdm_artifact.empty())
    {
        throw parameter_exception(status_codes::device_model_inaccessible);
    }
    json obj = json::parse(wdm_artifact); // TODO: switch to SAX-style parsing for less memory consumption

    vector<shared_ptr<parameter_definition>> added;
    bool success = parse_device_model(obj, model, added);
    if(!success)
    {
        throw parameter_exception(status_codes::device_model_parse_error);
    }
    append(model.definitions, added);

    model_resolver::resolve_includes(model);
    model_resolver::prepare_attributes(added, model);
}
}
}
}
