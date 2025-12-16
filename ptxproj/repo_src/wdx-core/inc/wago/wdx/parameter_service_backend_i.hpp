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

#ifndef INC_WAGO_WDX_PARAMETER_SERVICE_BACKEND_I_HPP_
#define INC_WAGO_WDX_PARAMETER_SERVICE_BACKEND_I_HPP_

#include <vector>
#include <cstdint>
#include <exception>
#include <wago/future.hpp>
#include "wago/wdx/responses.hpp"
#include "wago/wdx/requests.hpp"
#include "wago/wdx/file_transfer/file_transfer_definitions.hpp"

namespace wago {
namespace wdx {

class parameter_provider_i;
class model_provider_i;
class device_description_provider_i;
class device_extension_provider_i;
inline namespace file_transfer {
    class file_provider_i;
}

/// Call mode for the operational calls (after the registration process) to providers.
enum class provider_call_mode {
    // Calls come regardless if other calls are still being processed
    concurrent,
    // Calls will be made one after the other, i.e. queued as long as the future has not been resolved. 
    serialized
};

using parameter_provider_call_mode = provider_call_mode;


/**
\interface parameter_service_backend_i
This interface is implemented by the `ParameterService`, and can be used by other components to announce devices, features and settings to any client.
It represents all methods to register or provide devices, settings etc.
*Registering* tells the `ParameterService` that something is there.
*Providing* is done exclusively by `IParameterProvider`s to claim responsibility for parameters or methods.
The `ParameterService` will then call the `IParameterProvider`-methods for the `ParameterID`s corresponding to the Provide<something>-call.
*Registering* and *Providing* are separate concerns. 
If a parameter was registered, but not (yet) provided, the `ParameterService` will at least be able to answer with `StatusCodes::PARAMETER_NOT_PROVIDED`.

NOTE regarding exceptions:
The Core implementation will not throw, but other implementations, e.g. for IPC might throw if problems occur in the surrounding infrastructure.
*/

class parameter_service_backend_i
{
public:
    /**
    Tells the `ParameterService` that a device exists with given `DeviceID`.
    The `orderNumber` will be used to read the device description, and with that,
    the `ParameterService` will create all `ParameterInstance`s that exist for the device.
     */
    virtual wago::future<std::vector<response>> register_devices(std::vector<register_device_request> requests) = 0;
    wago::future<response> register_device(register_device_request request);

    /**
    Tells the `ParameterService` to unregister all devices for given `deviceCollection`.
    The `ParameterService` will remove all `ParameterInstance`s that exist for the devices,
    and with that, will also forget which `ParameterProvider` was providing those.
    This means that there may exist `ParameterProviders` which do not provide anything anymore.
     */
    virtual wago::future<response> unregister_all_devices(device_collection_id_t device_collection) = 0;

    /**
    Tells the `ParameterService` to unregister all devices with given `device_ids`.
    The `ParameterService` will remove all `ParameterInstance`s that exist for the devices,
    and with that, will also forget which `ParameterProvider` was providing those.
    This means that there may exist `ParameterProviders` which do not provide anything anymore.
    */
    virtual wago::future<std::vector<response>> unregister_devices(std::vector<device_id> device_ids) = 0;
    wago::future<response> unregister_device(device_id device);

    /**
    Gives the `ParameterService` a list of `parameter_provider_i` implementations to use.
    Will return PROVIDER_NOT_OPERATIONAL if provider was already registered or problems occur.
     */
    virtual wago::future<std::vector<response>> register_parameter_providers(std::vector<parameter_provider_i*> providers, parameter_provider_call_mode mode = parameter_provider_call_mode::concurrent) = 0;
    wago::future<response> register_parameter_provider(parameter_provider_i* provider, parameter_provider_call_mode mode = parameter_provider_call_mode::concurrent);

    /**
    Removes the list of providers from the `ParameterService`.
    Parameters that a `parameter_provider_i` implementation may have provided will be unprovided.
     */
    virtual void unregister_parameter_providers(std::vector<parameter_provider_i*> providers) = 0;
    void unregister_parameter_provider(parameter_provider_i* provider);

    /**
    Gives the `ParameterService` a list of `model_provider_i` implementations to use.
    Will return PROVIDER_NOT_OPERATIONAL if provider was already registered or problems occur.
     */
    virtual wago::future<std::vector<response>> register_model_providers(std::vector<model_provider_i*> providers) = 0;
    wago::future<response> register_model_provider(model_provider_i* provider);

    /**
    Removes the list of providers from the `ParameterService`.
    Model information that a `model_provider_i` might have provided will not be retracted.
     */
    virtual void unregister_model_providers(std::vector<model_provider_i*> providers) = 0;
    void unregister_model_provider(model_provider_i* provider);

    /**
    Gives the `ParameterService` a list of `device_description_provider_i` implementations to use.
    Will return PROVIDER_NOT_OPERATIONAL if provider was already registered or problems occur.
     */
    virtual wago::future<std::vector<response>> register_device_description_providers(std::vector<device_description_provider_i*> providers) = 0;
    wago::future<response> register_device_description_provider(device_description_provider_i* provider);

    /**
    Removes the list of providers from the `ParameterService`.
    Device information that a `device_description_provider_i` might have provided will not be retracted.
     */
    virtual void unregister_device_description_providers(std::vector<device_description_provider_i*> providers) = 0;
    void unregister_device_description_provider(device_description_provider_i* provider);

    /**
    Gives the `ParameterService` a list of `device_extension_provider_i` implementations to use.
    Will return PROVIDER_NOT_OPERATIONAL if provider was already registered or problems occur.
     */
    virtual wago::future<std::vector<response>> register_device_extension_providers(std::vector<device_extension_provider_i*> providers) = 0;
    wago::future<response> register_device_extension_provider(device_extension_provider_i* provider);

    /**
    Removes the list of providers from the `ParameterService`.
    Device information that a `device_extension_provider_i` might have provided will be retracted (not implemented yet).
     */
    virtual void unregister_device_extension_providers(std::vector<device_extension_provider_i*> providers) = 0;
    void unregister_device_extension_provider(device_extension_provider_i* provider);

    virtual wago::future<std::vector<register_file_provider_response>> register_file_providers(std::vector<register_file_provider_request> requests, provider_call_mode mode = provider_call_mode::concurrent) = 0;
    wago::future<register_file_provider_response> register_file_provider(register_file_provider_request request, provider_call_mode mode = provider_call_mode::concurrent);
    wago::future<register_file_provider_response> register_file_provider(file_transfer::file_provider_i* provider, parameter_id_t context, provider_call_mode mode = provider_call_mode::concurrent);

    virtual void unregister_file_providers(std::vector<file_transfer::file_provider_i*> providers) = 0;
    void unregister_file_provider(file_transfer::file_provider_i* provider);

    virtual ~parameter_service_backend_i() noexcept = default;
};

/**
 * The base class of backend exceptions.
 * The Core implementation will not throw, but other implementations, e.g. for IPC might throw if problems occur in the surrounding infrastructure.
 * */
class backend_exception : public std::runtime_error {
    public:
    using std::runtime_error::runtime_error;
};

}
}
#endif // INC_WAGO_WDX_PARAMETER_SERVICE_BACKEND_I_HPP_
