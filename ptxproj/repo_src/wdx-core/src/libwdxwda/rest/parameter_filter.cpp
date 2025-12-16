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
///  \brief    Implementation for combined parameter filtering.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "parameter_filter.hpp"
#include "http/http_exception.hpp"

#include "wda_ipc/representation.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {
std::string create_query_error_message_for_invalid_value(std::string const &filter_name,
                                                         std::string const &detail_message);

std::string create_query_error_message_for_invalid_value(std::string                        const &filter_name,
                                                         std::initializer_list<std::string> const &allowed_values);
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
bool combine_with_beta_filter(wdx::parameter_filter       &filter,
                              std::string           const &required_filter_name,
                              std::string           const &filter_name,
                              std::string           const &filter_value)
{
    if(filter_name != required_filter_name)
    {
        return false;
    }

    if(filter_value == "true")
    {
        filter = filter | wdx::parameter_filter::only_beta();
    }
    else if(filter_value == "false")
    {
        filter = filter | wdx::parameter_filter::without_beta();
    }
    else
    {
        throw http::http_exception(create_query_error_message_for_invalid_value(filter_name, {"true", "false"}),
                                   http::http_status_code::bad_request);
    }
    return true;
}

bool combine_with_deprecated_filter(wdx::parameter_filter       &filter,
                                    std::string           const &required_filter_name,
                                    std::string           const &filter_name,
                                    std::string           const &filter_value)
{
    if(filter_name != required_filter_name)
    {
        return false;
    }

    if(filter_value == "true")
    {
        filter = filter | wdx::parameter_filter::only_deprecated();
    }
    else if(filter_value == "false")
    {
        filter = filter | wdx::parameter_filter::without_deprecated();
    }
    else
    {
        throw http::http_exception(create_query_error_message_for_invalid_value(filter_name, {"true", "false"}),
                                   http::http_status_code::bad_request);
    }
    return true;
}

bool combine_with_writeable_filter(wdx::parameter_filter       &filter,
                                   std::string           const &required_filter_name,
                                   std::string           const &filter_name,
                                   std::string           const &filter_value)
{
    if(filter_name != required_filter_name)
    {
        return false;
    }

    if(filter_value == "true")
    {
        filter = filter | wdx::parameter_filter::only_writeable();
    }
    else if(filter_value == "false")
    {
        filter = filter | wdx::parameter_filter::without_writeable();
    }
    else
    {
        throw http::http_exception(create_query_error_message_for_invalid_value(filter_name, {"true", "false"}),
                                   http::http_status_code::bad_request);
    }
    return true;
}

bool combine_with_userSetting_filter(wdx::parameter_filter       &filter,
                                     std::string           const &required_filter_name,
                                     std::string           const &filter_name,
                                     std::string           const &filter_value)
{
    if(filter_name != required_filter_name)
    {
        return false;
    }

    if(filter_value == "true")
    {
        filter = filter | wdx::parameter_filter::only_usersettings();
    }
    else if(filter_value == "false")
    {
        filter = filter | wdx::parameter_filter::without_usersettings();
    }
    else
    {
        throw http::http_exception(create_query_error_message_for_invalid_value(filter_name, {"true", "false"}),
                                   http::http_status_code::bad_request);
    }
    return true;
}

bool combine_with_device_filter(wdx::parameter_filter       &filter,
                                std::string           const &required_filter_name,
                                std::string           const &filter_name,
                                std::string           const &filter_value)
{
    if(filter_name != required_filter_name)
    {
        return false;
    }
    try {
        auto device_id = ((filter_value == "headstation")) ? wdx::device_id::headstation
                                                           : wda_ipc::from_string<wdx::device_id>(filter_value);
        filter = filter | wdx::parameter_filter::only_device(
            wdx::device_selector::specific(device_id)
        );
    }
    catch (std::exception const &e)
    {
        throw http::http_exception(create_query_error_message_for_invalid_value(filter_name, "Value must be a valid device ID."),
                                   http::http_status_code::bad_request);
    }
    return true;
}

bool combine_with_path_filter(wdx::parameter_filter       &filter,
                              std::string           const &required_filter_name,
                              std::string           const &filter_name,
                              std::string           const &filter_value)
{
    if(filter_name != required_filter_name)
    {
        return false;
    }
    try {
        filter = filter | wdx::parameter_filter::only_subpath(filter_value);
    }
    catch (std::exception const &e)
    {
        throw http::http_exception(create_query_error_message_for_invalid_value(filter_name, "Value must be a valid path."),
                                   http::http_status_code::bad_request);
    }
    return true;
}

wdx::parameter_filter extract_filters_from_query(std::map<std::string, filter_extractor> const &filter_extractors_by_filter_name,
                                                 std::map<std::string, std::string>      const &filter_query_parameters)
{
    wdx::parameter_filter filter;
    for(auto const &filter_query : filter_query_parameters)
    {
        bool matched_known_filter = false;
        for(auto const &extractor : filter_extractors_by_filter_name)
        {
            if(extractor.second(filter, extractor.first, filter_query.first, filter_query.second))
            {
                matched_known_filter = true;
                break;
            }
        }
        if(!matched_known_filter)
        {
            throw http::http_exception("Unknown filter option \"filter[" + filter_query.first + "]\".", http::http_status_code::bad_request);
        }
    }
    return filter;
}

namespace {
std::string create_query_error_message_for_invalid_value(std::string const &filter_name,
                                                         std::string const &detail_message)
{
    return "Invalid value for \"filter[" + filter_name + "]\"" + (detail_message.empty() ? "." : (": " + detail_message));
}

std::string create_query_error_message_for_invalid_value(std::string                        const &filter_name,
                                                         std::initializer_list<std::string> const &allowed_values)
{
    std::string allowed_values_listing;
    size_t i = 0;
    for(auto const &allowed_value : allowed_values)
    {
        if(i > 0)
        {
            allowed_values_listing += ((i == allowed_values.size() - 1) ? " or " : ", ");
        }
        allowed_values_listing += "\"" + allowed_value + "\"";
        ++i;
    }
    auto allowed_values_sentence = (allowed_values.size() > 0)
                                 ? ("Only " + allowed_values_listing + ( (allowed_values.size() > 1) ? " are" : " is") + " allowed.")
                                 : "";
    return create_query_error_message_for_invalid_value(filter_name, allowed_values_sentence);
}
} // Namespace anonymous (for internal functionality)


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
