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
#include <algorithm>
#include <regex>
#include <nlohmann/json.hpp>
#include "wago/wdx/parameter_value.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "wda_ipc/json_representation.hpp"
#include "wda_ipc/representation.hpp"
#include <wc/base64.h>
#include "utils/algo_helpers.hpp"
#include "utils/string_util.hpp"

using json = nlohmann::json;
using namespace std;

namespace wago {
namespace wdx {

namespace {
constexpr char const instance_path_pattern[] = "^([A-Za-z][A-Za-z0-9]*/)+([0-9]+)$";
std::regex const instance_path_regex(instance_path_pattern, std::regex::optimize); // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Does only throw if pattern is invalid, but pattern is constant"

void validate_instance_path(std::string const &instance_path)
{
    std::smatch matches;
    std::regex_match(instance_path, matches, instance_path_regex);
    if(matches.size() < 2)
    {
        throw parameter_exception(status_codes::invalid_value, "Invalid path value.");
    }
    try
    {
        wda_ipc::from_string<wago::wdx::instance_id_t>(matches[matches.size() - 1]);
    }
    catch(...)
    {
        throw parameter_exception(status_codes::invalid_value, "Invalid instance ID.");
    }
}
}

parameter_value::parameter_value()
: m_value(std::monostate{})
{ }

parameter_value::parameter_value(string value, parameter_value_types string_type)
: m_type(string_type)
, m_rank(parameter_value_rank::scalar)
, m_value(std::move(value))
{ }

parameter_value::parameter_value(char const *value)
: m_type(parameter_value_types::string)
, m_rank(parameter_value_rank::scalar)
, m_value(std::string(value))
{ }

parameter_value::parameter_value(bool value)
: m_type(parameter_value_types::boolean)
, m_rank(parameter_value_rank::scalar)
, m_value(value)
{ }

parameter_value::parameter_value(double value, parameter_value_types type)
: m_type(type)
, m_rank(parameter_value_rank::scalar)
, m_value(value)
{ }

parameter_value::parameter_value(uint64_t value, parameter_value_types type)
: m_type(type)
, m_rank(parameter_value_rank::scalar)
, m_value(value)
{ }

parameter_value::parameter_value(bytes_t value)
: m_type(parameter_value_types::bytes)
, m_rank(parameter_value_rank::scalar)
, m_value(std::move(value))
{ }

parameter_value::parameter_value(vector<parameter_value> value, parameter_value_types type)
: m_type(type)
, m_rank(parameter_value_rank::array)
, m_value(std::move(value))
{
    // Validate after move by accessing the stored vector
    const auto& stored_vector = std::get<std::vector<parameter_value>>(m_value);
    for(auto const &v : stored_vector)
    {
        if(v.get_type() != type)
        {
            throw parameter_exception(status_codes::wrong_value_type, "Element data type not matching array data type.");
        }
    }
}

std::shared_ptr<parameter_value> parameter_value::create(string value)
{
    return make_shared<parameter_value>(std::move(value));
}

std::shared_ptr<parameter_value> parameter_value::create(char const *value)
{
    return make_shared<parameter_value>(value);
}

std::shared_ptr<parameter_value> parameter_value::create(bool value)
{
    return make_shared<parameter_value>(std::move(value));
}

std::shared_ptr<parameter_value> parameter_value::create_string(std::string value)
{
    return make_shared<parameter_value>(std::move(value), parameter_value_types::string);
}

std::shared_ptr<parameter_value> parameter_value::create_string(char const *value)
{
    return make_shared<parameter_value>(value, parameter_value_types::string);
}

std::shared_ptr<parameter_value> parameter_value::create_boolean(bool value)
{
    return make_shared<parameter_value>(std::move(value));
}

std::shared_ptr<parameter_value> parameter_value::create_float32(float value)
{
    return make_shared<parameter_value>(std::move(value), parameter_value_types::float32);
}

std::shared_ptr<parameter_value> parameter_value::create_float64(double value)
{
    return make_shared<parameter_value>(std::move(value), parameter_value_types::float64);
}

std::shared_ptr<parameter_value> parameter_value::create_uint8(uint8_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::uint8);
}

std::shared_ptr<parameter_value> parameter_value::create_uint16(uint16_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::uint16);
}

std::shared_ptr<parameter_value> parameter_value::create_uint32(uint32_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::uint32);
}

std::shared_ptr<parameter_value> parameter_value::create_uint64(uint64_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::uint64);
}

