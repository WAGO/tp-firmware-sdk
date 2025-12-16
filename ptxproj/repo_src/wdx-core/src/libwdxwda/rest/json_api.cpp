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
///
///  \brief    Implementation of JSON:API serialization.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "json_api.hpp"
#include "definitions.hpp"
#include "attribute_error.hpp"
#include "relationship_error.hpp"
#include "core_exception.hpp"
#include "data_exception.hpp"

#include "wda_ipc/representation.hpp"
#include <wc/assertion.h>
#include <wc/compiler.h>
#include <wc/structuring.h>
#include <wc/log.h>
#include <nlohmann/json.hpp>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
using namespace nlohmann; // NOLINT(google-build-using-namespace)

template<typename T>
json serialize_to_json(const std::vector<T> &elements);

template<typename T>
json serialize_to_json(const std::map<std::string, T> &elements);

json serialize_to_json(const wdx::device_response &device);
json serialize_to_json(const std::shared_ptr<wdx::parameter_value> value);
json serialize_to_json(const service_identity_i * const &service_identity);
json serialize_to_json(const std::shared_ptr<http_exception> &error);
json serialize_to_json(const shared_ptr<core_error> &error);
json serialize_to_json(const shared_ptr<data_error> &error);
json serialize_to_json(const core_error &error);
json serialize_to_json(const generic_document& single_resource);
json serialize_to_json(const generic_collection_document& resource_collection);
json serialize_to_json(const generic_resource& resource);
json serialize_to_json(const service_identity_document &document);
json serialize_to_json(const service_identity_resource &resource);
json serialize_to_json(const device_resource& resource);
json serialize_to_json(const device_collection_document& device_topology);
json serialize_to_json(const basic_relationship& relationship);
json serialize_to_json(const related_resource& related);
json serialize_to_json(const device_document& device_topology);
json serialize_to_json(const parameter_resource& parameter_response);
json serialize_to_json(const parameter_document& parameter);
json serialize_to_json(const parameter_collection_document& parameters);
json serialize_to_json(const method_resource& method_response);
json serialize_to_json(const method_document& method);
json serialize_to_json(const method_collection_document& methods);
json serialize_to_json(const method_invocation_document& invocation);
json serialize_to_json(const method_invocation_resource& invocation);
json serialize_to_json(const method_invocation_collection_document& invocations);
json serialize_to_json(const monitoring_list_document& monitoring_list);
json serialize_to_json(const monitoring_list_collection_document& monitoring_lists);
json serialize_to_json(const monitoring_list_resource& monitoring_list);
json serialize_to_json(const enum_definition_document& enum_definition);
json serialize_to_json(const enum_definition_collection_document& enum_definitions);
json serialize_to_json(const enum_definition_resource& enum_definition);
json serialize_to_json(const wdx::enum_member& enum_definition_case);
json serialize_to_json(const feature_document& feature_definition);
json serialize_to_json(const feature_collection_document& feature_definitions);
json serialize_to_json(const feature_resource& feature_definition);
json serialize_to_json(const method_definition_document& method_def);
json serialize_to_json(const method_definition_collection_document& method_defs);
json serialize_to_json(const method_definition_resource& method_def);
json serialize_to_json(const method_arg_definition_document& arg_def);
json serialize_to_json(const method_arg_definition_collection_document& arg_defs);
json serialize_to_json(const method_arg_definition_resource& arg_def);
json serialize_to_json(const parameter_definition_document& parameter_def);
json serialize_to_json(const parameter_definition_collection_document& parameter_defs);
json serialize_to_json(const parameter_definition_resource& parameter_def);
json serialize_to_json(const class_instance_document& class_instance);
json serialize_to_json(const class_instance_collection_document& class_instances);
json serialize_to_json(const class_instance_resource& class_instance);

void deserialize_from_json(json const &j, std::string &value);
void deserialize_from_json(json const &j, std::shared_ptr<wdx::parameter_value> &value);
void deserialize_from_json(json const &j, std::vector<wdx::value_path_request> &value);
void deserialize_from_json(json const &j, wdx::value_path_request &value);
void deserialize_from_json(json const &j, create_monitoring_list_request &value);
void deserialize_from_json(json const &j, std::map<std::string, std::shared_ptr<wdx::parameter_value>> &new_map);

//------------------------------------------------------------------------------
// generic container serializer functions
//------------------------------------------------------------------------------
template<typename T>
json serialize_to_json(const std::vector<T> &elements)
{
    json json_array = json::array();
    for(auto &element : elements)
    {
        json_array.push_back(serialize_to_json(element));
    }

    return json_array;
}

template<typename T>
json serialize_to_json(const std::map<std::string, T> &elements)
{
    json json_object = json::object();
    for(auto &element : elements)
    {
        json_object.emplace(element.first, serialize_to_json(element.second));
    }

    return json_object;
}

//------------------------------------------------------------------------------
// type specific serializer functions
//------------------------------------------------------------------------------
json serialize_to_json(const wdx::device_response &device)
{
    auto id = wda_ipc::to_string(device.id);

    json j{
        {"orderNumber", device.order_number},
        {"firmwareVersion", device.firmware_version}
    };
    if(id != "0-0")
    {
        j.emplace("busPosition", device.id.slot);
    }

    return j;
}

json serialize_to_json(const std::shared_ptr<wdx::parameter_value> value)
{
    json j;
    if(value != nullptr)
    {
        auto data_type = wda_ipc::to_string(value->get_type());
        auto data_rank = wda_ipc::to_string(value->get_rank());
        j = json{
            {"value", json::parse(value->get_json())},
            {"dataType", data_type},
            {"dataRank", data_rank}
        };
        // add the value as a string, if it is numeric.
        // Due to some client limitations, we need to provide an alternative value
        // for at least >53bit values. To keep the API as consistent as possible
        // we will provide a "stringValue" for every numeric value.
        // FIXME: there should be a helper method somewhere to detect, if a datatype is numeric/integer/float.
        if((data_type.find("int") != std::string::npos) || (data_type.find("float") != std::string::npos))
        {
            j.emplace("stringValue", value->get_json());
        }
    }
    else
    {
        j = json{
            {"value",    nullptr },
            {"dataType", nullptr },
            {"dataRank", nullptr }
        };
    }

    return j;
}

