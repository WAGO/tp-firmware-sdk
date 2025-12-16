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
///  \brief    Implementation of REST specific core error class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "core_error.hpp"
#include "method_response.hpp"

#include "wago/wdx/responses.hpp"
#include "wago/wdx/requests.hpp"
#include "wda_ipc/representation.hpp"
#include <wc/assertion.h>

#include <algorithm>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

// Define instantiations for each core response supported and evaluated by the REST API
template core_error::core_error(wdx::device_response const &);
template core_error::core_error(wdx::device_response const &,                  wdx::device_id const &);
template core_error::core_error(wdx::device_collection_response const &);
template core_error::core_error(wdx::device_collection_response const &,       wdx::device_id const &);
template core_error::core_error(wdx::parameter_response_list_response const &);
template core_error::core_error(wdx::parameter_response const &);
template core_error::core_error(wdx::parameter_response const &,               wdx::parameter_instance_path const &);
template core_error::core_error(wdx::set_parameter_response const &);
template core_error::core_error(wdx::set_parameter_response const &,           wdx::parameter_instance_path const &);
template core_error::core_error(method_response const &);
template core_error::core_error(method_response const &,                       wdx::parameter_instance_path const &);
template core_error::core_error(wdx::method_invocation_named_response const &);
template core_error::core_error(wdx::method_invocation_named_response const &, wdx::parameter_instance_path const &);
template core_error::core_error(wdx::monitoring_lists_response const &);
template core_error::core_error(wdx::monitoring_list_response const &);
template core_error::core_error(wdx::monitoring_list_values_response const &);
template core_error::core_error(wdx::delete_monitoring_list_response const &);
template core_error::core_error(wdx::enum_definition_response const &);
template core_error::core_error(wdx::enum_definition_response const &,         wdx::name_t const&);
template core_error::core_error(wdx::feature_response const &);
template core_error::core_error(wdx::feature_response const &,                 std::string const&);
template core_error::core_error(wdx::feature_list_response const &,            wdx::device_id const &);
template core_error::core_error(wdx::instance_list_response const &,           wdx::parameter_instance_path const &);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static std::string get_core_status_title(wdx::status_codes core_status);
static std::string extract_associated_type_description(wdx::status_codes status);

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
inline bool is_blank(char c)
{
    return (::isblank(c) != 0);
}

template <typename WdaResponseType>
std::string extract_resource_type_description(WdaResponseType const &)
{
    std::string resource_type;
    if (std::is_same<WdaResponseType, wdx::device_response>() ||
        std::is_same<WdaResponseType, wdx::feature_list_response>()) // Features always belong to a device
    {
        resource_type = "device";
    }
    else if (std::is_same<WdaResponseType, wdx::device_collection_response>())
    {
        resource_type = "device collection";
    }
    else if (std::is_same<WdaResponseType, wdx::parameter_response_list_response>())
    {
        resource_type = "parameters";
    }
    else if (std::is_same<WdaResponseType, wdx::parameter_response>() ||
             std::is_same<WdaResponseType, wdx::set_parameter_response>())
    {
        resource_type = "parameter";
    }
    else if (std::is_same<WdaResponseType, method_response>()||
             std::is_same<WdaResponseType, wdx::method_invocation_named_response>())
    {
        resource_type = "method";
    }
    else if (std::is_same<WdaResponseType, wdx::monitoring_lists_response>())
    {
        resource_type = "monitoring lists";
    }
    else if (std::is_same<WdaResponseType, wdx::monitoring_list_response>() ||
             std::is_same<WdaResponseType, wdx::monitoring_list_values_response>() ||
             std::is_same<WdaResponseType, wdx::delete_monitoring_list_response>())
    {
        resource_type = "monitoring list";
    }
    else if (std::is_same<WdaResponseType, wdx::enum_definition_response>())
    {
        resource_type = "enum definition";
    }
    else if (std::is_same<WdaResponseType, wdx::feature_response>())
    {
        resource_type = "feature";
    }
    else
    {
        WC_FAIL("Unhandled response type");
        resource_type = "resource";
    }
    return resource_type;
}

