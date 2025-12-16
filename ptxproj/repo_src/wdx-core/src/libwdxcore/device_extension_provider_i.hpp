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

#ifndef SRC_LIBWDXCORE_DEVICE_EXTENSION_PROVIDER_I_HPP_
#define SRC_LIBWDXCORE_DEVICE_EXTENSION_PROVIDER_I_HPP_

#include <string>
#include "wago/wdx/responses.hpp"

namespace wago {
namespace wdx {

/**
    \interface device_extension_provider_i
    Provides the parameter service with additional information about the device, building upon the information already known by a `device_description_provider`.
    In contrast to the `device_description_provider`, the provided information of `device_extension_provider_i` is bound the the lifetime of the `device_extension_provider_i`.
*/
class device_extension_provider_i
{
public:

    /** The `ParameterService` will call this method to retrieve additional information for a registered device that matches the provided `device_selector`s.
        It is expected not to throw. Errors should instead be reported with `status code::internal_error`.
        The provided information is retracted when the `device_extension_provider_i` is unregistered. NOTE: The retraction is not implemented yet.
    */
    virtual device_extension_response get_device_extensions() = 0;

    virtual ~device_extension_provider_i() noexcept = default;
};

}}

#endif // SRC_LIBWDXCORE_DEVICE_EXTENSION_PROVIDER_I_HPP_