json serialize_to_json(service_identity_i const * const &service_identity)
{
    json j{
        {"apiVersion", REST_API_VERSION},
        {"coreVersion", WC_SUBST_STR(WDXWDA_VERSION)},
        {"serviceName", service_identity->get_name()},
        {"serviceVersion", service_identity->get_version_string()}
    };

    return j;
}

json serialize_to_json(const std::shared_ptr<http::http_exception> &error)
{
    json j;
    auto const *data_errors = dynamic_cast<data_exception const *>(error.get()); // parasoft-suppress CERT_C-EXP39-b-3 "data_exception is a derived type of a http_exception"
    if((data_errors != nullptr) && (!data_errors->get_errors().empty()))
    {
        j = json{
            {"errors", serialize_to_json(data_errors->get_errors())}
        };
    }
    else
    {
        j = json{
            {"errors", {{
                {"status", to_string(error->get_http_status_code())},
                {"title", error->get_title()},
                {"detail", error->what()}
                }}
            }
        };

        auto const *core_error = dynamic_cast<core_exception const *>(error.get()); // parasoft-suppress CERT_C-EXP39-b-3 "core_exception is a derived type of a http_exception"
        if(core_error != nullptr)
        {
            auto const core_status = core_error->get_core_status_code();
            WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(core_status));
            j["errors"][0].emplace("code", wda_ipc::to_string(core_status));

            auto const domain_status = core_error->get_domain_status_code();
            WC_STATIC_ASSERT(sizeof(unsigned) >= sizeof(domain_status));

            if(domain_status != domain_status_code_not_set)
            {
                j["errors"][0].emplace("meta", json::object({
                    {"domainSpecificStatusCode", std::to_string(static_cast<unsigned>(domain_status))}
                }));
            }
        }
    }

    return j;
}

json serialize_to_json(const shared_ptr<core_error> &error)
{
    return serialize_to_json(*error);
}

json serialize_to_json(const shared_ptr<data_error> &error)
{
    return serialize_to_json(*error);
}

json serialize_to_json(const core_error &error)
{
    json j{
        {"status", wda::http::to_string(error.get_http_status_code())},
        {"code", wda_ipc::to_string(error.get_core_status_code())},
        {"title", error.get_title()}
    };
    auto message = error.get_message();
    if(!message.empty())
    {
        j.emplace("detail", message);
    }
    uint16_t const domain_status = error.get_domain_status_code();
    if(domain_status != domain_status_code_not_set)
    {
        j.emplace("meta", json::object({
            {"domainSpecificStatusCode", std::to_string(static_cast<unsigned>(domain_status))}
        }));
    }

    // Check for data information
    auto *data_error_ptr = dynamic_cast<data_error const *>(&error); // parasoft-suppress CERT_C-EXP39-b-3 "data_error is a derived type of a core_error"
    if(data_error_ptr != nullptr)
    {
        string json_pointer;
        int data_index      = data_error_ptr->get_data_index();
        if(data_index >= 0) // FIXME: Workaround: When we have indexed data? But we have requests without any data...
        {
            string data_index_part = (data_index >= 0 ? ("/" + std::to_string(data_index)) : "");
            json_pointer           = "/data" + data_index_part;
        }

        // Check for attribute information
        auto *attribute_error_ptr = dynamic_cast<attribute_error const *>(&error); // parasoft-suppress CERT_C-EXP39-b-3 "attribute_error is a derived type of a core_error"
        if(attribute_error_ptr != nullptr)
        {
            json_pointer += "/data/attributes/" + attribute_error_ptr->get_attribute_name();
        }
        else
        {
            auto relationship_error_ptr = dynamic_cast<relationship_error const *>(&error); // parasoft-suppress CERT_C-EXP39-b-3 "relationship_error is a derived type of a core_error"
            if(relationship_error_ptr != nullptr)
            {   
                int rel_index           = relationship_error_ptr->get_relationship_index();
                string rel_index_part   = (rel_index >= 0 ? ("/" + std::to_string(rel_index)) : "");
                string sub_data = "/data"; // FIXME: Workaround because of embedded data part in post request for monitor list
                json_pointer += "/data/relationships/" + relationship_error_ptr->get_relationship_name() + sub_data + rel_index_part;
            }
        }

        // Emplace error pointer only when not empty
        if(!json_pointer.empty())
        {
            j.emplace("source", json {
                {"pointer", json_pointer}
            });
        }
    }

    return j;
}

json serialize_to_json(const generic_document& single_resource)
{
    json j{
        {"data",  serialize_to_json(single_resource.get_data())},
        {"links",                   single_resource.get_links()}
    };

    return j;
}

json serialize_to_json(const generic_collection_document& resource_collection)
{
    WC_DEBUG_LOG(string("Serialize resource collection, member count: ") + std::to_string(resource_collection.get_data().size()));

    json j{
        {"data",  serialize_to_json(resource_collection.get_data())},
        {"links",                   resource_collection.get_links()}
    };

    return j;
}

json serialize_to_json(const generic_resource& resource)
{
    json j{
        {"id",                              resource.get_id()},
        {"type",                            resource.get_type()},
        {"links",                           resource.get_links()},
        {"relationships", serialize_to_json(resource.get_relationships())}
    };

    return j;
}


json serialize_to_json(const service_identity_document& document)
{
    json j{
        {"data",  serialize_to_json(document.get_data())},
        {"links",                   document.get_links()}
    };

    return j;
}

json serialize_to_json(const service_identity_resource& resource)
{
    json j{
        {"id",                              resource.get_id()},
        {"type",                            resource.get_type()},
        {"links",                           resource.get_links()},
        {"attributes",    serialize_to_json(resource.get_data())},
        {"relationships", serialize_to_json(resource.get_relationships())}
    };

    return j;
}

