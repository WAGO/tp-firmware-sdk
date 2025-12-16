//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------

#include "wago/wdx/unauthorized.hpp"
#include "authorized.hpp"

namespace wago {
namespace wdx {

template<class ParameterServiceInterface>
unauthorized<ParameterServiceInterface>::unauthorized(std::shared_ptr<parameter_service_i> service) noexcept
: service_m(service)
{ }

template<class ParameterServiceInterface>
std::unique_ptr<ParameterServiceInterface> unauthorized<ParameterServiceInterface>::authorize(user_data user) const noexcept
{
    return std::make_unique<authorized>(user, service_m);
}

// add instantiations for each interface which is supported to be wrapped by "unauthorized":
template class unauthorized<parameter_service_frontend_i>;
template class unauthorized<parameter_service_frontend_extended_i>;
template class unauthorized<parameter_service_file_api_i>;

}
}
