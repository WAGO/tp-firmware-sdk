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
///  \brief    Definitions specific to the REST API used in several places.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_DEFINITIONS_HPP_
#define SRC_LIBWDXWDA_REST_DEFINITIONS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------

namespace wago {
namespace wdx {
namespace wda {
namespace rest {
//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define IDENTITY_ENDPOINT                                                   ""
#define DEVICE_ENDPOINT                                                     "/devices"
#define METHOD_ENDPOINT                                                     "/methods"
#define PARAMETER_ENDPOINT                                                  "/parameters"
#define MONITOR_LIST_ENDPOINT                                               "/monitoring-lists"
#define FEATURE_ENDPOINT                                                    "/features"
#define PARAMETER_DEFINITION_ENDPOINT                                       "/parameter-definitions"
#define METHOD_DEFINITION_ENDPOINT                                          "/method-definitions"
#define ENUM_DEFINITION_ENDPOINT                                            "/enum-definitions"
#define PAGINATION_PARAMETER_COMMON                                         "page["
#define FILTER_PARAMETER_COMMON                                             "filter["

static constexpr char const         wda_core_exception_text[]             = "Parameter service core error";

static constexpr char const         path_param_device_id[]                = "device_id";
static constexpr char const         path_param_collection_name[]          = "collection_name";
static constexpr char const         path_param_method_id[]                = "method_id";
static constexpr char const         path_param_method_run_id[]            = "method_run_id";
static constexpr char const         path_param_parameter_id[]             = "parameter_id";
static constexpr char const         path_param_monitoring_list_id[]       = "monitoring_list_id";
static constexpr char const         path_param_feature_id[]               = "feature_id";
static constexpr char const         path_param_feature_name[]             = "feature_name";
static constexpr char const         path_param_method_definition_id[]     = "method_definition_id";
static constexpr char const         path_param_method_inarg_name[]        = "method_inarg_name";
static constexpr char const         path_param_method_outarg_name[]       = "method_outarg_name";
static constexpr char const         path_param_parameter_definition_id[]  = "parameter_definition_id";
static constexpr char const         path_param_enum_definition_id[]       = "enum_definition_id";
static constexpr char const         path_param_instance_no[]              = "instance_no";

static constexpr char const         identity_endpoint[]                   = IDENTITY_ENDPOINT;
static constexpr char const         device_endpoint[]                     = DEVICE_ENDPOINT;
static constexpr char const         method_endpoint[]                     = METHOD_ENDPOINT;
static constexpr char const         parameter_endpoint[]                  = PARAMETER_ENDPOINT;
static constexpr char const         monitoring_list_endpoint[]            = MONITOR_LIST_ENDPOINT;
static constexpr char const         feature_endpoint[]                    = FEATURE_ENDPOINT;
static constexpr char const         parameter_definition_endpoint[]       = PARAMETER_DEFINITION_ENDPOINT;
static constexpr char const         method_definition_endpoint[]          = METHOD_DEFINITION_ENDPOINT;
static constexpr char const         enum_definition_endpoint[]            = ENUM_DEFINITION_ENDPOINT;
static constexpr char const         headstation_device[]                  = DEVICE_ENDPOINT "/0-0";
static constexpr char const         short_headstation[]                   = "/headstation";
static constexpr char const * const short_headstation_target              = headstation_device;

static constexpr char const         query_separator                       = '&';
static constexpr char const         page_common_start_base[]              = PAGINATION_PARAMETER_COMMON;
static constexpr char const         page_limit_query_key[]                = PAGINATION_PARAMETER_COMMON "limit]";
static constexpr char const         page_offset_query_key[]               = PAGINATION_PARAMETER_COMMON "offset]";
static constexpr char const         self_link_name[]                      = "self";
static constexpr char const         first_page_link_name[]                = "first";
static constexpr char const         last_page_link_name[]                 = "last";
static constexpr char const         next_page_link_name[]                 = "next";
static constexpr char const         previous_page_link_name[]             = "prev";

static constexpr unsigned           page_limit_default                    = 255;
static constexpr unsigned           page_offset_default                   =   0;

static constexpr char const         parameter_errors_as_data_attributes[] ="parameter-errors-as-data-attributes";
static constexpr char const         deferred_parameters_as_errors[]       = "deferred-parameters-as-errors";
static constexpr char const         adjusted_parameters_as_errors[]       = "adjusted-parameters-as-errors";
static constexpr char const         result_behavior[]                     = "result-behavior";

static constexpr char const         filter_beta[]                         = FILTER_PARAMETER_COMMON "beta]";
static constexpr char const         filter_deprecated[]                   = FILTER_PARAMETER_COMMON "deprecated]";
static constexpr char const         filter_device[]                       = FILTER_PARAMETER_COMMON "device]";

enum result_behavior_types
{
    any,
    automatic, // auto is a keyword in C++
    async,
    sync
};

#define JSON_API_CONTENT_TYPE_PART1                                         "application"
#define JSON_API_CONTENT_TYPE_PART2                                         "vnd.api+json"
static constexpr char const json_api_content_type_part1[]                 = JSON_API_CONTENT_TYPE_PART1;
static constexpr char const json_api_content_type_part2[]                 = JSON_API_CONTENT_TYPE_PART2;
static constexpr char const json_api_content_type[]                       = JSON_API_CONTENT_TYPE_PART1 "/" JSON_API_CONTENT_TYPE_PART2;


static constexpr char const cors_allowed_headers[] = "Accept, Authorization, Content-Length, Content-Type, Wago-Wdx-No-Auth-Popup";
static constexpr char const cors_exposed_headers[] = "Content-Length, Content-Type, Www-Authenticate, Wago-Wdx-Auth-Token, Wago-Wdx-Auth-Token-Expiration, Wago-Wdx-Auth-Token-Type";


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_DEFINITIONS_HPP_
//---- End of source file ------------------------------------------------------