json serialize_to_json(const device_resource& resource)
{
    json j{
        {"id",                           resource.get_id()},
        {"type",                         resource.get_type()},
        {"links",                        resource.get_links()},
        {"attributes",    serialize_to_json(resource.get_data())},
        {"relationships", serialize_to_json(resource.get_relationships())}
    };
    return j;
}

json serialize_to_json(const basic_relationship& relationship)
{
    json j{
        {"links", relationship.get_links()}
    };
    switch(relationship.get_data_mode())
    {
        case basic_relationship::data_mode::single:
            j.emplace("data", serialize_to_json(relationship.get_data().at(0)));
            break;
        case basic_relationship::data_mode::multi:
            j.emplace("data", serialize_to_json(relationship.get_data()));
            break;
        default:
            // no data member to embed
            break;
    }

    return j;
}

json serialize_to_json(const related_resource& related)
{
    json j{
        {"id",   related.id},
        {"type", related.type}
    };

    return j;
}

json serialize_to_json(const device_document& device_topology)
{
    json j{
        {"data",  serialize_to_json(device_topology.get_data())},
        {"links",                   device_topology.get_links()}
    };

    return j;
}

json serialize_to_json(const device_collection_document& device_topology)
{
    WC_DEBUG_LOG(string("Serialize topology, device count: ") + std::to_string(device_topology.get_data().size()));

    json j{
        {"data",  serialize_to_json(device_topology.get_data())},
        {"links",                   device_topology.get_links()}
    };

    return j;
}

json serialize_to_json(const parameter_resource& parameter_response)
{
    json j{
        {"id",   parameter_response.get_id()},
        {"type", parameter_response.get_type()},
        {"links",parameter_response.get_links()}
    };
    auto value = parameter_response.get_data().value;
    // fixme: as the core does not provide us with a parameter_value
    //        instance at all, when status is 'status_value_unavailable'
    //        we need to get this from the definition for now.
    auto definition = parameter_response.get_data().definition;
    json attributes = (value != nullptr) ? serialize_to_json(value) : (json {
        {"value",    nullptr},
        {"dataType", definition ? json(wda_ipc::to_string(definition->value_type)) : json(nullptr)},
        {"dataRank", definition ? json(wda_ipc::to_string(definition->value_rank)) : json(nullptr)}
    });

    // add beta attribute only, if a parameter IS beta
    if(definition != nullptr && definition->is_beta)
    {
        attributes.emplace("beta", true);
    }

    // add deprecated attribute only, if a parameter IS deprecated
    if(definition != nullptr && definition->is_deprecated)
    {
        attributes.emplace("deprecated", true);
    }

    // needed for '?parameter-errors-as-data-attributes=true' only
    if(parameter_response.has_errors())
    {
        attributes.emplace("errors", serialize_to_json(parameter_response.get_errors()));
    }

    if(parameter_response.has_error_like_meta())
    {
        j.emplace("meta", serialize_to_json(parameter_response.get_error_like_meta()));
    }
    attributes.emplace("path", parameter_response.get_data().path.parameter_path);
    j.emplace("attributes", attributes);
    if(parameter_response.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(parameter_response.get_relationships()));
    }

    return j;
}

json serialize_to_json(const parameter_document& parameter)
{
    WC_DEBUG_LOG("Serialize parameter result.");

    json j{
        {"data",  serialize_to_json(parameter.get_data())},
        {"links",                   parameter.get_links()}
    };

    return j;
}

json serialize_to_json(const parameter_collection_document& parameters)
{
    WC_DEBUG_LOG("Serialize parameter collection result.");

    json j{
        {"data",  serialize_to_json(parameters.get_data())}
    };

    if (parameters.has_links())
    {
        j.emplace("links", parameters.get_links());
    }

    return j;
}


json serialize_to_json(const set_parameter_resource& parameter_response)
{
    json j{
        {"id",   parameter_response.get_id()},
        {"type", parameter_response.get_type()},
        {"links",parameter_response.get_links()}
    };
    auto value = parameter_response.get_data().value;
    json attributes{
        {"value",    (value != nullptr) ? json::parse(value->get_json()) : json(nullptr)}
    };
    j.emplace("attributes", attributes);
    return j;
}

json serialize_to_json(const set_parameter_document& parameter)
{
    WC_DEBUG_LOG("Serialize set parameter result.");

    json j{
        {"data",  serialize_to_json(parameter.get_data())},
    };
    if (parameter.has_links())
    {
        j.emplace("links", parameter.get_links());
    }

    return j;
}

json serialize_to_json(const set_parameter_collection_document& parameters)
{
    WC_DEBUG_LOG("Serialize set parameter collection result.");

    json j{
        {"data",  serialize_to_json(parameters.get_data())}
    };

    if (parameters.has_links())
    {
        j.emplace("links", parameters.get_links());
    }

    return j;
}

json serialize_to_json(const method_resource& method_response)
{
    json j{
        {"id",   method_response.get_id()},
        {"type", method_response.get_type()},
        {"links",method_response.get_links()},
    };
    // FIXME: insert method inargs and out args by extracting some member from the response like we do for the value.
    //        this is currently not provided by the core lib.
    json attributes({});
    if(method_response.has_errors())
    {
        attributes.emplace("errors", serialize_to_json(method_response.get_errors()));
    }
    attributes.emplace("path", method_response.get_data().path.parameter_path);

    // add beta attribute only, if a method IS beta
    auto definition = method_response.get_data().definition;
    if(definition != nullptr && definition->is_beta)
    {
        attributes.emplace("beta", true);
    }

    // add deprecated attribute only, if a method IS deprecated
    if(definition != nullptr && definition->is_deprecated)
    {
        attributes.emplace("deprecated", true);
    }

    j.emplace("attributes", attributes);
    if(method_response.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(method_response.get_relationships()));
    }

    return j;
}

json serialize_to_json(const method_document& method)
{
    WC_DEBUG_LOG("Serialize parameter result.");

    json j{
        {"data",  serialize_to_json(method.get_data())},
        {"links", method.get_links()}
    };

    return j;
}

