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
//------------------------------------------------------------------------------

#ifndef INC_WAGO_WDX_PARAMETER_SERVICE_FRONTEND_I_HPP_
#define INC_WAGO_WDX_PARAMETER_SERVICE_FRONTEND_I_HPP_

#include <vector>
#include <map>
#include <cstdint>
#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/parameter_instance_path.hpp"
#include "wago/wdx/responses.hpp"
#include "wago/wdx/requests.hpp"
#include "wago/future.hpp"

namespace wago {
namespace wdx {

/**
This interface is used by clients that want to access parameters of a device.

NOTE regarding exceptions:
The Core implementation will not throw, but other implementations, e.g. for IPC might throw if problems occur in the surrounding infrastructure.
 */

class parameter_service_frontend_i
{
public:
    /**
    Returns all present devices as flat list, no matter in which device_collection they are.
    */
    virtual wago::future<device_collection_response> get_all_devices() = 0;

    /** Get all parameters that satisfy `filter`, with paging if needed.
    */
    virtual wago::future<parameter_response_list_response> get_all_parameters(parameter_filter filter, size_t paging_offset = 0, size_t paging_limit = SIZE_MAX) = 0;

    /**
    Returns the value for every requested parameter instance, addressed by its numerical ID.
    */
    virtual wago::future<std::vector<parameter_response>> get_parameters(std::vector<parameter_instance_id> ids) = 0;

    /**
    Returns the value for every requested parameter instance, addressed by its path.
    Paths are case-insensitive.
    */
    virtual wago::future<std::vector<parameter_response>> get_parameters_by_path(std::vector<parameter_instance_path> paths) = 0;

    /**
    Invokes the modelled method with the given arguments, addressed by its ID.
    Returns the result of the invocation.
     */
    virtual wago::future<method_invocation_named_response> invoke_method(parameter_instance_id method_id, std::map<std::string, std::shared_ptr<parameter_value>> in_args) = 0;

    /**
    Invokes the modelled method with the given arguments, addressed by its path.
    Returns the result of the invocation.
     */
    virtual wago::future<method_invocation_named_response> invoke_method_by_path(parameter_instance_path method_path, std::map<std::string, std::shared_ptr<parameter_value>> in_args) = 0;

    /**
    For each value request, sets the parameter with specified ID to the given value.
    The batch will - if possible - be treated as one consistent set (either all are written or none),
    but this will be only be possible within one device and not all devices will be able to support this.
     */
    virtual wago::future<std::vector<set_parameter_response>> set_parameter_values(std::vector<value_request> value_requests) = 0;

    /**
    For each value request, sets the parameter with specified path to the given value.
    The batch will - if possible - be treated as one consistent set (either all are written or none),
    but this will be only be possible within one device and not all devices will be able to support this.
     */
    virtual wago::future<std::vector<set_parameter_response>> set_parameter_values_by_path(std::vector<value_path_request> value_path_requests) = 0;

    virtual ~parameter_service_frontend_i() noexcept = default;
};

/**
 * The base class of frontend exceptions.
 * The Core implementation will not throw, but other implementations, e.g. for IPC might throw if problems occur in the surrounding infrastructure.
 * */
class frontend_exception : public std::runtime_error {
    public:
    using std::runtime_error::runtime_error;
};

}
}
#endif // INC_WAGO_WDX_PARAMETER_SERVICE_FRONTEND_I_HPP_