std::shared_ptr<parameter_value> parameter_value::create_int8(int8_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::int8);
}

std::shared_ptr<parameter_value> parameter_value::create_int16(int16_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::int16);
}

std::shared_ptr<parameter_value> parameter_value::create_int32(int32_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::int32);
}

std::shared_ptr<parameter_value> parameter_value::create_int64(int64_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::int64);
}

std::shared_ptr<parameter_value> parameter_value::create_file_id(file_id id)
{
    return make_shared<parameter_value>(std::move(id), parameter_value_types::file_id);
}

std::shared_ptr<parameter_value> parameter_value::create_instantiations(std::vector<class_instantiation> instantiations)
{
    auto val = make_shared<parameter_value>();
    val->m_json_value = json(std::move(instantiations)).dump();
    val->set_type_internal(parameter_value_types::instantiations, parameter_value_rank::scalar);
    return val;
}

std::shared_ptr<parameter_value> parameter_value::create_ipv4address(ipv4address address)
{
    return make_shared<parameter_value>(std::move(address), parameter_value_types::ipv4address);
}

std::shared_ptr<parameter_value> parameter_value::create_instance_ref(instance_id_t instance_id)
{
    return make_shared<parameter_value>(std::move((uint64_t)instance_id), parameter_value_types::instance_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_unset_instance_ref()
{
    return make_shared<parameter_value>(std::move((uint64_t)0), parameter_value_types::instance_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_instance_identity_ref(std::string instance_path)
{
    if(!instance_path.empty())
    {
        validate_instance_path(instance_path);
    }
    return make_shared<parameter_value>(std::move(instance_path), parameter_value_types::instance_identity_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_instance_identity_ref(std::string base_path, instance_id_t instance_id)
{
    if(base_path.empty())
    {
        return make_shared<parameter_value>("", parameter_value_types::instance_identity_ref);
    }
    if(base_path.back() == '/')
    {
        throw parameter_exception(status_codes::invalid_value, "Path value must not end with \"/\"."); 
    }
    return create_instance_identity_ref(std::move(base_path) + "/" + std::move(std::to_string(instance_id)));
}

std::shared_ptr<parameter_value> parameter_value::create_unset_instance_identity_ref()
{
    return make_shared<parameter_value>("", parameter_value_types::instance_identity_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_enum_value(uint16_t value)
{
    return make_shared<parameter_value>(std::move((uint64_t)value), parameter_value_types::enum_member);
}

std::shared_ptr<parameter_value> parameter_value::create_bytes(bytes_t value)
{
    return make_shared<parameter_value>(std::move(value));
}

std::shared_ptr<parameter_value> parameter_value::create_string_array(std::vector<std::string> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const &v : values) { arr.emplace_back(v); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::string);
}

std::shared_ptr<parameter_value> parameter_value::create_boolean_array(std::vector<bool> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back(v); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::boolean);
}

std::shared_ptr<parameter_value> parameter_value::create_float32_array(std::vector<float> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back(v, parameter_value_types::float32); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::float32);
}

std::shared_ptr<parameter_value> parameter_value::create_float64_array(std::vector<double> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back(v); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::float64);
}

std::shared_ptr<parameter_value> parameter_value::create_uint8_array(std::vector<uint8_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::uint8); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::uint8);
}

std::shared_ptr<parameter_value> parameter_value::create_uint16_array(std::vector<uint16_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::uint16); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::uint16);
}

std::shared_ptr<parameter_value> parameter_value::create_uint32_array(std::vector<uint32_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::uint32); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::uint32);
}
std::shared_ptr<parameter_value> parameter_value::create_uint64_array(std::vector<uint64_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::uint64); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::uint64);
}

std::shared_ptr<parameter_value> parameter_value::create_int8_array(std::vector<int8_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::int8); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::int8);
}

std::shared_ptr<parameter_value> parameter_value::create_int16_array(std::vector<int16_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::int16); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::int16);
}

std::shared_ptr<parameter_value> parameter_value::create_int32_array(std::vector<int32_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::int32); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::int32);
}

std::shared_ptr<parameter_value> parameter_value::create_int64_array(std::vector<int64_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::int64); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::int64);
}

std::shared_ptr<parameter_value> parameter_value::create_ipv4address_array(std::vector<ipv4address> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const &v : values) { arr.emplace_back(v, parameter_value_types::ipv4address); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::ipv4address);
}