template <typename WdaRequest>
std::string extract_resource_id(WdaRequest const &request);

template <>
std::string extract_resource_id(wdx::parameter_instance_path const &request)
{
    return wda_ipc::to_string(request);
}

template <>
std::string extract_resource_id(wdx::device_id const &request)
{
    return wda_ipc::to_string(request);
}

template <>
std::string extract_resource_id(std::string const &request)
{
    std::string id = request;
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    return id;
}

template<typename WdaResponseType>
domain_status_code extract_domain_status_code(WdaResponseType const &)
{
    return 0;
}

template<>
domain_status_code extract_domain_status_code(wdx::method_invocation_named_response const &core_response)
{
    return core_response.domain_specific_status_code;
}

template<>
domain_status_code extract_domain_status_code(wdx::set_parameter_response const &core_response)
{
    return core_response.domain_specific_status_code;
}

template<>
domain_status_code extract_domain_status_code(wdx::parameter_response const &core_response)
{
    return core_response.domain_specific_status_code;
}

template<class WdaResponseType>
bool guess_if_request_error(WdaResponseType const &res)
{
    // we'll assume a request error by default
    bool guess_its_a_request_error = true;

    // only wdx::parameter_value or method argument related status codes 
    // are both used for request and server related errors.
    bool status_is_parameter_value_related = false;
    switch(res.status)
    {
        case wdx::status_codes::wrong_argument_count:
        case wdx::status_codes::missing_argument:
        case wdx::status_codes::wrong_out_argument_count:
        case wdx::status_codes::value_not_possible:
        case wdx::status_codes::value_null:
        case wdx::status_codes::invalid_value:
        case wdx::status_codes::other_invalid_value_in_set:
        case wdx::status_codes::wrong_value_type:
        case wdx::status_codes::wrong_value_representation:
        case wdx::status_codes::wrong_value_pattern:
            status_is_parameter_value_related = true;
            break;
        default:
            break;
    }

    // Exception 1: when reading parameters, the request cannot cause parameter_value related errors
    if (std::is_same<WdaResponseType, wdx::parameter_response>() && status_is_parameter_value_related)
    {
        guess_its_a_request_error = false;
    }

    return guess_its_a_request_error;
}

static std::string get_core_status_title(wdx::status_codes const core_status)
{
    std::string result;

    auto core_text = wdx::to_string(core_status);
    bool first = true;
    for(char c: core_text)
    {
        if(c == '_')
        {
            first = true;
            result += ' ';
        }
        else if(first)
        {
            result += static_cast<char>(::toupper(c));
            first = false;
        }
        else
        {
            result += static_cast<char>(::tolower(c));
        }
    }
    auto index = result.find("Id");
    if((index != std::string::npos) &&
       ((result[index+2] == '\0') || is_blank(result[index+2])))
    {
        result.replace(index, 2, "ID");
    }

    return result;
}

static std::string extract_associated_type_description(wdx::status_codes const status)
{
    std::string associated_type;
    switch(status)
    {
        case wdx::status_codes::unknown_device:
        case wdx::status_codes::unknown_device_collection:
        case wdx::status_codes::invalid_device_slot:
        case wdx::status_codes::invalid_device_collection:
            associated_type = "device";
            break;
        default:
            break;
    }

    return associated_type;
}

template <class WdaRequest>
std::string extract_device_id(WdaRequest const &)
{
    bool extract_device_id_called_for_unknown_request_type = false;
    WC_ASSERT(extract_device_id_called_for_unknown_request_type);
    return "";
}

template <>
std::string extract_device_id(wdx::device_id const &request)
{
    return wda_ipc::to_string(request);
}

template <>
std::string extract_device_id(wdx::parameter_instance_path const &request)
{
    // fixme: the device path is sometimes provided separated by '/' instead of '-' by the core api.
    auto id_str = request.device_path;
    std::replace(id_str.begin(), id_str.end(), '/', '-');
    return id_str;
}

template <>
std::string extract_device_id(wdx::value_path_request const &request)
{
    return extract_device_id(request.param_path);
}

