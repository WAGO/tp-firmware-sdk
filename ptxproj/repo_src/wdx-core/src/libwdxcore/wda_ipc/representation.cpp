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
#include <exception>
#include <limits>
#include <regex>

#include <nlohmann/json.hpp>
#include "representation.hpp"
#include "ipc.hpp"
#include "json_representation.hpp"
#include "utils/string_util.hpp"

using json = nlohmann::json;

namespace wago {
namespace wda_ipc {

using wago::wdx::device_id;
using wago::wdx::parameter_instance_id;
using wago::wdx::parameter_instance_path;
using wago::wdx::parameter_id_t;
using wago::wdx::instance_id_t;
using wago::wdx::class_instantiation;
using wago::wdx::parameter_value_types;
using wago::wdx::parameter_value_rank;


std::string to_string(device_id const &id)
{
    return std::to_string(id.device_collection_id) + "-" + std::to_string(id.slot);
}

std::string to_string(parameter_instance_id const &id)
{
    return std::to_string(id.device.device_collection_id) + "-" + std::to_string(id.device.slot) + "-" + to_string(id.id) + "-" + to_string(id.instance_id);
}

std::string to_string(parameter_instance_path const &path)
{
    return to_lower_copy(replace(path.device_path + "-" + path.parameter_path, "/", "-"));
}

std::string to_string(wago::wdx::parameter_id_t const &id)
{
    return std::to_string(id);
}

std::string to_string(wago::wdx::instance_id_t const &id)
{
    return std::to_string(id);
}

std::string to_string(wago::wdx::device_collection_id_t const &id)
{
    return std::to_string(id);
}

std::string to_string(wago::wdx::monitoring_list_id_t const &id)
{
    return std::to_string(id);
}

std::string to_string(wago::wdx::status_codes const &status)
{
    return std::to_string((uint16_t)status);
}

std::string to_string(wago::wdx::parameter_id_t const &id, wago::wdx::instance_id_t const &instance_id)
{
    return to_string(id) + "-" + to_string(instance_id);
}


std::string to_string(parameter_value_types const &t)
{
    switch (t)
    {
    case parameter_value_types::unknown:               return "unknown";
    case parameter_value_types::string:                return "string";
    case parameter_value_types::boolean:               return "boolean";
    case parameter_value_types::float32:               return "float32";
    case parameter_value_types::float64:               return "float64";
    case parameter_value_types::bytes:                 return "bytes";
    case parameter_value_types::uint8:                 return "uint8";
    case parameter_value_types::uint16:                return "uint16";
    case parameter_value_types::uint32:                return "uint32";
    case parameter_value_types::uint64:                return "uint64";
    case parameter_value_types::int8:                  return "int8";
    case parameter_value_types::int16:                 return "int16";
    case parameter_value_types::int32:                 return "int32";
    case parameter_value_types::int64:                 return "int64";
    case parameter_value_types::enum_member:           return "enum_member";
    case parameter_value_types::instance_ref:          return "instance_ref";
    case parameter_value_types::instance_identity_ref: return "instance_identity_ref";
    case parameter_value_types::method:                return "method";
    case parameter_value_types::file_id:               return "file_id";
    case parameter_value_types::ipv4address:           return "ipv4address";
    case parameter_value_types::instantiations:        return "instantiations";
    default:                                           return std::to_string((int)t);
    }
}

std::string to_string(parameter_value_rank const &rank)
{
    switch (rank)
    {
    case parameter_value_rank::scalar: return "scalar";
    case parameter_value_rank::array:  return "array";
    default:                           return std::to_string((int)rank);
    }
}

template <>
device_id from_string<device_id>(std::string const &s)
{
    auto fragments = split_string(s, '-');
    if(fragments.size() != 2)
    {
        throw std::runtime_error("Malformed string representing a device_id.");
    }
    int coll = 0;
    int slot = 0;
    if(!convert_to_int(fragments[0], coll))
    {
        throw std::runtime_error("Malformed string representing a device_id.");
    }
    if(!convert_to_int(fragments[1], slot))
    {
        throw std::runtime_error("Malformed string representing a device_id.");
    }
    if(slot < 0 || coll < 0)
    {
        throw std::runtime_error("String representing a device_id has parts outside of range.");
    }
    if(slot >= (1<<16) || coll >= (1<<8))
    {
        throw std::runtime_error("String representing a device_id has parts outside of range.");
    }
    return device_id(slot, coll);
}

wago::wdx::status_codes parse_device_collection_and_slot(std::string                       const &collectionStr,
                                                         std::string                       const &slotStr,
                                                         wago::wdx::device_collection_id_t       &coll,
                                                         wago::wdx::slot_index_t                 &slot)
{
    if(collectionStr.empty())
    {
        return wago::wdx::status_codes::invalid_device_collection;
    }
    int c = 0;
    if(!convert_to_int(collectionStr, c))
    {
        return wago::wdx::status_codes::invalid_device_collection;
    }
    if(c < 0 || c > std::numeric_limits<wago::wdx::device_collection_id_t>::max())
    {
        return wago::wdx::status_codes::invalid_device_collection;
    }
    coll = c;
    int s = 0;
    if(!convert_to_int(slotStr, s))
    {
        return wago::wdx::status_codes::invalid_device_slot;
    }
    if(s < 0 || s > std::numeric_limits<wago::wdx::slot_index_t>::max())
    {
        return wago::wdx::status_codes::invalid_device_slot;
    }
    slot = s;
    return wago::wdx::status_codes::success;
}

template <>
parameter_instance_id from_string<parameter_instance_id>(std::string const &s)
{
    auto fragments = split_string(s, '-');
    if(fragments.size() != 4)
    {
        throw std::runtime_error("Malformed string representing a parameter_instance_id.");
    }
    wago::wdx::device_collection_id_t coll = 0;
    wago::wdx::slot_index_t slot = 0;
    auto def = from_string<parameter_id_t>(fragments[2]);
    auto instance = from_string<instance_id_t>(fragments[3]);
    auto status = parse_device_collection_and_slot(fragments[0], fragments[1], coll, slot);
    if(has_error(status))
    {
        throw std::runtime_error("String representing a parameter_instance_id has invalid parts.");
    }
    return parameter_instance_id{def, instance, device_id(slot, coll)};
}

template <>
wago::wdx::parameter_instance_path from_string<wago::wdx::parameter_instance_path>(std::string const &s)
{
    auto regex = std::regex("^(([a-z|A-Z|0-9]+)-([0-9]+))-(.*)$", std::regex_constants::ECMAScript);
    std::smatch m;
    if(!std::regex_match(s, m, regex))
    {
        throw std::runtime_error("Malformed string representing a parameter_instance_path.");
    }
    if(m.size() != 5)
    {
        throw std::runtime_error("Malformed string representing a parameter_instance_path.");
    }
    wago::wdx::device_collection_id_t coll = 0;
    wago::wdx::slot_index_t slot = 0;
    auto status = parse_device_collection_and_slot(m.str(2), m.str(3), coll, slot);
    if(has_error(status))
    {
        throw std::runtime_error("String representing a parameter_instance_path has invalid parts.");
    }
    auto device = replace(m.str(1), "-", "/");
    auto parameter = replace(m.str(4), "-", "/");
    
    return wago::wdx::parameter_instance_path(parameter, device);
}

template <>
wago::wdx::parameter_id_t from_string<wago::wdx::parameter_id_t>(std::string const &s)
{
    if(s[0] == '-')
    {
        throw std::runtime_error("String representing a parameter_id_t is outside of range.");
    }
    uint32_t def = 0;
    if(!convert_to_uint32(s, def))
    {
        throw std::runtime_error("Malformed string representing a parameter_id_t.");
    }
    return def;
}

template <>
wago::wdx::instance_id_t from_string<wago::wdx::instance_id_t>(std::string const &s)
{
    int inst = 0;
    if(!convert_to_int(s, inst))
    {
        throw std::runtime_error("Malformed string representing an instance_id_t.");
    }
    if(inst < 0 || inst >= (1<<16))
    {
        throw std::runtime_error("String representing an instance_id_t is outside of range.");
    }
    return inst;
}

template <>
wago::wdx::device_collection_id_t from_string<wago::wdx::device_collection_id_t>(std::string const &s)
{
    int coll = 0;
    if(!convert_to_int(s, coll))
    {
        throw std::runtime_error("Malformed string representing a device_collection_id_t.");
    }
    if(coll < 0 || coll >= (1<<8))
    {
        throw std::runtime_error("String representing a device_collection_id_t is outside of range.");
    }
    return coll;
}

template <>
wago::wdx::monitoring_list_id_t from_string<wago::wdx::monitoring_list_id_t>(std::string const &s)
{
    if(s[0] == '-')
    {
        throw std::runtime_error("String representing a monitoring_list_id_t is outside of range.");
    }
    uint64_t def = 0;
    if(!convert_to_uint64(s, def))
    {
        throw std::runtime_error("Malformed string representing a monitoring_list_id_t.");
    }
    return def;
}

template <>
wago::wdx::status_codes from_string<wago::wdx::status_codes>(std::string const &s)
{
    uint16_t val = from_string<uint16_t>(s);
    return static_cast<wago::wdx::status_codes>(val); // parasoft-suppress CERT_CPP-INT50-a-3 "Explicit conversion from other representation"
}

template <>
wago::wdx::parameter_value_types from_string<wago::wdx::parameter_value_types>(std::string const &s)
{
    auto return_value = parameter_value_types::unknown;
    if(s == "string")                     { return_value = parameter_value_types::string; }
    else if(s == "boolean")               { return_value = parameter_value_types::boolean; }
    else if(s == "float32")               { return_value = parameter_value_types::float32; }
    else if(s == "float64")               { return_value = parameter_value_types::float64; }
    else if(s == "bytes")                 { return_value = parameter_value_types::bytes; }
    else if(s == "uint8")                 { return_value = parameter_value_types::uint8; }
    else if(s == "uint16")                { return_value = parameter_value_types::uint16; }
    else if(s == "uint32")                { return_value = parameter_value_types::uint32; }
    else if(s == "uint64")                { return_value = parameter_value_types::uint64; }
    else if(s == "int8")                  { return_value = parameter_value_types::int8; }
    else if(s == "int16")                 { return_value = parameter_value_types::int16; }
    else if(s == "int32")                 { return_value = parameter_value_types::int32; }
    else if(s == "int64")                 { return_value = parameter_value_types::int64; }
    else if(s == "enum_member")           { return_value = parameter_value_types::enum_member; }
    else if(s == "instance_ref")          { return_value = parameter_value_types::instance_ref; }
    else if(s == "instance_identity_ref") { return_value = parameter_value_types::instance_identity_ref; }
    else if(s == "method")                { return_value = parameter_value_types::method; }
    else if(s == "file_id")               { return_value = parameter_value_types::file_id; }
    else if(s == "ipv4address")           { return_value = parameter_value_types::ipv4address; }
    else if(s == "instantiations")        { return_value = parameter_value_types::instantiations; }
    return return_value;
}

template <>
wago::wdx::parameter_value_rank from_string<wago::wdx::parameter_value_rank>(std::string const &s)
{
    if(s == "array")
    {
        return parameter_value_rank::array;
    }
    else
    {
        return parameter_value_rank::scalar;
    }
}

template<typename T>
std::string to_ipc_string_impl(const T& thing)
{
    return json(thing).dump();
}

template<typename T>
T from_ipc_string_impl(const std::string& s)
{
    return json::parse(s).get<T>();
}

template<typename T>
std::vector<std::uint8_t> to_bytes_impl(const T& thing, serialization_method const &method)
{
    if(method == serialization_method::JSON)
    {
        auto s = json(thing).dump();
        return std::vector<std::uint8_t>(s.begin(), s.end());
    }
    else if(method == serialization_method::BINARY)
    {
        return json::to_cbor(json(thing));
    }
    else
    {
        return {};
    }
}

template<typename T>
T from_bytes_impl(const std::vector<std::uint8_t>& v, serialization_method const &method)
{
    if(method == serialization_method::JSON)
    {
        auto s = std::string(v.begin(), v.end());
        return json::parse(s).get<T>();
    }
    else if(method == serialization_method::BINARY)
    {
        return json::from_cbor(v);
    }
    else
    {
        T p{};
        return p;
    }
}

#define EXPLICIT_SERIALIZATION_IMPL(T) \
template <> \
std::vector<std::uint8_t> to_bytes<T>(const T& thing, serialization_method method) { \
    return to_bytes_impl(thing, method); \
} \
template<> \
T from_bytes<T>(const std::vector<std::uint8_t>& s, serialization_method method) { \
    return from_bytes_impl<T>(s, method); \
} \
template <> \
std::vector<std::uint8_t> to_bytes<std::vector<T>>(const std::vector<T>& thing, serialization_method method) { \
    return to_bytes_impl(thing, method); \
} \
template<> \
std::vector<T> from_bytes<std::vector<T>>(const std::vector<std::uint8_t>& s, serialization_method method) { \
    return from_bytes_impl<std::vector<T>>(s, method); \
} \
template <> \
std::string to_ipc_string<T>(const T& thing) { \
    return to_ipc_string_impl(thing); \
} \
template<> \
T from_ipc_string<T>(const std::string& s) { \
    return from_ipc_string_impl<T>(s); \
} \
template <> \
std::string to_ipc_string<std::vector<T>>(const std::vector<T>& thing) { \
    return to_ipc_string_impl(thing); \
} \
template<> \
std::vector<T> from_ipc_string<std::vector<T>>(const std::string& s) { \
    return from_ipc_string_impl<std::vector<T>>(s); \
}

EXPLICIT_SERIALIZATION_IMPL(wago::wdx::response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::class_instantiation)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::device_id)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::register_device_request)
EXPLICIT_SERIALIZATION_IMPL(std::uint8_t)
EXPLICIT_SERIALIZATION_IMPL(std::uint32_t)
EXPLICIT_SERIALIZATION_IMPL(std::uint64_t)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::file_id_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::file_info_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::file_read_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_selector)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::device_selector)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_selector_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_filter)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::value_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_instance_id)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_instance_path)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::set_parameter_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::value_request)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::value_path_request)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::method_invocation_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::method_invocation_named_response)
EXPLICIT_SERIALIZATION_IMPL(std::shared_ptr<wago::wdx::parameter_value>)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::device_selector_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::wdd_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::device_extension_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::wdm_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::device_collection_response)
EXPLICIT_SERIALIZATION_IMPL(wago::wdx::parameter_response_list_response)

#undef EXPLICIT_SERIALIZATION_IMPL

}
}