std::shared_ptr<parameter_value> parameter_value::create_file_id_array(std::vector<file_id> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const &v : values) { arr.emplace_back(v, parameter_value_types::file_id); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::file_id);
}

std::shared_ptr<parameter_value> parameter_value::create_instance_ref_array(std::vector<instance_id_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::instance_ref); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::instance_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_instance_identity_ref_array(std::vector<std::string> const &instance_paths)
{
    vector<parameter_value> arr;
    for(auto const &instance_path: instance_paths)
    {
        if(!instance_path.empty())
        {
            validate_instance_path(instance_path);
        }
        arr.emplace_back(instance_path, parameter_value_types::instance_identity_ref);
    }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::instance_identity_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_instance_identity_ref_array(std::vector<std::pair<std::string,instance_id_t>> const &instances)
{
    vector<parameter_value> arr;
    for(auto const &v: instances)
    {
        std::string   const &base_path   = v.first;
        instance_id_t const &instance_id = v.second;
        if(base_path.empty())
        {
            arr.emplace_back("", parameter_value_types::instance_identity_ref);
        }
        else
        {
            if(base_path.back() == '/')
            {
                throw parameter_exception(status_codes::invalid_value, "Path value must not end with \"/\"."); 
            }
            std::string instance_path = base_path + "/" + std::to_string(instance_id);
            validate_instance_path(instance_path);
            arr.emplace_back(instance_path, parameter_value_types::instance_identity_ref);
        }
    }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::instance_identity_ref);
}

std::shared_ptr<parameter_value> parameter_value::create_enum_value_array(std::vector<uint16_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto v : values) { arr.emplace_back((uint64_t)v, parameter_value_types::enum_member); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::enum_member);
}

std::shared_ptr<parameter_value> parameter_value::create_bytes_array(std::vector<bytes_t> const &values)
{
    vector<parameter_value> arr;
    arr.reserve(values.size());
    for(auto const &v : values) { arr.emplace_back(v); }
    return make_shared<parameter_value>(std::move(arr), parameter_value_types::bytes);
}

std::shared_ptr<parameter_value> parameter_value::create_with_json(parameter_value_types type_,
                                                                   parameter_value_rank  rank_,
                                                                   json_string_t         value_)
{
    auto val = create_with_unknown_type(std::move(value_));
    val->set_type_internal(std::move(type_), std::move(rank_));
    return val;
}

std::shared_ptr<parameter_value> parameter_value::create_with_unknown_type(json_string_t value)
{
    parameter_value epv;
    epv.m_json_value = std::move(value);
    return make_shared<parameter_value>(epv);
}


parameter_value_types parameter_value::get_type() const
{
    return m_type;
}

parameter_value_rank parameter_value::get_rank() const
{
    return m_rank;
}

bool parameter_value::is_array() const
{
    return m_rank == parameter_value_rank::array;
}

string parameter_value::get_string() const
{
    check_type(parameter_value_types::string, parameter_value_rank::scalar);
    return std::get<std::string>(m_value);
}

bool parameter_value::get_boolean() const
{
    check_type(parameter_value_types::boolean, parameter_value_rank::scalar);
    return std::get<bool>(m_value);
}

float parameter_value::get_float32() const
{
    check_type(parameter_value_types::float32, parameter_value_rank::scalar);
    return static_cast<float>(std::get<double>(m_value));
}

double parameter_value::get_float64() const
{
    check_type(parameter_value_types::float64, parameter_value_rank::scalar);
    return std::get<double>(m_value);
}

uint8_t parameter_value::get_uint8() const
{
    check_type(parameter_value_types::uint8, parameter_value_rank::scalar);
    return static_cast<uint8_t>(std::get<uint64_t>(m_value));
}

uint16_t parameter_value::get_uint16() const
{
    check_type(parameter_value_types::uint16, parameter_value_rank::scalar);
    return static_cast<uint16_t>(std::get<uint64_t>(m_value));
}

uint32_t parameter_value::get_uint32() const
{
    check_type(parameter_value_types::uint32, parameter_value_rank::scalar);
    return static_cast<uint32_t>(std::get<uint64_t>(m_value));
}

uint64_t parameter_value::get_uint64() const
{
    check_type(parameter_value_types::uint64, parameter_value_rank::scalar);
    return std::get<uint64_t>(m_value);
}

int8_t parameter_value::get_int8() const
{
    check_type(parameter_value_types::int8, parameter_value_rank::scalar);
    return static_cast<int8_t>(std::get<uint64_t>(m_value));
}

