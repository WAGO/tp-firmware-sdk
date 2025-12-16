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
///  \brief    Service frontend proxy for IPC.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_PROXY_HPP_
#define SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/abstract_proxy.hpp"
#include "frontend_methods.hpp"

#include <wago/wdx/parameter_service_frontend_i.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class frontend_proxy final : public abstract_proxy<wdx::parameter_service_frontend_i>
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(frontend_proxy)

public:
    using abstract_proxy::abstract_proxy;
    ~frontend_proxy() noexcept override = default;

    wago::future<wdx::device_collection_response> get_all_devices() override
    {
        return call<future<wdx::device_collection_response>>(frontend_method_id::frontend_get_all_devices);
    }

    wago::future<wdx::parameter_response_list_response> get_all_parameters(wdx::parameter_filter filter, size_t paging_offset, size_t paging_limit) override
    {
        return call<future<wdx::parameter_response_list_response>>(frontend_method_id::frontend_get_all_parameters, filter, paging_offset, paging_limit);
    }

    wago::future<std::vector<wdx::parameter_response>> get_parameters(std::vector<wdx::parameter_instance_id> ids) override
    {
        return call<future<std::vector<wdx::parameter_response>>>(frontend_method_id::frontend_get_parameters, ids);
    }

    wago::future<std::vector<wdx::parameter_response>> get_parameters_by_path(std::vector<wdx::parameter_instance_path> paths) override
    {
        return call<future<std::vector<wdx::parameter_response>>>(frontend_method_id::frontend_get_parameters_by_path, paths);
    }

    wago::future<wdx::method_invocation_named_response> invoke_method(wdx::parameter_instance_id method_id, std::map<std::string, std::shared_ptr<wdx::parameter_value>> in_args) override
    {
        return call<future<wdx::method_invocation_named_response>>(frontend_method_id::frontend_invoke_method, method_id, in_args);
    }

    wago::future<wdx::method_invocation_named_response> invoke_method_by_path(wdx::parameter_instance_path method_path, std::map<std::string, std::shared_ptr<wdx::parameter_value>> in_args) override
    {
        return call<future<wdx::method_invocation_named_response>>(frontend_method_id::frontend_invoke_method_by_path, method_path, in_args);
    }

    wago::future<std::vector<wdx::set_parameter_response>> set_parameter_values(std::vector<wdx::value_request> value_requests) override
    {
        return call<future<std::vector<wdx::set_parameter_response>>>(frontend_method_id::frontend_set_parameter_values, value_requests);
    }

    wago::future<std::vector<wdx::set_parameter_response>> set_parameter_values_by_path(std::vector<wdx::value_path_request> value_path_requests) override
    {
        return call<future<std::vector<wdx::set_parameter_response>>>(frontend_method_id::frontend_set_parameter_values_by_path, value_path_requests);
    }
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_FRONTEND_FRONTEND_PROXY_HPP_
//---- End of source file ------------------------------------------------------
