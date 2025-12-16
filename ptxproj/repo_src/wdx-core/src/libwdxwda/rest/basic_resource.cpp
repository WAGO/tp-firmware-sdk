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
///
///  \brief    Implementation of container for a generic basic resource.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "basic_resource.hpp"
#include "data_error.hpp"

#include "wda_ipc/representation.hpp"
#include "wago/wdx/requests.hpp"
#include <wc/assertion.h>
#include <algorithm>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::to_string;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
template<class T>
static string id_from(T const &);
template<class T>
static string type_from(T const &);

template<class T>
static string self_link_from(T const &t, std::string id)
{
    return "/wda/" + type_from(t) + "/" + (id.empty() ? id_from(t) : id);
}

template<class T>
static map<string, basic_relationship> relationships_from(T const &, std::string const &WC_UNUSED_PARAM(self_link))
{
    return {};
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

template<>
string id_from(std::nullptr_t const &)
{
    bool cannot_deduce_id_from_nullptr = false;
    WC_ASSERT(cannot_deduce_id_from_nullptr);
    return "";
}

template<>
string type_from(std::nullptr_t const &)
{
    bool cannot_deduce_type_from_nullptr = false;
    WC_ASSERT(cannot_deduce_type_from_nullptr);
    return "";
}

template<>
string id_from(wdx::device_response const &device)
{
    return wda_ipc::to_string(device.id);
}

template<>
string type_from(wdx::device_response const &)
{
    return "devices";
}

template<>
string id_from(service_identity_i const * const &service_identity)
{
    return service_identity->get_id();
}

template<>
string type_from(service_identity_i const * const &)
{
    return "service-identity";
}

template<>
string self_link_from(service_identity_i const * const &, std::string)
{
    return "/wda";
}

template<>
string id_from(method_run_data const &)
{
    bool id_from_method_invocation_not_implemented = false;
    WC_ASSERT(id_from_method_invocation_not_implemented);
    return "";
}

template<>
string type_from(method_run_data const &)
{
    return "runs";
}

template<>
string self_link_from(method_run_data const &resp, std::string id)
{
    return "/wda/methods/" + resp.get_associated_method() + "/runs/" + id;
}

template<>
string id_from(wdx::parameter_response const &parameter)
{
    return wda_ipc::to_string(parameter.path);
}

template<>
string type_from(wdx::parameter_response const &)
{
    return "parameters";
}

template<>
string id_from(wdx::set_parameter_response const &)
{
    WC_FAIL("id_from(set_parameter_response) not implemented");
    return "";
}

template<>
string id_from(wdx::value_path_request const &request)
{
    return wda_ipc::to_string(request.param_path);
}

template<>
string type_from(wdx::set_parameter_response const &)
{
    return "parameters";
}

template<>
string id_from(method_response const &method)
{
    return wda_ipc::to_string(method.path);
}

template<>
string type_from(method_response const &)
{
    return "methods";
}

template<>
string type_from(monitoring_list_data const &)
{
    return "monitoring-lists";
}

template<>
string id_from(monitoring_list_data const &monitoring_list)
{
    return wda_ipc::to_string(monitoring_list.get_monitoring_list_id());
}

template<>
string id_from(wdx::enum_definition_response const &enum_def)
{
    std::string id = enum_def.definition->name;
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    return id;
}

template<>
string type_from(wdx::enum_definition_response const &)
{
    return "enum-definitions";
}

template<>
string type_from(wdx::feature_response const &)
{
    return "features";
}

template<>
string id_from(wdx::feature_response const &feature_resp)
{
    std::string id = feature_resp.device_path + "-" + feature_resp.feature.name;
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    return id;
}


template<>
string type_from(method_definition_data const &)
{
    return "method-definitions";
}

template<>
string id_from(method_definition_data const &method_def)
{
    return method_def.get_id();
}

template<>
string type_from(method_arg_definition_data const &arg_def)
{
    return arg_def.get_arg_type() == method_arg_definition_data::arg_type::in 
        ? "method-inarg-definitions"
        : "method-outarg-definitions";
}

template<>
string id_from(method_arg_definition_data const &arg_def)
{
    return arg_def.get_id();
}

template<>
string self_link_from(method_arg_definition_data const &arg_def, std::string)
{
    std::string arg_rel_name = arg_def.get_arg_type() == method_arg_definition_data::arg_type::in 
        ? "inargs"
        : "outargs";
    return "/wda/method-definitions/" + arg_def.get_associated_method_definition() + "/" + arg_rel_name + "/" + arg_def.get_lowercase_arg_name();
}


template<>
string type_from(parameter_definition_data const &)
{
    return "parameter-definitions";
}

template<>
string id_from(parameter_definition_data const &param_def_data)
{
    return wda_ipc::to_string(param_def_data.get_associated_parameter());
}

template<>
string type_from(class_instance_data const &)
{
    return "instances";
}

template<>
string id_from(class_instance_data const &instance_data)
{
    return (instance_data.is_dangling())
        ? "" 
        : wda_ipc::to_string(instance_data.get_class_parameter()) + '-' + wda_ipc::to_string(instance_data.get_instance_no());
}

template<>
string self_link_from(class_instance_data const &instance, std::string)
{
    return (instance.is_dangling())
        ? "" 
        : ( "/wda/parameters/" + wda_ipc::to_string(instance.get_class_parameter()) 
          + "/instances/"      + wda_ipc::to_string(instance.get_instance_no()) );
}

template<>
std::map<string, basic_relationship> relationships_from(wdx::parameter_response const &param, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    relationships.emplace("device", basic_relationship({
        {"related", self_link + "/device"}
    }));
    relationships.emplace("definition", basic_relationship({
        {"related", self_link + "/definition"}
    }));
    if(param.definition)
    {
        if (param.definition->value_type == wdx::parameter_value_types::instance_identity_ref)
        {
            relationships.emplace("referencedInstances", basic_relationship({
                {"related", self_link + "/referencedinstances"}
            }));
        }
        else if (param.definition->value_type == wdx::parameter_value_types::instantiations)
        {
            relationships.emplace("instances", basic_relationship({
                {"related", self_link + "/instances"}
            }));
        }
    }
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(method_response const &, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    relationships.emplace("device", basic_relationship({
        {"related", self_link + "/device"}
    }));
    relationships.emplace("runs", basic_relationship({
        {"related", self_link + "/runs"}
    }));
    relationships.emplace("definition", basic_relationship({
        {"related", self_link + "/definition"}
    }));
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(wdx::device_response const &device, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    relationships.emplace("features", basic_relationship({
        {"related", self_link + "/features"}
    }));
#if ENABLE_SUBDEVICES
    if(device.id == wdx::device_id::headstation)
    {
        relationships.emplace("subdevices", basic_relationship({
            {"related", self_link + "/subdevices"}
        }));
    }
#endif
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(wdx::feature_response const &, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    relationships.emplace("includedFeatures", basic_relationship({
        {"related", self_link + "/includedfeatures"}
    }));
    relationships.emplace("containedParameters", basic_relationship({
        {"related", self_link + "/containedparameters"}
    }));
    relationships.emplace("containedMethods", basic_relationship({
        {"related", self_link + "/containedmethods"}
    }));
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(method_definition_data const &, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    relationships.emplace("inArgs", basic_relationship({
        {"related", self_link + "/inargs"}
    }));
    relationships.emplace("outArgs", basic_relationship({
        {"related", self_link + "/outargs"}
    }));
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(method_arg_definition_data const &def, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    if (def.get_argument_definition()->value_type == wdx::wdmm::parameter_value_types::enum_member)
    {
        relationships.emplace("enum", basic_relationship({
            {"related", self_link + "/enum"}
        }));
    }
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(parameter_definition_data const &def, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    if (def.get_definition()->value_type == wdx::wdmm::parameter_value_types::enum_member)
    {
        relationships.emplace("enum", basic_relationship({
            {"related", self_link + "/enum"}
        }));
    }
    return relationships;
}

template<>
std::map<string, basic_relationship> relationships_from(class_instance_data const &, std::string const &self_link)
{
    std::map<string, basic_relationship> relationships;
    relationships.emplace("device", basic_relationship({
        {"related", self_link + "/device"}
    }));
    relationships.emplace("parameters", basic_relationship({
        {"related", self_link + "/parameters"}
    }));
    relationships.emplace("methods", basic_relationship({
        {"related", self_link + "/methods"}
    }));
    return relationships;
}
//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
template class basic_resource<std::nullptr_t>;
template class basic_resource<service_identity_i const *>;
template class basic_resource<wdx::device_response>;
template class basic_resource<wdx::parameter_response>;
template class basic_resource<wdx::set_parameter_response>;
template class basic_resource<method_response>;
template class basic_resource<method_run_data>;
template class basic_resource<monitoring_list_data>;
template class basic_resource<wdx::enum_definition_response>;
template class basic_resource<wdx::feature_response>;
template class basic_resource<method_definition_data>;
template class basic_resource<method_arg_definition_data>;
template class basic_resource<parameter_definition_data>;
template class basic_resource<class_instance_data>;

template <class T>
basic_resource<T>::basic_resource(string const &type,
                                  string const &id,
                                  T const &data,
                                  map<string, basic_relationship> const &relationships,
                                  map<string, string> const &data_links)
: type_m(type)
, id_m(id)
, data_m(data)
, relationships_m(relationships)
, links_m(data_links)
{
    std::string self_link;
    if (links_m.count("self") < 1)
    {
        self_link = self_link_from(data, id);
        links_m.emplace("self", self_link);
    }
    else
    {
        self_link = links_m.at("self");
    }
    auto default_relationships = relationships_from(data, self_link);
    relationships_m.insert(default_relationships.begin(), default_relationships.end());
}

template basic_resource<wdx::set_parameter_response>::basic_resource(wdx::value_path_request         const &, 
                                                                     wdx::set_parameter_response     const &, 
                                                                     map<string, basic_relationship> const &,
                                                                     map<string, string>             const &);

template <class T>
template <class IdType>
basic_resource<T>::basic_resource(IdType const &id_type, 
                                  T const &data,
                                  map<string, basic_relationship> const &relationships,
                                  map<string, string> const &data_links)
: basic_resource(type_from(data), id_from(id_type), data, relationships, data_links)
{
    return;
}

template <class T>
basic_resource<T>::basic_resource(T const &data,
                                  map<string, basic_relationship> const &relationships,
                                  map<string, string> const &data_links)
: basic_resource(type_from(data), id_from(data), data, relationships, data_links)
{
    return;
}

template <class T>
string const & basic_resource<T>::get_type() const
{
    return type_m;
}

template <class T>
string const & basic_resource<T>::get_id() const
{
    return id_m;
}

template <class wrapped_type>
wrapped_type const & basic_resource<wrapped_type>::get_data() const
{
    return data_m;
}

template <class T>
bool basic_resource<T>::has_relationships() const
{
    return !relationships_m.empty();
}

template <class T>
map<string, basic_relationship> const & basic_resource<T>::get_relationships() const
{
    return relationships_m;
}

template <class T>
bool basic_resource<T>::has_links() const
{
    return !links_m.empty();
}

template <class T>
map<string, string> const & basic_resource<T>::get_links() const
{
    return links_m;
}

template <class T>
vector<shared_ptr<core_error>> basic_resource<T>::get_errors() const
{
    return { };
}

// we only need an overload for parameter_response, as this is 
// actually the only case in which we allow documents to carry errors
template <>
vector<shared_ptr<core_error>> basic_resource<wdx::parameter_response>::get_errors() const
{
    return { std::make_shared<core_error>(data_m) };
}

template <class T>
bool basic_resource<T>::has_errors() const
{
    return false;
}

template <>
bool basic_resource<wdx::parameter_response>::has_errors() const
{
    return data_m.has_error();
}

template <class T>
bool basic_resource<T>::has_error_like_meta() const
{
    return !get_error_like_meta().empty();
}

template <class T>
map<string, shared_ptr<core_error>> basic_resource<T>::get_error_like_meta() const
{
    return {};
}

template <>
map<string, shared_ptr<core_error>> basic_resource<wdx::parameter_response>::get_error_like_meta() const
{
    map<string, shared_ptr<core_error>> meta;
    if (data_m.status == wdx::status_codes::status_value_unavailable)
    {
        meta.emplace("value", std::make_shared<core_error>(data_m));
    }
    return meta;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