int16_t parameter_value::get_int16() const
{
    check_type(parameter_value_types::int16, parameter_value_rank::scalar);
    return static_cast<int16_t>(std::get<uint64_t>(m_value));
}

int32_t parameter_value::get_int32() const
{
    check_type(parameter_value_types::int32, parameter_value_rank::scalar);
    return static_cast<int32_t>(std::get<uint64_t>(m_value));
}

int64_t parameter_value::get_int64() const
{
    check_type(parameter_value_types::int64, parameter_value_rank::scalar);
    return static_cast<int64_t>(std::get<uint64_t>(m_value));
}

file_id parameter_value::get_file_id() const
{
    check_type(parameter_value_types::file_id, parameter_value_rank::scalar);
    return std::get<std::string>(m_value);
}

std::vector<class_instantiation> parameter_value::get_instantiations() const
{
    check_type(parameter_value_types::instantiations, parameter_value_rank::scalar);
    try
    {
        auto result = json::parse(m_json_value).get<std::vector<class_instantiation>>();
        sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
        return result;
    }
    catch(json::parse_error const &)
    {
        throw parameter_exception(status_codes::wrong_value_type);
    }
}

ipv4address parameter_value::get_ipv4address() const
{
    check_type(parameter_value_types::ipv4address, parameter_value_rank::scalar);
    return std::get<std::string>(m_value);
}

instance_id_t parameter_value::get_instance_ref() const
{
    check_type(parameter_value_types::instance_ref, parameter_value_rank::scalar);
    return static_cast<instance_id_t>(std::get<uint64_t>(m_value));
}
bool parameter_value::is_unset_instance_ref() const
{
    check_type(parameter_value_types::instance_ref, parameter_value_rank::scalar);
    return std::get<uint64_t>(m_value) == 0;
}

std::string parameter_value::get_instance_identity_ref() const
{
    check_type(parameter_value_types::instance_identity_ref, parameter_value_rank::scalar);
    return std::get<std::string>(m_value);
}
bool parameter_value::is_unset_instance_identity_ref() const
{
    check_type(parameter_value_types::instance_identity_ref, parameter_value_rank::scalar);
    return std::get<std::string>(m_value).empty();
}

uint16_t parameter_value::get_enum_value() const
{
    check_type(parameter_value_types::enum_member, parameter_value_rank::scalar);
    return static_cast<uint16_t>(std::get<uint64_t>(m_value));
}

bytes_t parameter_value::get_bytes() const
{
    check_type(parameter_value_types::bytes, parameter_value_rank::scalar);
    return std::get<bytes_t>(m_value);
}

vector<parameter_value> parameter_value::get_items() const
{
    check_type(m_type, parameter_value_rank::array);
    return std::get<std::vector<parameter_value>>(m_value);
}

