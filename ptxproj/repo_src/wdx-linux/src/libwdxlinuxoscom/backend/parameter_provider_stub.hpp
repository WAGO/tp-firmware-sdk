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
///  \brief    Parameter provider stub for IPC.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_BACKEND_PARAMETER_PROVIDER_STUB_HPP_
#define SRC_LIBWDXLINUXOSCOM_BACKEND_PARAMETER_PROVIDER_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_stub.hpp"
#include "backend_methods.hpp"

#include <wago/wdx/parameter_provider_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class parameter_provider_stub final : public abstract_stub<wdx::parameter_provider_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(parameter_provider_stub)

public:
    bool used_serial_mode_on_register = false;
    using abstract_stub::abstract_stub;
    ~parameter_provider_stub() noexcept override = default;

private:
    void handle_call(method_id_type method_id,
                     data_istream  &method_args) override
    {
        switch (method_id)
        {
        case parameter_provider_method_id::display_name:
            call(&wdx::parameter_provider_i::display_name, method_id, method_args);
            break;
        case parameter_provider_method_id::get_provided_parameters:
            call(&wdx::parameter_provider_i::get_provided_parameters, method_id, method_args);
            break;
        case parameter_provider_method_id::get_parameter_values:
            call(&wdx::parameter_provider_i::get_parameter_values, method_id, method_args);
            break;
        case parameter_provider_method_id::set_parameter_values:
            call(&wdx::parameter_provider_i::set_parameter_values, method_id, method_args);
            break;
        case parameter_provider_method_id::invoke_method:
            call(&wdx::parameter_provider_i::invoke_method, method_id, method_args);
            break;
        case parameter_provider_method_id::set_parameter_values_connection_aware:
            call(&wdx::parameter_provider_i::set_parameter_values_connection_aware, method_id, method_args);
            break;
        case parameter_provider_method_id::create_parameter_upload_id:
            call(&wdx::parameter_provider_i::create_parameter_upload_id, method_id, method_args);
            break;
        case parameter_provider_method_id::remove_parameter_upload_id:
            call(&wdx::parameter_provider_i::remove_parameter_upload_id, method_id, method_args);
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


#endif // SRC_LIBWDXLINUXOSCOM_BACKEND_PARAMETER_PROVIDER_STUB_HPP_
//---- End of source file ------------------------------------------------------