json serialize_to_json(const method_collection_document& methods)
{
    WC_DEBUG_LOG("Serialize parameter collection result.");

    json j{
        {"data",  serialize_to_json(methods.get_data())},
        {"links", methods.get_links()}
    };

    return j;
}

json serialize_to_json(const method_invocation_document& invocation)
{
    WC_DEBUG_LOG("Serialize method invocation result.");

    json j{
        {"data", serialize_to_json(invocation.get_data())},
    };
    if(invocation.has_links()) {
        j.emplace("links", invocation.get_links());
    }

    return j;
}

json serialize_to_json(const method_invocation_resource& invocation)
{
    json j{
        {"id",         invocation.get_id()},
        {"type",       invocation.get_type()},
        {"links",      invocation.get_links()},
        {"attributes", json::object()}
    };
    auto &attributes = j.at("attributes");

    auto &run_data = invocation.get_data();
    auto  response = run_data.get_response();
    if(response.get() == nullptr)
    {
        attributes.emplace("executionStatus", "progress");
        attributes.emplace("timeout",         run_data.get_timeout());
    }
    else if(response->is_success())
    {
        attributes.emplace("executionStatus",                   "done");
        attributes.emplace("timeout",                           run_data.get_timeout());
        attributes.emplace("outArgs",         serialize_to_json(response->out_args));
    }
    else
    {
        core_error error(response->status, response->get_message(), response->domain_specific_status_code, "method run",
                         invocation.get_id(), true, "method", run_data.get_associated_method());

        attributes.emplace("executionStatus",                   "error");
        attributes.emplace("timeout",                           run_data.get_timeout());
        attributes.emplace("code",                              wda_ipc::to_string(error.get_core_status_code()));
        attributes.emplace("title",                             error.get_title());
        attributes.emplace("detail",                            error.get_message());
        if(    (error.get_core_status_code()   == wdx::status_codes::could_not_invoke_method)
            && (error.get_domain_status_code() != domain_status_code_not_set))
        {
            attributes.emplace("domainSpecificStatusCode", std::to_string(static_cast<unsigned>(error.get_domain_status_code())));
        }
    }

    return j;
}

json serialize_to_json(const method_invocation_collection_document& invocations)
{
    WC_DEBUG_LOG("Serialize method invocations collection result.");

    json j;
    j.emplace("data", serialize_to_json(invocations.get_data()));
    if(invocations.has_links()) {
        j.emplace("links", invocations.get_links());
    }

    return j;
}

json serialize_to_json(const monitoring_list_document& monitoring_list)
{
    WC_DEBUG_LOG("Serialize monitor list result.");

    json j;
    if(!monitoring_list.has_errors()) {
        j.emplace("data",     serialize_to_json(monitoring_list.get_data()));
    }
    if(monitoring_list.has_links()) {
        j.emplace("links",                      monitoring_list.get_links());
    }
    if(monitoring_list.has_included_data<parameter_resource>() && !monitoring_list.has_errors()) {
        j.emplace("included", serialize_to_json(monitoring_list.get_included_data<parameter_resource>()));
    }
    if(monitoring_list.has_errors()) {
        j.emplace("errors",   serialize_to_json(monitoring_list.get_errors()));
    }

    return j;
}

json serialize_to_json(const monitoring_list_collection_document& monitoring_lists)
{
    WC_DEBUG_LOG("Serialize monitor list collection result.");

    json j;
    if(!monitoring_lists.has_errors()) {
        j.emplace("data",   serialize_to_json(monitoring_lists.get_data()));
    };
    if(monitoring_lists.has_links()) {
        j.emplace("links",                    monitoring_lists.get_links());
    }
    if(monitoring_lists.has_errors()) {
        j.emplace("errors", serialize_to_json(monitoring_lists.get_errors()));
    }

    return j;
}

json serialize_to_json(const monitoring_list_resource& monitoring_list)
{
    json j{
        {"id",         monitoring_list.get_id()},
        {"type",       monitoring_list.get_type()},
        {"links",      monitoring_list.get_links()},
        {"attributes", {
            {"timeout", monitoring_list.get_data().get_timeout() }
        }},
        {"relationships", serialize_to_json(monitoring_list.get_relationships())}
    };

    return j;
}

json serialize_to_json(const enum_definition_document& enum_definition)
{
    WC_DEBUG_LOG("Serialize enum definition result.");

    json j;
    if(enum_definition.has_errors())
    {
        j.emplace("errors", serialize_to_json(enum_definition.get_errors()));
    } 
    else
    {
        j.emplace("data",   serialize_to_json(enum_definition.get_data()));
    }
    if(enum_definition.has_links()) {
        j.emplace("links",                    enum_definition.get_links());
    }

    return j;
}

json serialize_to_json(const enum_definition_collection_document& enum_definitions)
{
    WC_DEBUG_LOG("Serialize enum definition collection result.");

    json j;
    if(enum_definitions.has_errors()) 
    {
        j.emplace("errors", serialize_to_json(enum_definitions.get_errors()));
    }
    else
    {
        j.emplace("data",   serialize_to_json(enum_definitions.get_data()));
    }
    if(enum_definitions.has_links()) 
    {
        j.emplace("links",                    enum_definitions.get_links());
    }

    return j;
}

json serialize_to_json(const enum_definition_resource& enum_definition)
{
    json j{
        {"id",         enum_definition.get_id()},
        {"type",       enum_definition.get_type()},
        {"links",      enum_definition.get_links()},
        {"attributes", {
            {"name", enum_definition.get_data().definition->name },
            {"cases", serialize_to_json(enum_definition.get_data().definition->members) }
        }}
    };
    if(enum_definition.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(enum_definition.get_relationships()));
    }

    return j;
}

json serialize_to_json(const wdx::enum_member& enum_definition_case)
{
    json j{
        {"value",       enum_definition_case.id},
        {"stringValue", enum_definition_case.name}
    };

    return j;
}

