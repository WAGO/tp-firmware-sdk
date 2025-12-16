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

#ifndef SRC_LIBWDXCORE_MODEL_PROVIDER_I_HPP_
#define SRC_LIBWDXCORE_MODEL_PROVIDER_I_HPP_

#include <string>
#include "wago/wdx/responses.hpp"
#include <wago/future.hpp>

namespace wago {
namespace wdx {

/**
    \interface model_provider_i
    Provides the parameter service with WDM information.
*/
class model_provider_i
{
public:

    /**
        Provides the `ParameterService` with WDM information that is independent of specific devices.
        Useful to add WDM feature definitions to the pool that is generally available.
        The `ParameterService` will ask this once at registration.
        It is expected not to throw. Errors should instead be reported with `status code::internal_error`.
        If nothing is to be provided, use a default-constructed `wdm_response()`
     */
    virtual wago::future<wdm_response> get_model_information() = 0;

    virtual ~model_provider_i() noexcept = default;
};

}}

#endif // SRC_LIBWDXCORE_MODEL_PROVIDER_I_HPP_