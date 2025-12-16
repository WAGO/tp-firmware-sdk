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
///  \brief    Operation object specific to the rest frontend wrapping
///            FCGI request object.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_OPERATION_HPP_
#define SRC_LIBWDXWDA_REST_OPERATION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "request.hpp"
#include "run_object_manager.hpp"

#include "parameter_service_frontend_extended_i.hpp"

#include <memory>
#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

namespace http {
class response_i;
} // Namespace http

namespace rest {
using http::response_i;
using std::unique_ptr;
using std::function;

using frontend_i = wdx::parameter_service_frontend_extended_i;

class operation_i;

typedef future<unique_ptr<response_i>>(operation_handler_raw_t)(operation_i *, std::shared_ptr<request>);
typedef function<operation_handler_raw_t> operation_handler_t;

class operation_i
{
public:
    operation_i() = default;
    virtual ~operation_i() noexcept = default;

    virtual service_identity_i        const &get_service_identity()        const = 0;
    virtual frontend_i                      &get_service_frontend()        const = 0;
    virtual std::vector<http_method>  const &get_allowed_methods()         const = 0;
    virtual uint16_t                         get_run_result_timeout_span() const = 0;
    virtual shared_ptr<run_object_manager_i> get_run_manager()                   = 0;

    virtual void handle(operation_handler_t const  &handler, 
                        request                   &&req) noexcept = 0;

    virtual void set_deferred_handler(std::function<void()> handler) = 0;
};

class operation : public operation_i 
{
public:
    struct settings
    {
        std::vector<http_method> allowed_methods;
        uint16_t                 run_result_timeout_span;
    };

    operation(service_identity_i        const &identity,
              unique_ptr<frontend_i>           frontend,
              settings                       &&operation_settings,
              shared_ptr<run_object_manager_i> run_manager);
    ~operation() noexcept override = default;

    service_identity_i        const &get_service_identity()        const override;
    frontend_i                      &get_service_frontend()        const override;
    std::vector<http_method>  const &get_allowed_methods()         const override;
    uint16_t                         get_run_result_timeout_span() const override;
    shared_ptr<run_object_manager_i> get_run_manager()                   override;

    void handle(operation_handler_t const  &handler, 
                request                   &&req) noexcept override;

    void set_deferred_handler(std::function<void()> handler) override;

    // Operation handlers:
    static future<unique_ptr<response_i>> options(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_service(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_devices(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_device(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_subdevices_object(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_subdevices_by_collection_name(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_methods(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_method(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_method_runs(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_method_run(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> post_method_run(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> delete_method_run(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_method_definitions(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_method_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_method_inarg_definitions(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_method_inarg_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_method_outarg_definitions(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_method_outarg_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_parameters(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_parameter(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> set_parameters(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> set_parameter(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_parameter_definitions(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_parameter_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_referenced_class_instances(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_class_instances(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_class_instance(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_monitoring_lists(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> post_monitoring_list(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_monitoring_list(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> delete_monitoring_list(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_monitoring_list_parameters(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_features_of_device(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_features(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_feature(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_included_features(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_contained_parameters_of_feature(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_contained_methods_of_feature(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_all_enum_definitions(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_enum_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_method_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_method_device(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_method_inarg_enum(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_method_outarg_enum(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_parameter_definition(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_parameter_device(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_parameter_enum(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_headstation(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_feature(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_trailing_slash(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> redirect_to_lowercase(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> not_found(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> not_implemented(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> not_acceptable(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> not_supported_content_type(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> method_not_allowed(operation_i * operation, std::shared_ptr<request> req);
    static future<unique_ptr<response_i>> get_head(operation_i * operation, std::shared_ptr<request> req);

private:
    service_identity_i        const &service_identity_m;
    std::shared_ptr<frontend_i>      service_frontend_m;
    settings                         operation_settings_m;
    std::function<void()>            deferred_handler_m;
    shared_ptr<run_object_manager_i> run_manager_m;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_OPERATION_HPP_
//---- End of source file ------------------------------------------------------