json_string_t parameter_value::create_json_result() const
{
    auto primitive_to_j = [](const parameter_value& v) {
        switch (v.m_type)
        {
        case parameter_value_types::boolean:
            return json(std::get<bool>(v.m_value));
        case parameter_value_types::float32:
            return json(static_cast<float>(std::get<double>(v.m_value)));
        case parameter_value_types::float64:
            return json(std::get<double>(v.m_value));
        case parameter_value_types::string:
        case parameter_value_types::file_id:
        case parameter_value_types::ipv4address:
        case parameter_value_types::instance_identity_ref:
            return json(std::get<std::string>(v.m_value));
        case parameter_value_types::uint8:
            return json(static_cast<uint8_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::uint16:
            return json(static_cast<uint16_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::uint32:
            return json(static_cast<uint32_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::uint64:
            return json(std::get<uint64_t>(v.m_value));
        case parameter_value_types::int8:
            return json(static_cast<int8_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::int16:
            return json(static_cast<int16_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::int32:
            return json(static_cast<int32_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::int64:
            return json(static_cast<int64_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::bytes:
        {
            const auto& bytes_value = std::get<bytes_t>(v.m_value);
            auto const expected_encoded_len = wc_base64_calc_encoded_length(bytes_value.size());
            if (expected_encoded_len == 0 && !bytes_value.empty())
            {
                throw std::runtime_error("Failed to encode to Base64 string: payload too large.");
            }
            char   encoded[expected_encoded_len];
            size_t encoded_len = 0;
            if (!wc_base64_encode(bytes_value.data(), bytes_value.size(), encoded, &encoded_len))
            {
                throw std::runtime_error("Failed to encode to Base64 string.");
            }
            return json(std::string(encoded, encoded_len));
        }
        case parameter_value_types::instance_ref:
            return json(static_cast<instance_id_t>(std::get<uint64_t>(v.m_value)));
        case parameter_value_types::enum_member:
            return json(static_cast<uint16_t>(std::get<uint64_t>(v.m_value)));

        default:
            return json("");
        }
    };

    json jsonResult;

    if (m_rank == parameter_value_rank::array)
    {
        jsonResult = json::array();
        const auto& pv_vector = std::get<std::vector<parameter_value>>(m_value);
        for (auto const &pv : pv_vector)
        {
            jsonResult += primitive_to_j(pv);
        }
    }
    else
    {
        if(m_type == parameter_value_types::instantiations || m_type == parameter_value_types::unknown)
        {
            return this->m_json_value;
        }
        jsonResult = primitive_to_j(*this);
    }
    return jsonResult.dump();
}

json_string_t parameter_value::get_json() const
{
    return create_json_result();
}

string parameter_value::to_string() const
{
    return create_json_result();
}

void parameter_value::set_type_internal(parameter_value_types const &type_, parameter_value_rank const &rank_)
{
    if(m_type != parameter_value_types::unknown)
    {
        return;
    }
    
    // Store new value temporarily - only update member variables if parsing succeeds
    auto new_type = type_;
    auto new_rank = rank_;
    decltype(m_value) new_value = std::monostate{};

    if(new_type == parameter_value_types::unknown)
    {
        m_type = new_type;
        m_rank = new_rank;
        return;
    }
    auto ensure_int = [](json& value) {
        if(!value.is_number() || !value.is_number_integer())
        {
            // nlohmann detects numbers > UInt64.MAX as float, but is_number_integer() is false.
            // A number > UInt64.MAX is therefore "not an Integer" instead of "out of range".
            throw parameter_exception(status_codes::invalid_value, "Value is not an integer."); 
        }
    };
    auto ensure_uint = [&ensure_int](json& value) {
        ensure_int(value);
        if(!value.is_number_unsigned())
        {
            throw parameter_exception(status_codes::invalid_value, "Value is not an unsigned integer.");
        }
    };
    auto ensure_bounds = [new_type](int64_t value, int64_t min, int64_t max_excl) {
        if(value < min || value >= max_excl)
        {
            throw parameter_exception(status_codes::invalid_value, "Value " + std::to_string(value) + " exceeds its bounds [" + std::to_string(min) + ", " + std::to_string(max_excl) + "). Scalar type is " + json(new_type).dump() + ".");
        }
    };

    if (!m_json_value.empty())
    {
        if(new_rank == parameter_value_rank::array)
        {
            json j = json::parse(m_json_value);
            std::vector<parameter_value> pv_vector;
            if(!j.is_array())
            {
                throw parameter_exception(status_codes::wrong_value_representation, "Value is not an array.");
            }
            for(auto& v : j)
            {
                parameter_value val;
                val.m_json_value = v.dump();
                val.set_type_internal(new_type, parameter_value_rank::scalar);
                pv_vector.push_back(val);
            }
            new_value = std::move(pv_vector);
        }
        else if(new_type == parameter_value_types::instantiations) {
            // parameter_value_types::instantiations is transported directly with json_value
            // No need to set new_value as it stays monostate for this type
        }
        else
        {
            json j = json::parse(m_json_value);

        switch (new_type)
        {
        case parameter_value_types::boolean:
            new_value = j.get<bool>();
            break;
        case parameter_value_types::float32:
        case parameter_value_types::float64:
            new_value = j.get<double>();
            break;
        case parameter_value_types::string:
        case parameter_value_types::file_id:
        case parameter_value_types::ipv4address:
            new_value = j.get<std::string>();
            break;
        case parameter_value_types::instance_identity_ref:
            {
                std::string string_value = j.get<std::string>();
                if(!string_value.empty())
                {
                    validate_instance_path(string_value);
                }
                new_value = std::move(string_value);
            }
            break;
        case parameter_value_types::uint8:
            ensure_uint(j);
            {
                uint64_t integer_value = j.get<int64_t>();
                ensure_bounds(integer_value, 0, 0x100);
                new_value = integer_value;
            }
            break;
        case parameter_value_types::uint16:
        case parameter_value_types::instance_ref:
        case parameter_value_types::enum_member:
            ensure_uint(j);
            {
                uint64_t integer_value = j.get<int64_t>();
                ensure_bounds(integer_value, 0, 0x10000);
                new_value = integer_value;
            }
            break;
        case parameter_value_types::uint32:
            ensure_uint(j);
            {
                uint64_t integer_value = j.get<int64_t>();
                ensure_bounds(integer_value, 0, 0x100000000ll);
                new_value = integer_value;
            }
            break;
        case parameter_value_types::uint64:
            ensure_uint(j); // this triggers an exception if bounds are exceeded, see comment in ensure_int().
            new_value = j.get<uint64_t>();
            break;
        case parameter_value_types::int8:
            ensure_int(j);
            {
                uint64_t integer_value = j.get<int64_t>();
                ensure_bounds(integer_value, -0x80, 0x80);
                new_value = integer_value;
            }
            break;
        case parameter_value_types::int16:
            ensure_int(j);
            {
                uint64_t integer_value = j.get<int64_t>();
                ensure_bounds(integer_value, -0x8000, 0x8000);
                new_value = integer_value;
            }
            break;
        case parameter_value_types::int32:
            ensure_int(j);
            {
                uint64_t integer_value = j.get<int64_t>();
                ensure_bounds(integer_value, -2147483648ll, 0x80000000ll);
                new_value = integer_value;
            }
            break;
        case parameter_value_types::int64: {
            ensure_int(j);
            int64_t num = 0;
            auto jd = j.dump();
            if(!convert_to_int64(jd, num))
            {
                throw parameter_exception(status_codes::invalid_value, "Value out of range.");
            }
            new_value = static_cast<uint64_t>(j.get<int64_t>());
            break;
        }
        case parameter_value_types::bytes:
        {
            auto const &encoded = j.get<string>();
            auto encoded_size = encoded.size();
            ensure_bounds(encoded_size, 0, 2*1024*1024); //allow max data size of 2MB
            uint8_t decoded[wc_base64_calc_decoded_length(encoded_size)];
            size_t  decoded_len = 0;
            if (!wc_base64_decode(encoded.data(), encoded.size(), decoded, &decoded_len))
            {
                throw std::runtime_error("Failed to decode Base64 string.");
            }
            new_value = bytes_t(decoded, decoded + decoded_len);
            break;
        }
        default:
            break;
        }
        }
    }
    
    // Only update member variables if parsing succeeded
    m_type = new_type;
    m_rank = new_rank;
    m_value = std::move(new_value);
}

void parameter_value::check_type(parameter_value_types type_, parameter_value_rank rank_) const
{
    if (m_type != type_ || m_rank != rank_)
    {
        throw parameter_exception(status_codes::wrong_value_type,
                                  "Wrong method for this data type, which is " + wago::wda_ipc::to_string(m_type)
                                  + (is_array() ? "(array)" : ""));
    }
}

bool parameter_value::is_numeric_value_allowed(allowed_values_spec const &spec)
{
    if(m_rank == parameter_value_rank::array)
    {
        for(auto& item : get_items())
        {
            if(!item.is_numeric_value_allowed(spec))
            {
                return false;
            }
        }
        return true;
    }

    int64_t number = 0;
    switch (m_type)
    {
    case parameter_value_types::uint8:
    case parameter_value_types::uint16:
    case parameter_value_types::uint32:
    case parameter_value_types::uint64:
    case parameter_value_types::int8:
    case parameter_value_types::int16:
    case parameter_value_types::int32:
    case parameter_value_types::int64:
    case parameter_value_types::instance_ref:
    case parameter_value_types::enum_member:
        number = static_cast<int64_t>(std::get<uint64_t>(m_value));
        break;
    case parameter_value_types::float32:
    case parameter_value_types::float64:
        number = static_cast<int64_t>(std::get<double>(m_value));
        break;
    
    default:
        return true; // spec is ignored for non_numeric types
    }

    return spec.is_valid(number);
}

bool operator==(parameter_value const &lhs, parameter_value const &rhs) noexcept
{
    if(&lhs == &rhs)
    {
        return true;
    }

    try
    {
        return lhs.get_type() == rhs.get_type() && lhs.get_rank() == rhs.get_rank() && lhs.get_json() == rhs.get_json();
    }
    catch(...)
    {
        return false;
    }
}

bool operator!=(parameter_value const &lhs, parameter_value const &rhs) noexcept
{
    return !(lhs == rhs);
}

}}
