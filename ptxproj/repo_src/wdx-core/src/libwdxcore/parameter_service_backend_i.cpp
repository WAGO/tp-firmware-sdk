//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#include "wago/wdx/parameter_service_backend_extended_i.hpp"
#include "utils/future-helpers.hpp"

namespace wago {
namespace wdx {

wago::future<response> parameter_service_backend_i::register_device(register_device_request request) {
    return mapped_future<std::vector<response>, response>(
        register_devices({std::move(request)}),
        [](auto r){ return r.front();
    });
}

wago::future<response> parameter_service_backend_i::unregister_device(device_id device) {
    return mapped_future<std::vector<response>, response>(
        unregister_devices({device}),
        [](auto r){ return r.front();
    });
}

wago::future<response> parameter_service_backend_i::register_parameter_provider(parameter_provider_i* provider, parameter_provider_call_mode mode) {
    return mapped_future<std::vector<response>, response>(
        register_parameter_providers({provider}, mode),
        [](auto r){ return r.front();
    });
}

void parameter_service_backend_i::unregister_parameter_provider(parameter_provider_i* provider) {
    unregister_parameter_providers({provider});
}

wago::future<response> parameter_service_backend_i::register_model_provider(model_provider_i* provider) {
    return mapped_future<std::vector<response>, response>(
        register_model_providers({provider}),
        [](auto r){ return r.front();
    });
}

void parameter_service_backend_i::unregister_model_provider(model_provider_i* provider) {
    unregister_model_providers({provider});
}

wago::future<response> parameter_service_backend_i::register_device_description_provider(device_description_provider_i* provider) {
    return mapped_future<std::vector<response>, response>(
        register_device_description_providers({provider}),
        [](auto r){ return r.front();
    });
}

void parameter_service_backend_i::unregister_device_description_provider(device_description_provider_i* provider) {
    unregister_device_description_providers({provider});
}

wago::future<response> parameter_service_backend_i::register_device_extension_provider(device_extension_provider_i* provider) {
    return mapped_future<std::vector<response>, response>(
        register_device_extension_providers({provider}),
        [](auto r){ return r.front();
    });
}

void parameter_service_backend_i::unregister_device_extension_provider(device_extension_provider_i* provider) {
    unregister_device_extension_providers({provider});
}

wago::future<register_file_provider_response> parameter_service_backend_i::register_file_provider(register_file_provider_request request, provider_call_mode mode) {
    return mapped_future<std::vector<register_file_provider_response>, register_file_provider_response>(
        register_file_providers({request}, mode),
        [](auto r){ return r.front();
    });
}

wago::future<register_file_provider_response> parameter_service_backend_i::register_file_provider(file_transfer::file_provider_i* provider, parameter_id_t context, provider_call_mode mode) {
    auto request = register_file_provider_request(provider, context); // parasoft-suppress CERT_CPP-EXP57-b-3 "Provider pointer (incomplete type) is only stored in request, not modified or deleted"
    return register_file_provider(std::move(request), mode);
}

void parameter_service_backend_i::unregister_file_provider(file_transfer::file_provider_i* provider) {
    unregister_file_providers({provider});
}

wago::future<register_file_provider_response> parameter_service_backend_extended_i::reregister_file_provider(file_transfer::file_provider_i* provider, file_id file_id_, parameter_id_t context, provider_call_mode mode) {
    (void)mode; // Currently unused

    auto request = reregister_file_provider_request(provider, std::move(file_id_), context); // parasoft-suppress CERT_CPP-EXP57-b-3 "Provider pointer (incomplete type) is only stored in request, not modified or deleted"
    return mapped_future<std::vector<register_file_provider_response>, register_file_provider_response>(
        reregister_file_providers({std::move(request)}),
        [](auto r){ return r.front();
    });
}

}}
