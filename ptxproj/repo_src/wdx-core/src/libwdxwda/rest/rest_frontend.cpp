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
///  \brief    REST-API frontend for WAGO Parameter Service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "rest_frontend.hpp"
#include "wago/wdx/wda/http/request_i.hpp"
#include "wago/wdx/wda/settings_store_i.hpp"
#include "http/head_response.hpp"
#include "http/url_utils.hpp"
#include "operation.hpp"
#include "definitions.hpp"
#include "request.hpp"

#include <wc/log.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace {

string build_doc_link(string const &doc_link_base, string const &operation_id)
{
    return doc_link_base.empty() ? "" : (doc_link_base + "#operation/" + operation_id);
}

}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
rest_frontend::rest_frontend(string                                                   const &service_base,
                             string                                                   const &doc_link_base,
                             service_identity_i                                       const &service_identity,
                             shared_ptr<settings_store_i>                             const  settings_store,
                             unauthorized<wdx::parameter_service_frontend_extended_i> const  frontend)
: service_identity_m(service_identity)
, settings_store_m(settings_store)
, core_frontend_m(frontend)
, router_m(service_identity, service_base)
, run_manager_m(new run_object_manager())
{
    constexpr char const marker                = http::parameter_marker;
    string const param_device_id               = marker      + string(path_param_device_id)               + marker;
    string const param_collection_name         = marker      + string(path_param_collection_name)         + marker;
    string const param_method_id               = marker      + string(path_param_method_id)               + marker;
    string const param_method_run_id           = marker      + string(path_param_method_run_id)           + marker;
    string const param_parameter_id            = marker      + string(path_param_parameter_id)            + marker;
    string const param_monitoring_list_id      = marker      + string(path_param_monitoring_list_id)      + marker;
    string const param_feature_id              = marker      + string(path_param_feature_id)              + marker;
    string const param_feature_name            = marker      + string(path_param_feature_name)            + marker;
    string const param_method_definition_id    = marker      + string(path_param_method_definition_id)    + marker;
    string const param_method_inarg_name       = marker      + string(path_param_method_inarg_name)       + marker;
    string const param_method_outarg_name      = marker      + string(path_param_method_outarg_name)      + marker;
    string const param_parameter_definition_id = marker      + string(path_param_parameter_definition_id) + marker;
    string const param_enum_id                 = marker      + string(path_param_enum_definition_id)      + marker;
    string const param_instance_no             = marker      + string(path_param_instance_no)             + marker;

    WC_DEBUG_LOG("Configure routes...");

    //  Headstation redirect: "/headstation/..."
    router_m.add_redirect(string(short_headstation),
                          &operation::redirect_headstation);

    // Service identity: "/"
    router_m.add_route(http_method::get,
                       string(identity_endpoint),
                       &operation::get_service,
                       build_doc_link(doc_link_base, "getServiceIdentity"));

    // Device related: "/devices/..."
    router_m.add_route(http_method::get,
                       string(device_endpoint),
                       &operation::get_all_devices,
                       build_doc_link(doc_link_base, "getDevices"));
#if ENABLE_SUBDEVICES
    router_m.add_route(http_method::get,
                       string(device_endpoint) + "/" + param_device_id + "/subdevices",
                       &operation::get_subdevices_object,
                       build_doc_link(doc_link_base, "getSubdeviceCollections"));
    router_m.add_route(http_method::get,
                       string(device_endpoint) + "/" + param_device_id + "/subdevices/" + param_collection_name,
                       &operation::get_subdevices_by_collection_name,
                       build_doc_link(doc_link_base, "getSubdevices"));
#endif
    router_m.add_route(http_method::get,
                       string(device_endpoint) + "/" + param_device_id,
                       &operation::get_device,
                       build_doc_link(doc_link_base, "getDevice"));
    router_m.add_route(http_method::get,
                       string(device_endpoint) + "/" + param_device_id + "/features",
                       &operation::get_features_of_device,
                       build_doc_link(doc_link_base, "getFeatureList"));
    router_m.add_redirect(string(device_endpoint) + "/" + param_device_id + "/features/" + param_feature_name,
                       &operation::redirect_feature);

    // Method related: "/methods/..."
    router_m.add_route(http_method::get,
                       string(method_endpoint),
                       &operation::get_methods,
                       build_doc_link(doc_link_base, "getMethods"));
    router_m.add_route(http_method::get,
                       string(method_endpoint) + "/" + param_method_id,
                       &operation::get_method,
                       build_doc_link(doc_link_base, "getMethod"));
    router_m.add_route(http_method::post,
                       string(method_endpoint) + "/" + param_method_id + "/runs",
                       &operation::post_method_run,
                       build_doc_link(doc_link_base, "createMethodRun"));
    router_m.add_route(http_method::get,
                       string(method_endpoint) + "/" + param_method_id + "/runs",
                       &operation::get_method_runs,
                       build_doc_link(doc_link_base, "getMethodRuns"));
    router_m.add_route(http_method::get,
                       string(method_endpoint) + "/" + param_method_id + "/runs/" + param_method_run_id,
                       &operation::get_method_run,
                       build_doc_link(doc_link_base, "getMethodRun"));
    router_m.add_route(http_method::delete_,
                       string(method_endpoint) + "/" + param_method_id + "/runs/" + param_method_run_id,
                       &operation::delete_method_run,
                       build_doc_link(doc_link_base, "deleteMethodRun"));
    router_m.add_redirect(string(method_endpoint) + "/" + param_method_id + "/definition",
                       &operation::redirect_method_definition);
    router_m.add_redirect(string(method_endpoint) + "/" + param_method_id + "/device",
                       &operation::redirect_method_device);

    // Method definition related: "/method-definitions/..."
    router_m.add_route(http_method::get,
                       string(method_definition_endpoint),
                       &operation::get_all_method_definitions,
                       build_doc_link(doc_link_base, "getMethodDefinitions"));
    router_m.add_route(http_method::get,
                       string(method_definition_endpoint) + "/" + param_method_definition_id,
                       &operation::get_method_definition,
                       build_doc_link(doc_link_base, "getMethodDefinition"));
    router_m.add_route(http_method::get,
                       string(method_definition_endpoint) + "/" + param_method_definition_id + "/inargs",
                       &operation::get_all_method_inarg_definitions,
                       build_doc_link(doc_link_base, "getMethodInArgDefinitions"));
    router_m.add_route(http_method::get,
                       string(method_definition_endpoint) + "/" + param_method_definition_id + "/inargs/" + param_method_inarg_name,
                       &operation::get_method_inarg_definition,
                       build_doc_link(doc_link_base, "getMethodInArgDefinition"));
    router_m.add_redirect(string(method_definition_endpoint) + "/" + param_method_definition_id + "/inargs/" + param_method_inarg_name + "/enum",
                       &operation::redirect_method_inarg_enum);
    router_m.add_route(http_method::get,
                       string(method_definition_endpoint) + "/" + param_method_definition_id + "/outargs",
                       &operation::get_all_method_outarg_definitions,
                       build_doc_link(doc_link_base, "getMethodOutArgDefinitions"));
    router_m.add_route(http_method::get,
                       string(method_definition_endpoint) + "/" + param_method_definition_id + "/outargs/" + param_method_outarg_name,
                       &operation::get_method_outarg_definition,
                       build_doc_link(doc_link_base, "getMethodOutArgDefinition"));
    router_m.add_redirect(string(method_definition_endpoint) + "/" + param_method_definition_id + "/outargs/" + param_method_outarg_name + "/enum",
                       &operation::redirect_method_outarg_enum);

    // Parameter related: "/parameters/..."
    router_m.add_route(http_method::get,
                       string(parameter_endpoint),
                       &operation::get_parameters,
                       build_doc_link(doc_link_base, "getParameters"));
    router_m.add_route(http_method::patch,
                       string(parameter_endpoint),
                       &operation::set_parameters,
                       build_doc_link(doc_link_base, "setParameters"));
    router_m.add_route(http_method::get,
                       string(parameter_endpoint) + "/" + param_parameter_id,
                       &operation::get_parameter,
                       build_doc_link(doc_link_base, "getParameter"));
    router_m.add_route(http_method::patch,
                       string(parameter_endpoint) + "/" + param_parameter_id,
                       &operation::set_parameter,
                       build_doc_link(doc_link_base, "setParameter"));
    router_m.add_route(http_method::get,
                       string(parameter_endpoint) + "/" + param_parameter_id + "/referencedinstances",
                       &operation::get_referenced_class_instances,
                       build_doc_link(doc_link_base, "getReferencedInstances"));
    router_m.add_redirect(string(parameter_endpoint) + "/" + param_parameter_id + "/definition",
                       &operation::redirect_parameter_definition);
    router_m.add_redirect(string(parameter_endpoint) + "/" + param_parameter_id + "/device",
                       &operation::redirect_parameter_device);

    // Parameter definition related: "/parameter-definitions/..."
    router_m.add_route(http_method::get,
                       string(parameter_definition_endpoint),
                       &operation::get_all_parameter_definitions,
                       build_doc_link(doc_link_base, "getParameterDefinitions"));
    router_m.add_route(http_method::get,
                       string(parameter_definition_endpoint) + "/" + param_parameter_definition_id,
                       &operation::get_parameter_definition,
                       build_doc_link(doc_link_base, "getParameterDefinitionByID"));
    router_m.add_redirect(string(parameter_definition_endpoint) + "/" + param_parameter_definition_id + "/enum",
                       &operation::redirect_parameter_enum);

    // Class Instances related
    router_m.add_route(http_method::get,
                       string(parameter_endpoint) + "/" + param_parameter_id + "/instances",
                       &operation::get_class_instances,
                       build_doc_link(doc_link_base, "getInstances"));
    router_m.add_route(http_method::get,
                       string(parameter_endpoint) + "/" + param_parameter_id + "/instances/" + param_instance_no,
                       &operation::get_class_instance,
                       build_doc_link(doc_link_base, "getInstance"));

    // Monitor list related: "/monitoring-lists/..."
    router_m.add_route(http_method::get,
                       string(monitoring_list_endpoint),
                       &operation::get_monitoring_lists,
                       build_doc_link(doc_link_base, "getMonitoringLists"));
    router_m.add_route(http_method::post,
                       string(monitoring_list_endpoint),
                       &operation::post_monitoring_list,
                       build_doc_link(doc_link_base, "createMonitoringList"));
    router_m.add_route(http_method::get,
                       string(monitoring_list_endpoint) + "/" + param_monitoring_list_id,
                       &operation::get_monitoring_list,
                       build_doc_link(doc_link_base, "getMonitoringList"));
    router_m.add_route(http_method::delete_,
                       string(monitoring_list_endpoint) + "/" + param_monitoring_list_id,
                       &operation::delete_monitoring_list,
                       build_doc_link(doc_link_base, "deleteMonitoringList"));
    router_m.add_route(http_method::get,
                       string(monitoring_list_endpoint) + "/" + param_monitoring_list_id + "/parameters",
                       &operation::get_monitoring_list_parameters,
                       build_doc_link(doc_link_base, "getMonitoringListParameters"));

    // Feature related
    router_m.add_route(http_method::get,
                       string(feature_endpoint),
                       &operation::get_all_features,
                       build_doc_link(doc_link_base, "getFeatures"));
    router_m.add_route(http_method::get,
                       string(feature_endpoint) + "/" + param_feature_id,
                       &operation::get_feature,
                       build_doc_link(doc_link_base, "getFeature"));
    router_m.add_route(http_method::get,
                       string(feature_endpoint) + "/" + param_feature_id + "/includedfeatures",
                       &operation::get_included_features,
                       build_doc_link(doc_link_base, "getIncludedFeature"));
    router_m.add_route(http_method::get,
                       string(feature_endpoint) + "/" + param_feature_id + "/containedparameters",
                       &operation::get_contained_parameters_of_feature,
                       build_doc_link(doc_link_base, "getContainedParametersOfFeature"));
    router_m.add_route(http_method::get,
                       string(feature_endpoint) + "/" + param_feature_id + "/containedmethods",
                       &operation::get_contained_methods_of_feature,
                       build_doc_link(doc_link_base, "getContainedMethodsOfFeature"));

    // Enum definition related
    router_m.add_route(http_method::get,
                       string(enum_definition_endpoint),
                       &operation::get_all_enum_definitions,
                       build_doc_link(doc_link_base, "getEnumDefinitions"));
    router_m.add_route(http_method::get,
                       string(enum_definition_endpoint) + "/" + param_enum_id,
                       &operation::get_enum_definition,
                       build_doc_link(doc_link_base, "getEnumDefinition"));

    WC_DEBUG_LOG("Route configuration done");
}

