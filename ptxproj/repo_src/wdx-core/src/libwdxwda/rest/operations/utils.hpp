//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utilities used among operation implementations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_OPERATIONS_UTILS_HPP_
#define SRC_LIBWDXWDA_REST_OPERATIONS_UTILS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <rest/request.hpp>

#include "parameter_service_frontend_extended_i.hpp"
#include "wago/wdx/parameter_instance_path.hpp"

#include <wc/log.h>
#include <memory>
#include <limits>

namespace wago {
namespace wdx {
namespace wda {
namespace http {
enum class http_method: uint8_t;
} // Namespace http
namespace rest {

/// Parse a Parameter ID (parameter instance path) from a request's URI
/// \param req The requests which URI contains an instance path
/// \throws http_exception when the URI does not contain a valid instance path
/// \return The instance path used as a Parameter ID
wdx::parameter_instance_path const get_parameter_instance_id_from_request_url(std::shared_ptr<request> const req);

/// Parse a Method ID (parameter instance path) from a request's URI
/// \param req The requests which URI contains an instance path
/// \throws http_exception when the URI does not contain a valid instance path
/// \return The instance path used as a Method ID
wdx::parameter_instance_path const get_method_instance_id_from_request_url(std::shared_ptr<request> const req);

/// Parse Method definition ID (parameter instance path) from a request's URI
/// \param req The requests which URI contains an instance path
/// \throws http_exception when the URI does not contain a valid instance path
/// \return The instance path used as a Method definition ID
wdx::parameter_instance_path const get_method_definition_id_from_request_url(std::shared_ptr<request> const req);

/// Parse Parameter definition ID (parameter instance path) from a request's URI
/// \param req The requests which URI contains an instance path
/// \throws http_exception when the URI does not contain a valid instance path
/// \return The instance path used as a Parameter definition ID
wdx::parameter_instance_path const get_parameter_definition_id_from_request_url(std::shared_ptr<request> const req);

/// Parse an instance number (parameter instance path) from a request's URI
/// \param req The requests which URI contains an instance number
/// \throws http_exception when the URI does not contain a valid instance number
/// \return The instance number
wdx::instance_id_t get_instance_number_from_request_url(std::shared_ptr<request> const req);

/// Parse a Device ID and Feature ID (both strings) from a request's URI
/// \param req The requests which URI contains a feature ID
/// \param device The device ID
/// \param feature_name The feature name
void get_feature_id_parts_from_request_url(shared_ptr<request> const  req,
                                           std::string               &device,
                                           std::string               &feature_name);

/// Parse a Monitoring List ID from a request's URI
/// \param req The requests which URI contains a monitoring list ID
/// \throws http_exception when the URI does not contain a valid monitoring list ID
/// \return The monitoring list ID
monitoring_list_id_t get_monitoring_list_id_from_request_url(std::shared_ptr<request> const req);

/// Checks a response and throws any error contained in it.
/// \param response The response received from the core
/// \throws core_exception when the response contains an error
template<typename WdaResponseType, typename...WDARequestType>
void verify_core_response(WdaResponseType const &response, WDARequestType const &... request)
{
    if(response.has_error())
    {
        throw core_exception(core_error(response, request...));
    }
}

/// Check if an instance path is valid for the given response. If not, build
/// and throw a core_exception with relevant information.
/// \throws core_exception when the instance path is not considered valid
///                        for the given response.
void verify_instance_path(wdx::parameter_response      const &response,
                          wdx::parameter_instance_path const &instance_path,
                          bool                         const  method,
                          bool                         const  definition);

/// Check if an instance path is valid by asking the core frontend. 
/// If not, build and throw a core_exception with relevant information.
/// \throws core_exception when the instance path is not considered valid.
void verify_instance_path(wdx::parameter_service_frontend_extended_i       &core_frontend,
                          wdx::parameter_instance_path               const &path,
                          bool                                       const  method,
                          bool                                       const  definition);

/// Build a subset of a given response vector.
/// \return subset of responses
template<typename T>
vector<T> apply_paging(vector<T> const responses,
                       unsigned  const page_limit,
                       unsigned  const page_offset)
{
    wc_log_format(debug, "apply_paging to %u elements: limit=%u, offset=%u", responses.size(), page_limit, page_offset);
    vector<T> data_subset;
    if(!responses.empty())
    {
        if(    (static_cast<uint64_t>(page_limit) + static_cast<uint64_t>(page_offset) < static_cast<uint64_t>(page_offset))
            || (std::numeric_limits<ssize_t>::max() < (static_cast<uint64_t>(page_limit) + static_cast<uint64_t>(page_offset))))
        {
            const char* error_msg = "Internal error: paging out of possible range.";
            WC_FAIL(error_msg);
            throw http_exception(error_msg, http_status_code::internal_server_error);
        }
        auto first = responses.begin() + static_cast<ssize_t>(page_offset);
        if(first > responses.end())
        {
            first = responses.end();
        }
        auto last = responses.begin() + static_cast<ssize_t>(page_offset + page_limit);
        if(last > responses.end())
        {
            last = responses.end();
        }
        data_subset = vector<T>(first, last);
    }
    return data_subset;
}

/// Generate an error response describing the given error.
/// \param req The request which caused the error
/// \param error The error to be described by the response 
std::unique_ptr<response_i> get_error_response(std::shared_ptr<request>        const req,
                                               std::shared_ptr<http_exception>       error);

/// Generate a simple redirect response.
/// \param request_method The HTTP method of the request
/// \param request_uri The URI of the request
/// \param request_part The part of the URI to be replaced
/// \param replacement The replacement path
std::unique_ptr<response_i> simple_redirect(http_method const &request_method,
                                            std::string const &request_uri,
                                            std::string const &request_part,
                                            std::string const &replacement);

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_OPERATIONS_UTILS_HPP_
//---- End of source file ------------------------------------------------------
