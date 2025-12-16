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

#ifndef INC_WAGO_WDX_PARAMETER_SERVICE_BACKEND_EXTENDED_I_HPP_
#define INC_WAGO_WDX_PARAMETER_SERVICE_BACKEND_EXTENDED_I_HPP_

#include "wago/wdx/parameter_service_backend_i.hpp"

namespace wago {
namespace wdx {

inline namespace file_transfer {
    class file_provider_i;
}

/**
This interface contains extended backend methods used by IPC implementation. It is not meant for widespread use.
Methods might change and/or be removed in the future.
 */

class parameter_service_backend_extended_i : public parameter_service_backend_i
{
public:

    virtual wago::future<std::vector<register_file_provider_response>> reregister_file_providers(std::vector<reregister_file_provider_request> requests, provider_call_mode mode = provider_call_mode::concurrent) = 0;
    wago::future<register_file_provider_response> reregister_file_provider(file_transfer::file_provider_i* provider, file_id file_id_, parameter_id_t context, provider_call_mode mode = provider_call_mode::concurrent);

    virtual ~parameter_service_backend_extended_i() noexcept = default;
};

}
}
#endif // INC_WAGO_WDX_PARAMETER_SERVICE_BACKEND_EXTENDED_I_HPP_
