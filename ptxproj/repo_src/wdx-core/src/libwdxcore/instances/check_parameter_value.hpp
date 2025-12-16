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
#ifndef SRC_LIBWDXCORE_INSTANCES_CHECK_PARAMETER_VALUE_HPP_
#define SRC_LIBWDXCORE_INSTANCES_CHECK_PARAMETER_VALUE_HPP_

#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/wdmm/method_argument_definition.hpp"
#include "wago/wdx/parameter_exception.hpp"
#include "wago/wdx/parameter_value.hpp"
#include "wago/wdx/status_codes.hpp"
#include "utils/algo_helpers.hpp"
#include "utils/string_util.hpp"

#include <wc/log.h>

#include <memory>
#include <regex>
#include <exception>

namespace wago {
namespace wdx {

// FIXME: This file needs a refactoring:
// - Multiple overloads do not explain it's specific use case
// - Implementation is to heavy for inline / header only

inline status_codes check_parameter_value(parameter_value             &value,
                                          parameter_value_types const &definition_type,
                                          parameter_value_rank  const &definition_rank,
                                          std::string           const &pattern) // TODO: unify validation attributes for ParameterDefinition and ArgumentDefinition
{
    if(value.get_type() == parameter_value_types::unknown)
    {
        try
        {
            value.set_type_internal(definition_type, definition_rank);
        }
        catch(...)
        {
            return status_codes::wrong_value_representation;
        }
    }
    else if(value.get_type() != definition_type || (value.get_rank() != definition_rank))
    {
        return status_codes::wrong_value_type;
    }

    if(definition_rank == parameter_value_rank::array)
    {
        status_codes status = status_codes::no_error_yet;
        for(auto& item : value.get_items())
        {
            status = check_parameter_value(item, definition_type, parameter_value_rank::scalar, pattern);
            if(has_error(status)) break;
        }
        return status;
    }

    // check if ParameterValue can actually perform the native type Getter
    try
    {
        switch (value.get_type())
        {
        case parameter_value_types::string:
            value.get_string();
            break;
        case parameter_value_types::boolean:
            value.get_boolean();
            break;
        case parameter_value_types::float32:
            value.get_float32();
            break;
        case parameter_value_types::float64:
            value.get_float64();
            break;
        case parameter_value_types::bytes:
            value.get_bytes();
            break;
        case parameter_value_types::uint8:
            value.get_uint8();
            break;
        case parameter_value_types::uint16:
            value.get_uint16();
            break;
        case parameter_value_types::uint32:
            value.get_uint32();
            break;
        case parameter_value_types::uint64:
            value.get_uint64();
            break;
        case parameter_value_types::int8:
            value.get_int8();
            break;
        case parameter_value_types::int16:
            value.get_int16();
            break;
        case parameter_value_types::int32:
            value.get_int32();
            break;
        case parameter_value_types::int64:
            value.get_int64();
            break;
        case parameter_value_types::instance_ref:
            value.get_instance_ref();
            break;
        case parameter_value_types::instance_identity_ref:
            value.get_instance_identity_ref();
            break;
        case parameter_value_types::enum_member:
            value.get_enum_value();
            break;
        case parameter_value_types::file_id:
            value.get_file_id();
            break;
        case parameter_value_types::ipv4address:
            value.get_ipv4address();
            break;
        case parameter_value_types::instantiations:
            value.get_instantiations();
            break;

        default:
            break;
        }
    }
    catch(parameter_exception&)
    {
        return status_codes::wrong_value_representation;
    }
    catch(...)
    {
        return status_codes::internal_error;
    }

    if(!pattern.empty() && definition_type == parameter_value_types::string)
    {
        std::string val = value.get_string();
        auto regex = std::regex(pattern, std::regex_constants::ECMAScript);
        std::smatch m;
        if(!std::regex_match(val, m, regex))
        {
            return status_codes::wrong_value_pattern;
        }
    }
    if(definition_type == parameter_value_types::ipv4address)
    {
        auto val = value.get_ipv4address();
        auto regex = std::regex("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$", std::regex_constants::ECMAScript); // http://www.ipregex.com/
        std::smatch m;
        if(!std::regex_match(val, m, regex))
        {
            return status_codes::wrong_value_representation;
        }
    }

    return status_codes::no_error_yet;
}

inline status_codes check_parameter_value(std::shared_ptr<parameter_value> value,
                                          parameter_value_types            definition_type,
                                          parameter_value_rank             definition_rank,
                                          std::string                      pattern)
{
    if(!value)
    {
        return status_codes::value_null;
    }
    return check_parameter_value(*value, definition_type, definition_rank, pattern);
}

inline status_codes check_enum_value(parameter_value                      &value,
                                     std::weak_ptr<enum_definition> const &enum_def,
                                     parameter_value_rank                  rank)
{
    if(value.get_type() == parameter_value_types::enum_member)
    {
        if(rank == parameter_value_rank::array)
        {
            auto items = value.get_items();
            status_codes status = status_codes::no_error_yet;
            for(auto& item : items)
            {
                status = check_enum_value(item, enum_def, parameter_value_rank::scalar);
                if(has_error(status)) break;
            }
            return status;
        }

        if(enum_def.expired())
        {
            return status_codes::value_not_possible;
        }
        else
        {
            auto val = value.get_enum_value();
            if(!contains<enum_member>(enum_def.lock()->members, [=](const auto& member) { return member.id == val; }))
            {
                return status_codes::value_not_possible;
            }
        }
    }
    return status_codes::no_error_yet;
}

// TODO: Allow not only status code as result, but also error messages
inline status_codes check_parameter_value_special(parameter_value            &value,
                                                  parameter_definition const *definition,
                                                  parameter_value_rank const &rank)
{
    if(rank == parameter_value_rank::array)
    {
        auto items = value.get_items();
        if(definition->overrideables.allowed_length_set)
        {
            if(!definition->overrideables.allowed_length.is_valid(items.size()))
            {
                return status_codes::invalid_value;
            }
        }
        status_codes status = status_codes::no_error_yet;
        for(auto& item : items)
        {
            status = check_parameter_value_special(item, definition, parameter_value_rank::scalar);
            if(has_error(status)) break;
        }
        return status;
    }
    
    auto status = check_enum_value(value, definition->enum_def, parameter_value_rank::scalar);
    if(has_error(status))
    {
        return status;
    }
    if(definition->value_type == parameter_value_types::instance_ref)
    {
        if((definition->ref_classes_def.size() == 0) || definition->ref_classes_def.at(0).expired())
        {
            return status_codes::value_not_possible;
        }
        else
        {
            value.get_instance_ref();
        }
    }
    else if(definition->value_type == parameter_value_types::instance_identity_ref)
    {
        std::string const ref_to_check = value.get_instance_identity_ref();
        if(!ref_to_check.empty())
        {
            bool              base_path_allowed = false;
            size_t      const last_slash_pos    = ref_to_check.find_last_of("/");
            if(    (last_slash_pos == std::string::npos)            // No slash at all
                || (last_slash_pos == 0)                            // No base path
                || (last_slash_pos == (ref_to_check.length() - 1))) // No instance number
            {
                wc_log(log_level_t::notice, "Identity reference has invalid format\".");
                return status_codes::invalid_value;
            }

            std::string const ref_base_path = to_lower_copy(ref_to_check.substr(0, last_slash_pos));
            for(auto allowed_class : definition->ref_classes_def)
            {
                if(allowed_class.expired())
                {
                    continue;
                }
                if(to_lower_copy(allowed_class.lock()->base_path) == ref_base_path)
                {
                    base_path_allowed = true;
                    break;
                }
            }
            if(!base_path_allowed)
            {
                wc_log(log_level_t::notice, "Invalid identity reference: Base path \"" + ref_base_path +
                                            "\" is not allowed for reference \"" + definition->path + "\".");
                return status_codes::invalid_value;
            }

            std::string const ref_instance_no_raw = ref_to_check.substr(last_slash_pos + 1);
            try
            {
                size_t stoul_pos = 0;
                auto const ref_instance_no = std::stoul(ref_instance_no_raw, &stoul_pos);
                if(    (ref_instance_no > INSTANCE_ID_MAX)
                    || (stoul_pos != ref_instance_no_raw.length()))
                {
                    wc_log(log_level_t::notice, "Invalid identity reference: Instance number invalid.");
                    return status_codes::invalid_value;
                }
            }
            catch(const std::exception& e)
            {
                wc_log(log_level_t::notice, "Invalid identity reference: Instance number invalid.");
                return status_codes::invalid_value;
            }
        }
    }
    else if(definition->value_type == parameter_value_types::instantiations)
    {
        // This is the place where we could place additional checks for instantiations.
        // Currently we do not need any.
    }
    
    if(definition->overrideables.allowed_values_set)
    {
        if(!value.is_numeric_value_allowed(definition->overrideables.allowed_values))
        {
            return status_codes::invalid_value;
        }
    }
    return status_codes::no_error_yet;
}

inline status_codes check_parameter_value(std::shared_ptr<parameter_value>        value,
                                          parameter_definition             const *definition)
{
    auto status = check_parameter_value(value, definition->value_type, definition->value_rank, definition->overrideables.pattern);
    if(has_error(status))
    {
        return status;
    }
    return check_parameter_value_special(*value, definition, definition->value_rank);
}

inline status_codes check_parameter_value(std::shared_ptr<parameter_value>        value,
                                          method_argument_definition       const *definition)
{
    auto status = check_parameter_value(value, definition->value_type, definition->value_rank, "");
    if(has_error(status))
    {
        return status;
    }
    status = check_enum_value(*value, definition->enum_def, definition->value_rank);
    if(has_error(status))
    {
        return status;
    }
    return status_codes::no_error_yet;
}

}}

#endif // SRC_LIBWDXCORE_INSTANCES_CHECK_PARAMETER_VALUE_HPP_
