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

#include "json_representation.hpp"

namespace wago {
namespace wdx {
inline namespace wdmm { 
 
constexpr char const * const any_name                   = "Any";
constexpr char const * const classes_name               = "Classes";
constexpr char const * const code_name                  = "Code";
constexpr char const * const collection_name            = "Collection";
constexpr char const * const content_name               = "Content";
constexpr char const * const data_name                  = "Data";
constexpr char const * const device_name                = "Device";
constexpr char const * const device_id_name             = "DeviceId";
constexpr char const * const devices_name               = "Devices";
constexpr char const * const features_name              = "Features";
constexpr char const * const file_id_name               = "FileId";
constexpr char const * const firmware_version_name      = "FirmwareVersion";
constexpr char const * const id_name                    = "Id";
constexpr char const * const instance_name              = "Instance";
constexpr char const * const message_name               = "Message";
constexpr char const * const parameter_name_name        = "Name";
constexpr char const * const only_beta_name             = "OnlyBeta";
constexpr char const * const only_deprecated_name       = "OnlyDeprecated";
constexpr char const * const only_features_name         = "OnlyFeatures";
constexpr char const * const only_subpath_name          = "OnlySubPath";
constexpr char const * const only_file_ids_name         = "OnlyFileIDs";
constexpr char const * const only_methods_name          = "OnlyMethods";        
constexpr char const * const only_user_settings_name    = "OnlyUserSettings";
constexpr char const * const only_writeable_name        = "OnlyWriteable";
constexpr char const * const order_number_name          = "OrderNumber";
constexpr char const * const out_name                   = "Out";
constexpr char const * const parameter_id_name          = "ParameterId";
constexpr char const * const parameters_name            = "Parameters";
constexpr char const * const path_name                  = "Path";
constexpr char const * const rank_name                  = "Rank";
constexpr char const * const selector_name              = "Selector";
constexpr char const * const selectors_name             = "Selectors";
constexpr char const * const size_name                  = "Size";
constexpr char const * const slot_name                  = "Slot";
constexpr char const * const status_name                = "Status";
constexpr char const * const type_name                  = "Type";
constexpr char const * const value_name                 = "Value";
constexpr char const * const whole_collection_name      = "WholeCollection";
constexpr char const * const without_beta_name          = "WithoutBeta";
constexpr char const * const without_deprecated_name    = "WithoutDeprecated";
constexpr char const * const without_file_ids_name      = "WithoutFileIDs";
constexpr char const * const without_methods_name       = "WithoutMethods";    
constexpr char const * const without_user_settings_name = "WithoutUserSettings";
constexpr char const * const without_writeable_name     = "WithoutWriteable";

void to_json(json &j, status_codes const &p)
{
    j = json(to_string(p));
}

void from_json(json const &j, status_codes &p)
{
    p = from_string(j.get<std::string>());
}

void to_json(json &j, device_id const &p)
{
    j = json{{slot_name, p.slot}, {collection_name, p.device_collection_id}};
}

void from_json(json const &j, device_id &p)
{
    j.at(slot_name).get_to(p.slot);
    j.at(collection_name).get_to(p.device_collection_id);
}

class json_device_selector : public device_selector {
public:
    json_device_selector(json const &j)
    :device_selector(j.at(device_name).get<device_id>(),
                     j.at(any_name).get<bool>(),
                     j.at(whole_collection_name).get<bool>())
    { };
};

void to_json(json &j, device_selector const &p)
{
    j = json{
        {device_name, p.get_selected_device()},
        {any_name, p.is_any_selector()},
        {whole_collection_name, p.is_collection_selector()}
        };
}

void from_json(json const &j, device_selector &p)
{
    p = json_device_selector(j);
}

class json_parameter_selector : public parameter_selector {
public:
    json_parameter_selector(json const &j)
    :parameter_selector(j.at(type_name).get<parameter_selector_type>(),
                        j.at(parameter_name_name).get<std::string>(),
                        j.at(parameter_id_name).get<parameter_id_t>(),
                        j.at(device_name).get<device_selector>())
    { };
};

void to_json(json &j, parameter_selector const &p)
{
    j = json{
        {type_name, p.get_selector_type()},
        {parameter_name_name, p.get_selected_name()},
        {device_name, p.get_selected_devices()},
        {parameter_id_name, p.get_selected_id()}
        };
}

void from_json(json const &j, parameter_selector &p)
{
    p = json_parameter_selector(j);
}

void to_json(json &j, parameter_filter const &p)
{
    j = json{
        {without_file_ids_name, p._without_file_ids},
        {only_file_ids_name, p._only_file_ids},
        {without_methods_name, p._without_methods},
        {only_methods_name, p._only_methods},
        {without_user_settings_name, p._without_usersettings},
        {only_user_settings_name, p._only_usersettings},
        {without_writeable_name, p._without_writeable},
        {only_writeable_name, p._only_writeable},
        {device_name, p._device},
        {without_beta_name, p._without_beta},
        {only_beta_name, p._only_beta},
        {without_deprecated_name, p._without_deprecated},
        {only_deprecated_name, p._only_deprecated},
        {only_features_name, p._only_features},
        {only_subpath_name, p._only_subpath},
    };
}

void from_json(json const &j, parameter_filter &p)
{
    j.at(without_methods_name).get_to(p._without_methods);
    j.at(only_methods_name).get_to(p._only_methods);
    j.at(without_file_ids_name).get_to(p._without_file_ids);
    j.at(only_file_ids_name).get_to(p._only_file_ids);
    j.at(without_user_settings_name).get_to(p._without_usersettings);
    j.at(only_user_settings_name).get_to(p._only_usersettings);
    j.at(without_writeable_name).get_to(p._without_writeable);
    j.at(only_writeable_name).get_to(p._only_writeable);
    j.at(device_name).get_to(p._device);
    j.at(without_beta_name).get_to(p._without_beta);
    j.at(only_beta_name).get_to(p._only_beta);
    j.at(without_deprecated_name).get_to(p._without_deprecated);
    j.at(only_deprecated_name).get_to(p._only_deprecated);
    j.at(only_features_name).get_to(p._only_features);
    j.at(only_subpath_name).get_to(p._only_subpath);
}

void to_json(json &j, response const &p)
{
    j = json{{status_name, p.status}};
}

void from_json(json const &j, response *p)
{
    j.at(status_name).get_to(p->status);
}

void from_json(json const&j, response &p)
{
    from_json(j, &p);
}

void to_json(json &j, parameter_selector_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(!p.selected_parameters.empty())
    {
        j[selectors_name] = p.selected_parameters;
    }
}

void from_json(json const &j, parameter_selector_response &p)
{
    from_json(j, &p);
    if(j.contains(selectors_name))
    {
        j.at(selectors_name).get_to(p.selected_parameters);
    }
}

void to_json(json &j, register_device_request const &p)
{
    j = json{
        {device_id_name, p.device_id},
        {order_number_name, p.order_number},
        {firmware_version_name, p.firmware_version}
        };
}

void from_json(json const &j, register_device_request &p)
{
    j.at(device_id_name).get_to(p.device_id);
    j.at(order_number_name).get_to(p.order_number);
    j.at(firmware_version_name).get_to(p.firmware_version);
}

void to_json(json &j, class_instantiation const &p)
{
    j = json{{id_name, p.id}, {classes_name, p.classes}};
}

void from_json(json const &j, class_instantiation &p)
{
    j.at(id_name).get_to(p.id);
    j.at(classes_name).get_to(p.classes);
}

void to_json(json &j, std::shared_ptr<parameter_value> const &p)
{
    // not nice. The reason is that parameter_value.hpp shouldn't have a dependency to nlohmann::json
    // it isn't that bad though because most parameter_values are scalars and their json representation is short.
    j = json{{value_name, json::parse(p->get_json())}, {type_name, p->get_type()}};
    if(p->is_array())
    {
        j[rank_name] = p->get_rank();
    }
}

void from_json(json const &j, std::shared_ptr<parameter_value> &p)
{
    p = parameter_value::create_with_unknown_type(j[value_name].dump());
    auto t = j[type_name].get<parameter_value_types>();
    auto r = j.contains(rank_name) ? j[rank_name].get<parameter_value_rank>() : parameter_value_rank::scalar;
    p->set_type_internal(t, r);
}

void to_json(json &j, parameter_instance_id const &p)
{
    j = json{{id_name, p.id}, {instance_name, p.instance_id}, {device_name, p.device}};
}

void from_json(json const &j, parameter_instance_id &p)
{
    j.at(id_name).get_to(p.id);
    j.at(instance_name).get_to(p.instance_id);
    j.at(device_name).get_to(p.device);
}

void to_json(json &j, parameter_instance_path const &p)
{
    j = json{{path_name, p.parameter_path}, {device_name, p.device_path}};
}

void from_json(json const &j, parameter_instance_path &p)
{
    j.at(path_name).get_to(p.parameter_path);
    j.at(device_name).get_to(p.device_path);
}

void to_json(json &j, value_request const &p)
{
    j = json{{id_name, p.param_id}, {value_name, p.value}};
}

void from_json(json const &j, value_request &p)
{
    j.at(id_name).get_to(p.param_id);
    j.at(value_name).get_to(p.value);
}

void to_json(json &j, value_path_request const &p)
{
    j = json{{path_name, p.param_path}, {value_name, p.value}};
}

void from_json(json const &j, value_path_request &p)
{
    j.at(path_name).get_to(p.param_path);
    j.at(value_name).get_to(p.value);
}

void to_json(json &j, value_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(p.value)
    {
        j[value_name] = p.value;
    }
    if(p.domain_specific_status_code > 0)
    {
        j[code_name] = p.domain_specific_status_code;
    }
    if(!p.message.empty())
    {
        j[message_name] = p.message;
    }
}

void from_json(json const &j, value_response &p)
{
    from_json(j, &p);
    if(j.contains(value_name))
    {
        p.value = j.at(value_name);
    }
    if(j.contains(code_name))
    {
        j.at(code_name).get_to(p.domain_specific_status_code);
    }
    if(j.contains(message_name))
    {
        j.at(message_name).get_to(p.message);
    }
}

void to_json(json &j, parameter_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[id_name] = p.id;
    j[path_name] = p.path;
    if(p.value)
    {
        j[value_name] = p.value;
    }
    if(p.domain_specific_status_code > 0)
    {
        j[code_name] = p.domain_specific_status_code;
    }
    if(!p.message.empty())
    {
        j[message_name] = p.message;
    }
}

void from_json(json const &j, parameter_response &p)
{
    from_json(j, &p);
    j.at(id_name).get_to(p.id);
    j.at(path_name).get_to(p.path);
    if(j.contains(value_name))
    {
        j.at(value_name).get_to(p.value);
    }
    if(j.contains(code_name))
    {
        j.at(code_name).get_to(p.domain_specific_status_code);
    }
    if(j.contains(message_name))
    {
        j.at(message_name).get_to(p.message);
    }
}

void to_json(json &j, method_invocation_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(p.domain_specific_status_code > 0)
    {
        j[code_name] = p.domain_specific_status_code;
    }
    if(!p.message.empty())
    {
        j[message_name] = p.message;
    }
    if(!p.out_args.empty())
    {
        j[out_name] = p.out_args;
    }
}

void from_json(json const &j, method_invocation_response &p)
{
    from_json(j, &p);
    if(j.contains(code_name))
    {
        j.at(code_name).get_to(p.domain_specific_status_code);
    }
    if(j.contains(message_name))
    {
        j.at(message_name).get_to(p.message);
    }
    if(j.contains(out_name))
    {
        j.at(out_name).get_to(p.out_args);
    }
}

void to_json(json &j, method_invocation_named_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(p.domain_specific_status_code > 0)
    {
        j[code_name] = p.domain_specific_status_code;
    }
    if(!p.message.empty())
    {
        j[message_name] = p.message;
    }
    if(!p.out_args.empty())
    {
        j[out_name] = p.out_args;
    }
}

void from_json(json const &j, method_invocation_named_response &p)
{
    from_json(j, &p);
    if(j.contains(code_name))
    {
        j.at(code_name).get_to(p.domain_specific_status_code);
    }
    if(j.contains(message_name))
    {
        j.at(message_name).get_to(p.message);
    }
    if(j.contains(out_name))
    {
        j.at(out_name).get_to(p.out_args);
    }
}

void to_json(json &j, set_parameter_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(p.domain_specific_status_code > 0)
    {
        j[code_name] = p.domain_specific_status_code;
    }
    if(!p.message.empty())
    {
        j[message_name] = p.message;
    }
    if(p.value)
    {
        j[value_name] = p.value;
    }
}

void from_json(json const &j, set_parameter_response &p)
{
    from_json(j, &p);
    if(j.contains(code_name))
    {
        j.at(code_name).get_to(p.domain_specific_status_code);
    }
    if(j.contains(message_name))
    {
        j.at(message_name).get_to(p.message);
    }
    if(j.contains(value_name))
    {
        j.at(value_name).get_to(p.value);
    }
}

void to_json(json &j, device_selector_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(!p.selected_devices.empty())
    {
        j[selectors_name] = p.selected_devices;
    }
}

void from_json(json const &j, device_selector_response &p)
{
    from_json(j, &p);
    if(j.contains(selectors_name))
    {
        j.at(selectors_name).get_to(p.selected_devices);
    }
}

void to_json(json &j, wdd_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[content_name] = p.content;
}

void from_json(json const &j, wdd_response &p)
{
    from_json(j, &p);
    j.at(content_name).get_to(p.content);
}

void to_json(json &j, wdm_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[content_name] = p.wdm_content;
}

void from_json(json const &j, wdm_response &p)
{
    from_json(j, &p);
    j.at(content_name).get_to(p.wdm_content);
}

void to_json(json &j, device_extension_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[selector_name] = p.selected_devices;
    j[features_name] = p.extension_features;
}

void from_json(json const &j, device_extension_response &p)
{
    from_json(j, &p);
    j.at(selector_name).get_to(p.selected_devices);
    auto x = j.dump();
    j.at(features_name).get_to(p.extension_features);
}

void to_json(json &j, device_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[id_name] = p.id;
    j[order_number_name] = p.order_number;
    j[firmware_version_name] = p.firmware_version;
}

void from_json(json const &j, device_response &p)
{
    from_json(j, &p);
    j.at(id_name).get_to(p.id);
    j.at(order_number_name).get_to(p.order_number);
    j.at(firmware_version_name).get_to(p.firmware_version);
}

void to_json(json &j, device_collection_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(!p.devices.empty())
    {
        j[devices_name] = p.devices;
    }
}

void from_json(json const &j, device_collection_response &p)
{
    from_json(j, &p);
    if(j.contains(devices_name))
    {
        j.at(devices_name).get_to(p.devices);
    }
}

void to_json(json &j, parameter_response_list_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[parameters_name] = p.param_responses;
}

void from_json(json const &j, parameter_response_list_response &p)
{
    from_json(j, &p);
    j.at(parameters_name).get_to(p.param_responses);
}

}

inline namespace file_transfer {

void to_json(json &j, file_id_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[file_id_name] = p.registered_file_id;
}

void from_json(json const &j, file_id_response &p)
{
    from_json(j, &p);
    j.at(file_id_name).get_to(p.registered_file_id);
}

void to_json(json &j, file_read_response const &p)
{
    to_json(j, static_cast<response>(p));
    if(!p.data.empty())
    {
        j[data_name] = p.data;
    }
}

void from_json(json const &j, file_read_response &p)
{
    from_json(j, &p);
    if(j.contains(data_name))
    {
        j.at(data_name).get_to(p.data);
    }
}

void to_json(json &j, file_info_response const &p)
{
    to_json(j, static_cast<response>(p));
    j[size_name] = p.file_size;
}

void from_json(json const &j, file_info_response &p)
{
    from_json(j, &p);
    j.at(size_name).get_to(p.file_size);
}

}

}}