json serialize_to_json(const feature_document& feature_definition)
{
    WC_DEBUG_LOG("Serialize feature result.");

    json j;
    //FIXME: Handling for non-debug build required?
    WC_ASSERT(feature_definition.has_errors() == false);
    j.emplace("data",   serialize_to_json(feature_definition.get_data()));
    if(feature_definition.has_links())
    {
        j.emplace("links",                    feature_definition.get_links());
    }
    return j;
}

json serialize_to_json(const feature_collection_document& feature_definitions)
{
    WC_DEBUG_LOG("Serialize feature collection result.");

    json j;
    WC_ASSERT(feature_definitions.has_errors() == false);
    j.emplace("data",   serialize_to_json(feature_definitions.get_data()));
    if(feature_definitions.has_links())
    {
        j.emplace("links",                    feature_definitions.get_links());
    }

    return j;
  }

json serialize_to_json(const feature_resource& feature_definition)
{
    json j{
        {"id",         feature_definition.get_id()},
        {"type",       feature_definition.get_type()},
        {"links",      feature_definition.get_links()},
        {"attributes", {
            {"name", feature_definition.get_data().feature.name }
        }}
    };
    if(feature_definition.get_data().feature.is_beta)
    {
        j.at("attributes").emplace("beta", true);
    }
    if(feature_definition.get_data().feature.is_deprecated)
    {
        j.at("attributes").emplace("deprecated", true);
    }
    if(feature_definition.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(feature_definition.get_relationships()));
    }

    return j;
}

json serialize_to_json(const method_definition_document& method_def)
{
    WC_DEBUG_LOG("Serialize method definition result.");

    json j;
    //FIXME: Handling for non-debug build required?
    WC_ASSERT(method_def.has_errors() == false);
    j.emplace("data",   serialize_to_json(method_def.get_data()));
    if(method_def.has_links())
    {
        j.emplace("links", method_def.get_links());
    }

    return j;
}

json serialize_to_json(const method_definition_collection_document& method_defs)
{
    WC_DEBUG_LOG("Serialize method definition collection result.");

    json j;
    WC_ASSERT(method_defs.has_errors() == false);
    j.emplace("data", serialize_to_json(method_defs.get_data()));
    if(method_defs.has_links())
    {
        j.emplace("links", method_defs.get_links());
    }

    return j;
  }

json serialize_to_json(const method_definition_resource& method_def)
{
    json j{
        {"id",         method_def.get_id()},
        {"type",       method_def.get_type()},
        {"links",      method_def.get_links()},
        {"attributes", {
            {"path", method_def.get_data().get_path() }
        }}
    };

    // FIXE: missing implementation in core to get this model information
    // 
    // add status codes defined for the method if available
    // if(method_def.get_data().definition->???)
    // {
    //     j.at("attributes").emplace("domainSpecificStatusCodes", ???);
    // }

    // add beta/deprecated attribute if a method is declared as beta/deprecated
    if(method_def.get_data().get_method_definition()->is_beta)
    {
        j.at("attributes").emplace("beta", true);
    }
    if(method_def.get_data().get_method_definition()->is_deprecated)
    {
        j.at("attributes").emplace("deprecated", true);
    }

    if(method_def.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(method_def.get_relationships()));
    }

    return j;
}

json serialize_to_json(const method_arg_definition_document& arg_def)
{
    WC_DEBUG_LOG("Serialize method argument definition result.");

    json j;
    //FIXME: Handling for non-debug build required?
    WC_ASSERT(arg_def.has_errors() == false);
    j.emplace("data",   serialize_to_json(arg_def.get_data()));
    if(arg_def.has_links())
    {
        j.emplace("links", arg_def.get_links());
    }

    return j;
}

json serialize_to_json(const method_arg_definition_collection_document& arg_defs)
{
    WC_DEBUG_LOG("Serialize method argument definition collection result.");

    json j;
    WC_ASSERT(arg_defs.has_errors() == false);
    j.emplace("data", serialize_to_json(arg_defs.get_data()));

    if(arg_defs.has_links())
    {
        j.emplace("links", arg_defs.get_links());
    }

    return j;
}

json serialize_to_json(const method_arg_definition_resource& arg_def)
{
    auto const definition = arg_def.get_data().get_argument_definition();
    json j{
        {"id",         arg_def.get_id()},
        {"type",       arg_def.get_type()},
        {"links",      arg_def.get_links()},
        {"attributes", {
            {"name",                        definition->name},
            {"dataType", wda_ipc::to_string(definition->value_type)},
            {"dataRank", wda_ipc::to_string(definition->value_rank)}
        }}
    };
    if(arg_def.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(arg_def.get_relationships()));
    }

    return j;
}

json serialize_to_json(const parameter_definition_document& parameter_def)
{
    WC_DEBUG_LOG("Serialize parameter definition result.");

    json j;
    //FIXME: Handling for non-debug build required?
    WC_ASSERT(parameter_def.has_errors() == false);
    j.emplace("data",   serialize_to_json(parameter_def.get_data()));
    if(parameter_def.has_links())
    {
        j.emplace("links", parameter_def.get_links());
    }

    return j;
}

json serialize_to_json(const parameter_definition_collection_document& parameter_defs)
{
    WC_DEBUG_LOG("Serialize parameter definition collection result.");

    json j;
    WC_ASSERT(parameter_defs.has_errors() == false);
    j.emplace("data", serialize_to_json(parameter_defs.get_data()));
    if(parameter_defs.has_links())
    {
        j.emplace("links",parameter_defs.get_links());
    }

    return j;
  }

