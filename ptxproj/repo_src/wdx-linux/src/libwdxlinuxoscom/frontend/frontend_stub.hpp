//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Service frontend stub for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_STUB_HPP_
#define SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_stub.hpp"
#include "frontend_methods.hpp"

#include <wago/wdx/parameter_service_frontend_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class frontend_stub final : public abstract_stub<wdx::parameter_service_frontend_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(frontend_stub)

    std::unique_ptr<wdx::parameter_service_frontend_i> owned_target_m;

public:
    frontend_stub(managed_object_id                                         id,
                  std::string                                        const &connection_name,
                  sender_i                                                 &sender,
                  driver_i                                                 &driver,
                  std::unique_ptr<wdx::parameter_service_frontend_i>        target,
                  wc_trace_channels                                  const  channel)
    : abstract_stub(id, connection_name, sender, driver, *target, channel)
    , owned_target_m(std::move(target))
    { }

    ~frontend_stub() noexcept override = default;

private:
    void handle_call(method_id_type  method_id,
                     data_istream   &method_args) override
    {
        switch (method_id)
        {
            // Device requests
            case frontend_method_id::frontend_get_all_devices:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: get all devices");
                call(&wdx::parameter_service_frontend_i::get_all_devices, method_id, method_args);
                break;

            // Parameter read requests
            case frontend_method_id::frontend_get_all_parameters:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: get all parameters");
                call(&wdx::parameter_service_frontend_i::get_all_parameters, method_id, method_args);
                break;
            case frontend_method_id::frontend_get_parameters:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: get parameters");
                call(&wdx::parameter_service_frontend_i::get_parameters, method_id, method_args);
                break;
            case frontend_method_id::frontend_get_parameters_by_path:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: get parameters by path");
                call(&wdx::parameter_service_frontend_i::get_parameters_by_path, method_id, method_args);
                break;

            // Parameter write requests
            case frontend_method_id::frontend_set_parameter_values:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: set parameter values");
                call(&wdx::parameter_service_frontend_i::set_parameter_values, method_id, method_args);
                break;
            case frontend_method_id::frontend_set_parameter_values_by_path:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: set parameter values by path");
                call(&wdx::parameter_service_frontend_i::set_parameter_values_by_path, method_id, method_args);
                break;

            // Method invocations
            case frontend_method_id::frontend_invoke_method:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: invoke method");
                call(&wdx::parameter_service_frontend_i::invoke_method, method_id, method_args);
                break;
            case frontend_method_id::frontend_invoke_method_by_path:
                WC_TRACE_SET_MARKER(channel_m, "Com frontend stub: invoke method by path");
                call(&wdx::parameter_service_frontend_i::invoke_method_by_path, method_id, method_args);
                break;

            default:
                break;
            }
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_STUB_HPP_
//---- End of source file ------------------------------------------------------