rest_frontend::~rest_frontend() noexcept = default;

uint16_t rest_frontend::read_run_result_timeout()
{
    char const * const option = settings_store_i::run_result_timeout;
    auto timeout_value = std::stoul(settings_store_m->get_setting(option));
    WC_ASSERT_RETURN(timeout_value <= UINT16_MAX, UINT16_MAX);
    return static_cast<uint16_t>(timeout_value);
}

void rest_frontend::handle(std::unique_ptr<request_i> request, auth::authentication_info auth_info) noexcept
{
    auto route = router_m.route(std::move(request));
    operation::settings op_settings = { route.allowed_methods_m, read_run_result_timeout() };
    operation op(service_identity_m, core_frontend_m.authorize( user_data(auth_info.user_name)), std::move(op_settings), run_manager_m);
    op.handle(route.handler_m, std::move(route.request_m));
}

std::string rest_frontend::get_unauthenticated_urls() const
{
    std::string result;

    char const * const option = settings_store_i::allow_unauthenticated_requests_for_scan_devices;
    std::string option_setting_scan_devices;
    option_setting_scan_devices = settings_store_m->get_setting(option);
    if(option_setting_scan_devices == "true")
    {
#if ENABLE_SUBDEVICES
        result = "/devices;/devices/:device_id:;/devices/:device_id:/subdevices;/devices/:device_id:/subdevices/:device_collection:";
#else
        result = "/devices;/devices/:device_id:";
#endif
    }
    else
    {
        WC_ASSERT(option_setting_scan_devices == "false");
    }

    return result;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
