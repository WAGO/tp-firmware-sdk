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

#ifndef SRC_LIBWDXCORE_DEVICE_DESCRIPTION_PROVIDER_I_HPP_
#define SRC_LIBWDXCORE_DEVICE_DESCRIPTION_PROVIDER_I_HPP_

#include <string>
#include "wago/wdx/responses.hpp"
#include <wago/future.hpp>
#include "wago/wdx/device_id.hpp"

namespace wago {
namespace wdx {

/**
    \interface device_description_provider_i
    Provides the parameter service with a device description.
*/
class device_description_provider_i
{
public:

    /**
        Tells the `ParameterService` for which devices the `device_description_provider_i` will provide WDD artifacts for.
        The `ParameterService` will ask this once at registration.
        It is expected not to throw. Errors should instead be reported with `status code::internal_error`.
        If the returned status code is not ::success, the `device_description_provider_i` will not be used in any way.
        Otherwise, for every device that is registered (even later) that matches the `device_selector`s,
        the `device_description_provider_i` will be asked to provide its information.
    */
    virtual device_selector_response get_provided_devices() = 0;

    /** The `ParameterService` will call this method to retrieve the device information for a registered device that matches the provided `device_selector`s.
        It is expected not to throw. Errors should instead be reported with `status code::internal_error`.
        When a default-constructed `wdd_response()` or one with an error status is returned,
        the ParameterService will try the next matching `device_description_provider_i`.
        Otherwise, it will stop asking other `device_description_provider_i`s for this particular device.
        The provided information remains active even after the `device_description_provider_i` is unregistered.
    */
    virtual wago::future<wdd_response> get_device_information(std::string order_number, std::string firmware_version) = 0;

    virtual ~device_description_provider_i() noexcept = default;
};

}}

#endif // SRC_LIBWDXCORE_DEVICE_DESCRIPTION_PROVIDER_I_HPP_