json serialize_to_json(const parameter_definition_resource& parameter_def)
{
    auto const data = parameter_def.get_data();
    bool const effective_writeable =      data.get_definition()->writeable
                                     && !(data.get_definition()->overrideables.inactive)
                                     && !(data.get_definition()->instance_key);
    json j{
        {"id",         parameter_def.get_id()},
        {"type",       parameter_def.get_type()},
        {"links",      parameter_def.get_links()},
        {"attributes", {
            {"path",                           data.get_associated_parameter().parameter_path},
            {"userSetting",                    data.get_definition()->user_setting},
            {"writeable",                      effective_writeable},
            {"dataType",    wda_ipc::to_string(data.get_definition()->value_type)},
            {"dataRank",    wda_ipc::to_string(data.get_definition()->value_rank)}
        }}
    };

    // FIXE: missing implementation in core to get this model information
    //
    // add status codes defined for the method if available
    // if(method_def.get_data().definition->???)
    // {
    //     j.at("attributes").emplace("domainSpecificStatusCodes", ???);
    // }

    // add beta/deprecated attribute if a parameter is declared as beta/deprecated
    if(parameter_def.get_data().get_definition()->is_beta)
    {
        j.at("attributes").emplace("beta", true);
    }
    if(parameter_def.get_data().get_definition()->is_deprecated)
    {
        j.at("attributes").emplace("deprecated", true);
    }

    if(parameter_def.has_relationships())
    {
        j.emplace("relationships", serialize_to_json(parameter_def.get_relationships()));
    }

    return j;
}

json serialize_to_json(const class_instance_document& class_instance)
{
    WC_DEBUG_LOG("Serialize class instance result.");

    json j;
    WC_ASSERT(class_instance.has_errors() == false);

    j.emplace("data", serialize_to_json(class_instance.get_data()));
    if(class_instance.has_links())
    {
        j.emplace("links", class_instance.get_links());
    }

    return j;
}

json serialize_to_json(const class_instance_collection_document& class_instances)
{
    WC_DEBUG_LOG("Serialize class instance collection result.");

    json j;
    WC_ASSERT(class_instances.has_errors() == false);
    j.emplace("data", serialize_to_json(class_instances.get_data()));
    if(class_instances.has_links())
    {
        j.emplace("links",class_instances.get_links());
    }

    return j;
}

json serialize_to_json(const class_instance_resource& class_instance)
{
    auto const data = class_instance.get_data();
    if (data.is_dangling()) {
        return nullptr;
    }
    json j{
        {"id",       class_instance.get_id()},
        {"type",     class_instance.get_type()},
        {"links",    class_instance.get_links()},
        {"attributes", {
            {"path",       data.get_path()},
            {"instanceNo", data.get_instance_no()}
        }},
        {"relationships", serialize_to_json(class_instance.get_relationships())},
    };

    return j;
}

//------------------------------------------------------------------------------
// type specific deserializer functions
//------------------------------------------------------------------------------
void deserialize_from_json(json const &j, std::vector<wdx::value_path_request> &value)
{
    for(json json_element : j)
    {
        wdx::value_path_request req(wdx::parameter_instance_path(), wdx::parameter_value::create(false));
        deserialize_from_json(json_element, req);
        value.push_back(req);
    }
}

void deserialize_from_json(json const &j, wdx::value_path_request &value)
{
    wdx::parameter_instance_path instance_path;
    if(j.contains("id"))
    {
        try
        {
            instance_path = wda_ipc::from_string<wdx::parameter_instance_path>(j.at("id").get<std::string>());
        }
        catch(...)
        {
            throw wda::rest::http_exception("Malformed parameter id.", wda::http::http_status_code::bad_request);
        }
    }
    std::shared_ptr<wdx::parameter_value> instance_value;
    deserialize_from_json(j.at("attributes"), instance_value);
    value = wdx::value_path_request(instance_path, instance_value);
}

void deserialize_from_json(json const &j, create_monitoring_list_request &value)
{
    if(!j.contains("relationships") || !j.at("relationships").contains("parameters"))
    {
        throw http::http_exception("Missing \"parameters\" relationship.", http_status_code::bad_request);
    }
    if(!j.contains("relationships") || !j.at("relationships").at("parameters").contains("data"))
    {
        throw http::http_exception("Missing \"data\" in \"parameters\" relationship.", http_status_code::bad_request);
    }
    std::vector<wdx::parameter_instance_path> instances;
    std::vector<shared_ptr<data_error>>       errors;
    size_t index = 0;
    for(auto instance_json : j.at("relationships").at("parameters").at("data"))
    {
        auto parameter_instance_id_string = instance_json.at("id").get<std::string>();
        wdx::parameter_instance_path id;
        try
        {
            id = wda_ipc::from_string<wdx::parameter_instance_path>(parameter_instance_id_string);
            instances.push_back(id);
        }
        catch(...)
        {
            std::string error_message = "Could not parse malformed parameter instance ID \"" + parameter_instance_id_string + "\".";
            errors.push_back(std::make_shared<relationship_error>(core_status_code::unknown_parameter_id, 
                                                                  error_message, 0, 
                                                                  "parameter",   "",
                                                                  -1, "parameters",  index));
        }
        ++index;
    }
    if(!errors.empty())
    {
        throw data_exception("Malformed parameter instance ID.", errors);
    }

    if(!j.contains("attributes") || !j.at("attributes").contains("timeout"))
    {
        throw http::http_exception("Missing \"timeout\" attribute.", http_status_code::bad_request);
    }
    uint16_t timeout_value;
    try {
        // json .get<uint16_t>() does not check any ranges and does not generate a parsing error (e.g. for -1)
        std::string const timeout_value_string = j.at("attributes").at("timeout").dump();
        size_t conversion_pos;
        auto timeout_value_raw = stoul(timeout_value_string, &conversion_pos);
        if(conversion_pos != timeout_value_string.length())
        {
            throw std::logic_error("Unexpected value format");
        }
        if(timeout_value_raw > UINT16_MAX)
        {
            throw std::out_of_range("Value overflow");
        }
        timeout_value = static_cast<uint16_t>(timeout_value_raw);
    }
    catch(...)
    {
        throw http::http_exception("Attribute \"timeout\" is not in range (0) - " WC_SUBST_STR(UINT16_MAX) ".",
                                   http_status_code::bad_request);
    }
    value = {
        timeout_value,
        instances
    };
}