template <class WdaRequest>
std::string extract_associated_id(WdaRequest        const &request,
                                  wdx::status_codes const  status)
{
    std::string associated_id;
    switch(status)
    {
        case wdx::status_codes::unknown_device:
        case wdx::status_codes::unknown_device_collection:
        case wdx::status_codes::invalid_device_slot:
        case wdx::status_codes::invalid_device_collection:
            associated_id = extract_device_id(request);
            break;

        default:
            break;
    }

    return associated_id;
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
template <class WdaResponse>
core_error::core_error(WdaResponse const &response)
: core_error(response.status,
             response.get_message(),
             extract_domain_status_code(response),
             extract_resource_type_description(response),
             "",
             guess_if_request_error(response),
             extract_associated_type_description(response.status),
             "")
{ }

template <class WdaResponse, class WdaRequest>
core_error::core_error(WdaResponse const &response,
                       WdaRequest  const &request)
: core_error(response.status,
             response.get_message(),
             extract_domain_status_code(response),
             extract_resource_type_description(response),
             extract_resource_id(request),
             guess_if_request_error(response),
             extract_associated_type_description(response.status),
             extract_associated_id(request, response.status))
{ }

core_error::core_error(wdx::status_codes core_status,
                       std::string       message,
                       uint16_t          domain_status,
                       std::string       resource_type,
                       std::string       resource_id,
                       bool              is_request_error,
                       std::string       associated_type,
                       std::string       associated_id)
: core_status_m(core_status)
, domain_status_m(domain_status)
{
    std::string core_status_text          = get_core_status_title(core_status);

    std::string object_with_id = resource_type + (resource_id.empty()
                                               ? ""
                                               : (" with " + std::string("ID") + " \"" + resource_id + "\""));

    std::string associated_object_with_id = ((associated_type == resource_type) || associated_type.empty() )
                                          ? ""
                                          : ("Associated " + associated_type + (associated_id.empty()
                                                                             ? ""
                                                                             : (" with ID \"" + associated_id + "\"")));

    // build error title and message based on the core status code
    switch(core_status)
    {
        // The success cases
        case wdx::status_codes::success:
            http_status_m = http::http_status_code::ok;
            break;

        case wdx::status_codes::status_value_unavailable:
            http_status_m = http::http_status_code::ok;
            title_m       = "Value unavailable";
            message_m     = "Value of " + object_with_id + " is currently unavailable.";
            break;

        case wdx::status_codes::wda_connection_changes_deferred:
            http_status_m = http::http_status_code::accepted;
            title_m       = "Changes will apply deferred";
            message_m     = object_with_id + " was checked successfully but would prevent"
                            " a response, therefore the value will be applied deferred."
                            " To verify if changes have been applied successfully, please read " + object_with_id +
                            " and check the value. Take into account that HTTP connection related parameters may"
                            " have changed.";
            break;

        case wdx::status_codes::success_but_value_adjusted:
            http_status_m = http::http_status_code::ok;
            title_m       = "Value has been accepted, but was adjusted";
            message_m     = object_with_id + " was written successfully but its value has been adjusted server-side."
                            " To check the applied value please read " + object_with_id + ".";
            break;

        // Request was not okay for some reason
        case wdx::status_codes::logic_error:
            http_status_m = http::http_status_code::bad_request;
            title_m       = "Wrong use of API for " + resource_type;
            message_m     = object_with_id + " is not usable in this way.";
            break;

        case wdx::status_codes::file_id_mismatch:
            http_status_m = http::http_status_code::bad_request;
            title_m       = "Failed to write " + resource_type;
            message_m     = object_with_id + " is not allowing the provided File-ID value. Another File-ID has been expected.";
            break;

        case wdx::status_codes::methods_do_not_have_value:
            http_status_m = http::http_status_code::bad_request;
            title_m       = "Failed to get " + resource_type;
            message_m     = "Object with id " + object_with_id + " is a method and does not have a value.";
            break;

        case wdx::status_codes::wrong_argument_count:
            if (is_request_error)
            {
                http_status_m = http::http_status_code::bad_request;
                title_m       = "Failed to run " + resource_type;
                message_m     = "Wrong number of arguments provided for " + object_with_id + ".";
            }
            else
            {
                http_status_m = http::http_status_code::internal_server_error;
                title_m       = "Error while running " + resource_type;
                message_m     = "Wrong number of arguments received from execution of " + object_with_id + ".";
            }
            break;

        case wdx::status_codes::missing_argument:
            if (is_request_error)
            {
                http_status_m = http::http_status_code::bad_request;
                title_m       = "Failed to run " + resource_type;
                message_m     = "No arguments provided for " + object_with_id + ".";
            }
            else
            {
                http_status_m = http::http_status_code::internal_server_error;
                title_m       = "Error while running " + resource_type;
                message_m     = "No arguments received from execution of " + object_with_id + ".";
            }
            break;

        case wdx::status_codes::wrong_out_argument_count:
            http_status_m = http::http_status_code::internal_server_error;
            title_m       = "Error while running " + resource_type;
            message_m     = "Wrong number of output arguments received from execution of " + object_with_id + ".";
            break;

        case wdx::status_codes::value_not_possible:
        case wdx::status_codes::value_null:
        case wdx::status_codes::invalid_value:
            if (is_request_error)
            {
                std::string verb = resource_type == "method" ? "run" : resource_type == "method run" ? "complete" : "write";
                http_status_m = http::http_status_code::bad_request;
                title_m       = "Failed to " + verb + " " + resource_type;
                message_m     = "Invalid value provided for " + object_with_id + "."
                                + (associated_object_with_id.empty()
                                ? ""
                                : (" (" + associated_object_with_id + ")"));
            }
            else
            {
                http_status_m = http::http_status_code::internal_server_error;
                title_m       = "Failed to read " + resource_type;
                message_m     = "Invalid value received from " + object_with_id + ".";
            }
            break;

        case wdx::status_codes::other_invalid_value_in_set:
            http_status_m = http::http_status_code::bad_request;
            title_m       = "Failed to write " + resource_type;
            message_m     = "Another invalid value prevents application of provided " + object_with_id + ".";
            break;

        case wdx::status_codes::wrong_value_type:
            if (is_request_error)
            {
                http_status_m = http::http_status_code::bad_request;
                if (resource_type == "method")
                {
                    title_m = "Failed to run " + resource_type;
                    message_m     = "Wrong value type of argument provided for " + object_with_id + ".";
                }
                else
                {
                    title_m = "Failed to write" + resource_type;
                    message_m     = "Wrong value type provided for" + object_with_id + ".";
                }
            }
            else
            {
                http_status_m = http::http_status_code::internal_server_error;
                if (resource_type == "method")
                {
                    title_m = "Error while running " + resource_type;
                    message_m     = "Wrong value type of argument received from " + object_with_id + ".";
                }
                else
                {
                    title_m = "Failed to read" + resource_type;
                    message_m     = "Wrong value type received for " + object_with_id + ".";
                }
            }
            break;

        case wdx::status_codes::wrong_value_representation:
            if (is_request_error)
            {
                http_status_m = http::http_status_code::bad_request;
                if (resource_type == "method")
                {
                    title_m = "Failed to run " + resource_type;
                    message_m     = "Wrong value representation of argument provided for " + object_with_id + ".";
                }
                else
                {
                    title_m = "Failed to write " + resource_type;
                    message_m     = "Wrong value representation provided for " + object_with_id + ".";
                }
            }
            else
            {
                http_status_m = http::http_status_code::internal_server_error;
                if (resource_type == "method")
                {
                    title_m = "Error while running " + resource_type;
                    message_m     = "Wrong value representation of argument received from " + object_with_id + ".";
                }
                else
                {
                    title_m = "Failed to read " + resource_type;
                    message_m     = "Wrong value representation received for " + object_with_id + ".";
                }
            }
            break;

        case wdx::status_codes::wrong_value_pattern:
            if (is_request_error)
            {
                http_status_m = http::http_status_code::bad_request;
                if (resource_type == "method")
                {
                    title_m = "Failed to run " + resource_type;
                    message_m     = "Wrong value pattern of argument provided for " + object_with_id + ".";
                }
                else
                {
                    title_m = "Failed to write " + resource_type;
                    message_m     = "Wrong value pattern provided for " + object_with_id + ".";
                }
            }
            else
            {
                http_status_m = http::http_status_code::internal_server_error;
                if (resource_type == "method")
                {
                    title_m = "Error while running " + resource_type;
                    message_m     = "Wrong value pattern of argument received from " + object_with_id + ".";
                }
                else
                {
                    title_m = "Failed to read " + resource_type;
                    message_m     = "Wrong value pattern received for " + object_with_id + ".";
                }
            }
            break;

        case wdx::status_codes::parameter_not_writeable: // FIXME: Check if HTTP 405 would be a better status code?
        case wdx::status_codes::instance_key_not_writeable:
        case wdx::status_codes::ignored:
            http_status_m = http::http_status_code::bad_request;
            title_m       = "Failed to write " + resource_type;
            message_m     = object_with_id + " is not writeable.";
            break;

        case wdx::status_codes::missing_parameter_for_instantiation:
        //case wdx::status_codes::not_existing_for_instance:
        http_status_m = http::http_status_code::bad_request;
            title_m       = "Failed to write " + resource_type;
            message_m     = "Instance of class " + object_with_id + " is missing a parameter.";
            break;

        case wdx::status_codes::not_existing_for_instance:
        http_status_m = http::http_status_code::bad_request;
            title_m       = "Failed to write " + resource_type;
            message_m     = "Parameter " + object_with_id + " is not existing for this class instance.";
            break;

        // A requested resource could not be found.
        // Client may change the request.
        case wdx::status_codes::unknown_device:
        case wdx::status_codes::unknown_parameter_id:
        case wdx::status_codes::unknown_parameter_path:
        case wdx::status_codes::unknown_class_instance_path:
        case wdx::status_codes::unknown_monitoring_list:
        case wdx::status_codes::invalid_device_collection:
        case wdx::status_codes::invalid_device_slot:
        case wdx::status_codes::not_a_method:
        case wdx::status_codes::unknown_enum_name:
        case wdx::status_codes::unknown_feature_name:
            http_status_m = http::http_status_code::not_found;
            title_m       = resource_type + " not found";
            message_m     = object_with_id + " does not exist."
                          + (associated_object_with_id.empty()
                          ? ""
                          : (" " + associated_object_with_id + " does not exist."));
            break;
        case wdx::status_codes::feature_not_available:
            http_status_m = http::http_status_code::not_found;
            title_m       = resource_type + " not found";
            message_m     = object_with_id + " is not available"
                          + (associated_object_with_id.empty()
                          ? "."
                          : (" for " + associated_object_with_id + "."));
            break;
        case wdx::status_codes::unknown_device_collection:
            http_status_m = http::http_status_code::not_found;
            title_m       = resource_type + " not found";
            if (resource_type == "device collection")
            {
                // TODO: mahe: there must be a better way to handle device collection resources
                //       maybe when device collections are refactored in the core library
                //       some day...
                associated_object_with_id.front() = static_cast<char>(::tolower(associated_object_with_id.front()));
                message_m = object_with_id + " does not exist"
                          + (associated_object_with_id.empty()
                          ? "."
                          : (" on " + associated_object_with_id + "."));
            }
            else
            {
                message_m = object_with_id + " does not exist."
                          + (associated_object_with_id.empty()
                          ? "" 
                          : (" " + associated_object_with_id + " does not exist."));
            }
            break;

        case wdx::status_codes::monitoring_list_max_exceeded:
            http_status_m = http::http_status_code::internal_server_error;
            title_m       = "Failed to create " + resource_type;
            message_m     = object_with_id + " could not be created. "
                            "The maximum number of monitoring lists has been reached. "
                            "You may want to wait until existing ones are removed either "
                            "by deleting them manually or by waiting for them to time out.";
            break;

        // A domain specific error can be set by parameter providers.
        // As we cannot tell whether it's the users or systems fault,
        // we'll set an internal sever error as the HTTP status.
        case wdx::status_codes::parameter_value_unavailable:
            http_status_m = http::http_status_code::internal_server_error;
            title_m       = "Failed to read " + resource_type;
            message_m     = "A domain specific error occurred on " + object_with_id + ".";
            break;
        case wdx::status_codes::could_not_set_parameter:
            http_status_m = http::http_status_code::internal_server_error;
            title_m       = "Failed to write " + resource_type;
            message_m     = "A domain specific error occurred on " + object_with_id + ".";
            break;
        case wdx::status_codes::could_not_invoke_method:
            title_m       = "Failed to complete " + resource_type;
            http_status_m = http::http_status_code::internal_server_error;
            message_m     = "A domain specific error occurred on " + object_with_id + "."
                            + (associated_object_with_id.empty()
                            ? ""
                            : (" " + associated_object_with_id + " could not be invoked."));
            break;

        // Something went wrong and the client cannot do anything about it.
        // Detailed error information is not available.
        case wdx::status_codes::internal_error:
        case wdx::status_codes::not_implemented:
        case wdx::status_codes::parameter_not_provided:
        case wdx::status_codes::provider_not_operational:
            http_status_m = http::http_status_code::internal_server_error;
            title_m       = "An internal error occurred";
            message_m     = "An internal error occurred on " + object_with_id + "."
                            + (associated_object_with_id.empty()
                            ? ""
                            : (" (" + associated_object_with_id + ")"));
            break;

        case wdx::status_codes::unauthorized:
            http_status_m = http::http_status_code::forbidden;
            title_m       = "Access denied";
            message_m     = (object_with_id + " is not accessible")
                          + (associated_object_with_id.empty() 
                            ? "."
                            : ("because " + associated_object_with_id + " is not accessible."));
            break;

        case wdx::status_codes::other_unauthorized_request_in_set:
            http_status_m = http::http_status_code::forbidden;
            title_m       = "Failed to write " + resource_type;
            message_m     = "Another unauthorized request prevents application of provided " + object_with_id + ".";
            break;

        // These errors should not occur on the frontend.
        // However, if they do: This is the fallback.
        case wdx::status_codes::no_error_yet:
        case wdx::status_codes::device_already_exists:
        case wdx::status_codes::device_description_inaccessible:
        case wdx::status_codes::device_description_parse_error:
        case wdx::status_codes::device_model_inaccessible:
        case wdx::status_codes::device_model_parse_error:
        case wdx::status_codes::unknown_include:
        case wdx::status_codes::ambiguous_base_path:
        case wdx::status_codes::parameter_already_provided:
        case wdx::status_codes::wdmm_version_not_supported:
        case wdx::status_codes::unknown_file_id:
        case wdx::status_codes::file_not_accessible:
        case wdx::status_codes::file_size_exceeded:
        case wdx::status_codes::not_a_file_id:
        case wdx::status_codes::upload_id_max_exceeded:
#ifdef NDEBUG
        // get warned in debug builds if not all core status codes are handled in this switch
        default:
#endif
            http_status_m = http::http_status_code::internal_server_error;
            title_m       = "An unexpected error occurred";
            message_m     = "An unexpected internal error occurred on " + object_with_id + ". This may be a bug.";
            break;
    }

    title_m.front() = static_cast<char>(::toupper(title_m.front()));
    if (!message.empty())
    {
        if ((message.back() != '.') && 
            (message.back() != '!') &&
            (message.back() != '?'))
        {
            message += ".";
        }
        message_m += " " + message;
    }
    message_m += " (" + core_status_text + ")";
    message_m.front() = static_cast<char>(::toupper(message_m.front()));
}

core_status_code core_error::get_core_status_code() const
{
    return core_status_m;
}

uint16_t core_error::get_domain_status_code() const
{
    return domain_status_m;
}

std::string core_error::get_message() const
{
    return message_m;
}

std::string core_error::get_title() const
{
    return title_m;
}

http::http_status_code core_error::get_http_status_code() const
{
    return http_status_m;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