void deserialize_from_json(json const &j, std::map<std::string, std::shared_ptr<wdx::parameter_value>> &new_map)
{
    WC_DEBUG_LOG("Deserialize method arguments");
    for(auto &element : j.items())
    {
        std::shared_ptr<wdx::parameter_value> value;
        deserialize_from_json(element.value(), value);
        wc_log(log_level_t::debug, "Deserialized method argument \"" + element.key() + "\"");
        new_map.emplace(element.key(), value);
    }
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
static string concat_json_path(vector<string> const &path);
template<class T>
static void value_from_json_path(json j, vector<string> path, T &value);
static bool has_value_on_json_path(json j, vector<string> path);
static json parse_json(string const &deserializable);

template <typename T>
void serialize_internal(json& out, const T& serializable)
{
    out = serialize_to_json(serializable);
    out.emplace("jsonapi", json::object({ {"version", "1.0"} }) );
    out.emplace("meta", json::object({ {"version", REST_API_VERSION} }) );
}

template <typename T>
void serialize_document(string& out, const basic_document<T>& serializable)
{
    json out_json;
    serialize_internal(out_json, serializable);
    auto additional_meta = serializable.get_meta();
    for(auto const &meta : serializable.get_meta())
    {
        if(!meta.second.empty())
        {
            out_json.at("meta").emplace(meta.first, meta.second);
        }
    }
    out = out_json.dump();
}

template <typename T>
void serialize_document(string& out, const collection_document<T>& serializable)
{
    json out_json;
    serialize_internal(out_json, serializable);
    auto additional_meta = serializable.get_meta();
    for(auto const &meta : serializable.get_meta())
    {
        if(!meta.second.empty())
        {
            out_json.at("meta").emplace(meta.first, meta.second);
        }
    }
    out = out_json.dump();
}

json_api::json_api()
:content_type_m("application/vnd.api+json")
{}

json_api::~json_api() noexcept = default;

string const & json_api::get_content_type() const
{
    return content_type_m;
}

void json_api::serialize(string                           &out,
                         shared_ptr<http_exception> const  error) const
{
    json out_json;
    serialize_internal(out_json, error);
    out = out_json.dump();
}

void json_api::serialize(string                 &out,
                         generic_document const &resource) const
{
    serialize_document(out, resource);
}

void json_api::serialize(string                            &out,
                         generic_collection_document const &resource_collection) const
{
    serialize_document(out, resource_collection);
}

void json_api::serialize(string                          &out,
                         service_identity_document const &service_identity) const
{
    serialize_document(out, service_identity);
}

void json_api::serialize(string                           &out,
                         device_collection_document const &device_responses) const
{
    serialize_document(out, device_responses);
}

void json_api::serialize(string                &out,
                         device_document const &resource) const
{
    serialize_document(out, resource);
}

void json_api::serialize(string                   &out,
                         parameter_document const &parameter_response) const
{
    serialize_document(out, parameter_response);
}

void json_api::serialize(string                              &out,
                         parameter_collection_document const &parameter_responses) const
{
    serialize_document(out, parameter_responses);
}

void json_api::serialize(string                       &out,
                         set_parameter_document const &parameter_response) const
{
    serialize_document(out, parameter_response);
}

void json_api::serialize(string                                  &out,
                         set_parameter_collection_document const &parameter_responses) const
{
    serialize_document(out, parameter_responses);
}

void json_api::serialize(string                &out,
                         method_document const &method_response) const
{
    serialize_document(out, method_response);
}

void json_api::serialize(string                           &out,
                         method_collection_document const &method_responses) const
{
    serialize_document(out, method_responses);
}

void json_api::serialize(string                           &out,
                         method_invocation_document const &method_invocation_response) const
{
    serialize_document(out, method_invocation_response);
}

void json_api::serialize(string                                      &out,
                         method_invocation_collection_document const &method_invocation_response) const
{
    serialize_document(out, method_invocation_response);
}

void json_api::serialize(string                         &out,
                         monitoring_list_document const &monitoring_list_response) const
{
    serialize_document(out, monitoring_list_response);
}

void json_api::serialize(string                                    &out,
                         monitoring_list_collection_document const &monitoring_lists_response) const
{
    serialize_document(out, monitoring_lists_response);
}

void json_api::serialize(string                         &out,
                         enum_definition_document const &enum_definition_response) const
{
    serialize_document(out, enum_definition_response);
}

void json_api::serialize(string                                    &out,
                         enum_definition_collection_document const &enum_definitions_response) const
{
    serialize_document(out, enum_definitions_response);
}

void json_api::serialize(string                 &out,
                         feature_document const &feature_response) const
{
    serialize_document(out, feature_response);
}

void json_api::serialize(string                            &out,
                         feature_collection_document const &features_response) const
{
    serialize_document(out, features_response);
}

void json_api::serialize(string                           &out,
                         method_definition_document const &method_def_response) const
{
    serialize_document(out, method_def_response);
}

void json_api::serialize(string                                      &out,
                         method_definition_collection_document const &method_def_responses) const
{
    serialize_document(out, method_def_responses);
}

void json_api::serialize(string                               &out,
                         method_arg_definition_document const &arg_def_response) const
{
    serialize_document(out, arg_def_response);
}

void json_api::serialize(string                                          &out,
                         method_arg_definition_collection_document const &arg_def_responses) const
{
    serialize_document(out, arg_def_responses);
}


void json_api::serialize(string                              &out,
                         parameter_definition_document const &parameter_definition_response) const
{
    serialize_document(out, parameter_definition_response);
}

void json_api::serialize(string                                         &out,
                         parameter_definition_collection_document const &parameter_definition_responses) const
{
    serialize_document(out, parameter_definition_responses);
}

void json_api::serialize(string                        &out,
                         class_instance_document const &class_instance_response) const
{
    serialize_document(out, class_instance_response);
}

void json_api::serialize(string                                   &out,
                         class_instance_collection_document const &class_instance_responses) const
{
    serialize_document(out, class_instance_responses);
}


template<class T>
static void value_from_json_path(json j, vector<string> path, T &value)
{
    json json_value;
    value_from_json_path(j, path, json_value);
    try
    {
        deserialize_from_json(json_value, value);
    }
    catch(json::exception const & e)
    {
        string message = "Failed to parse request content: wrong type at \""
                       + concat_json_path(path)
                       + "\".";
        throw http_exception(message, http_status_code::bad_request);
    }
}

bool has_value_on_json_path(json j, vector<string> path)
{
    json value = j;
    for(decltype(path.size()) i = 0; i < path.size(); ++i)
    {
        auto key = path.at(i);
        try
        {
            value = value.at(key);
        }
        catch(json::exception const &e)
        {
            return false;
        }
    }

    return true;
}

template<>
void value_from_json_path(json j, vector<string> path, json &value)
{
    value = j;
    for(decltype(path.size()) i = 0; i < path.size(); ++i)
    {
        auto key = path.at(i);
        try 
        {
            value = value.at(key);
        } 
        catch(json::exception const &e)
        {
            WC_ASSERT((i + 1) < static_cast<size_t>(std::numeric_limits<ssize_t>::max()));
            string message = "Failed to parse request content: expected a value at \"" 
                           + concat_json_path(vector<string>(path.begin(), path.begin() + static_cast<ssize_t>(i) + 1))
                           + "\".";
            throw http_exception(message, http_status_code::bad_request);
        }
    }
}

static json parse_json(string const &deserializable)
{
    try 
    {
        return json::parse(deserializable);
    }
    catch(json::exception const &e)
    {
        string message = string("Failed to parse request content as JSON.");
        throw http_exception(message, http_status_code::bad_request);
    }
}

void deserialize_from_json(json const &j, std::string &value)
{
    value = j.get<std::string>();
}

void deserialize_from_json(json const &j, std::shared_ptr<wdx::parameter_value> &value)
{
    // Check value object structure
    if(!j.is_object() || (!j.contains("value") && !j.contains("stringValue")))
    {
        std::string message = "Failed to parse value: must be at least an object with a \"value\" and/or \"stringValue\" property.";
        throw wda::rest::http_exception(message, wda::rest::http_status_code::bad_request);
    }

    // Parse value object
    std::shared_ptr<wdx::parameter_value> parsed_value = nullptr;
    wdx::parameter_value_rank  rank = wdx::parameter_value_rank::scalar;
    wdx::parameter_value_types type = wdx::parameter_value_types::unknown;
    if(j.contains("dataType")) {
        std::string data_type = j.at("dataType");
        // FIXME: we should be able to construct the enum by its string value!
    }
    if(j.contains("value")) {
        std::string raw_value = j.at("value").dump();
        parsed_value = wdx::parameter_value::create_with_json(type, rank, raw_value);
    }
    if(j.contains("stringValue")) {
        std::string type_str = wda_ipc::to_string(type);
        std::string str_value = j.at("stringValue");
        auto value_by_string = wdx::parameter_value::create_with_json(type, rank, str_value);
        if(parsed_value != nullptr) {
            // FIXME: we cannot to this equality check until we get function for that from the core!
            //if(*parsed_value == *value_by_string)
            if(false) //NOLINT (see above)
            {
                std::string message = "Failed to parse value: provided \"value\" property is not equal to provided \"stringValue\" property. "\
                                      "You must either provide equal values or only one of them.";
                throw wda::rest::http_exception(message, wda::rest::http_status_code::bad_request);
            }
        }
        parsed_value = value_by_string;
    }
    value = parsed_value;
}

void json_api::deserialize(method_invocation_in_args       &request,
                           string                    const &deserializable) const
{
    // FIXME: Make sure that no additional attributes and properties exist?
    json j = parse_json(deserializable);

    string type;
    value_from_json_path(j, {"data", "type"}, type);
    if(type != "runs")
    {
        string message = string("Failed to parse request content: unexpected value for property \"")
                       + "data/type"
                       + "\". Expected value was \"runs\".";
        throw http::http_exception(message, http_status_code::bad_request);
    }
    value_from_json_path(j, {"data", "attributes", "inArgs"}, request);
}

void json_api::deserialize(create_monitoring_list_request       &request,
                           string                         const &deserializable) const
{
    // FIXME: Make sure that no additional attributes and properties exist?
    json j = parse_json(deserializable);

    string type;
    value_from_json_path(j, {"data", "type"}, type);
    if(type != "monitoring-lists")
    {
        string message = string("Failed to parse request content: unexpected value for property \"")
                       + "data/type"
                       + "\". Expected value was \"monitoring-lists\".";
        throw http::http_exception(message, http_status_code::bad_request);
    }
    if(has_value_on_json_path(j, {"data", "id"}))
    {
        string message = string("Failed to parse request content: No ID element expected on \"")
                       + "data/id"
                       + "\". Server does not allow client generated IDs.";
        throw http::http_exception(message, http_status_code::forbidden);
    }
    value_from_json_path(j, {"data"}, request);
}

void json_api::deserialize(wdx::value_path_request       &parameter_value,
                           string                  const &deserializable) const
{
    // FIXME: Make sure that no additional attributes and properties exist?
    json j = parse_json(deserializable);

    string type;
    value_from_json_path(j, {"data", "type"}, type);
    if(type != "parameters")
    {
        string message = string("Failed to parse request content: unexpected value for property \"")
                       + "data/type"
                       + "\". Expected value was \"parameters\".";
        throw http::http_exception(message, http_status_code::bad_request);
    }
    value_from_json_path(j, {"data"}, parameter_value);
}

void json_api::deserialize(std::vector<wdx::value_path_request>       &parameter_values,
                           string                               const &deserializable) const
{
    json j = parse_json(deserializable);
    value_from_json_path(j, {"data"}, parameter_values);
    for(size_t i = 0; i < parameter_values.size(); ++i)
    {
        auto const &req = parameter_values.at(i);
        if(req.param_path == wdx::parameter_instance_path())
        {
            string message = string("Failed to parse request content: missing value for property \"") +
                             "data/" + std::to_string(i) + "/id\".";
            throw http::http_exception(message, http_status_code::bad_request);
        }
    }
}

static string concat_json_path(vector<string> const &path)
{
    string message;
    for(auto const &component : path)
    {
        if(!message.empty())
        {
            message += "/";
        }
        message += component;
    }
    return message;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
