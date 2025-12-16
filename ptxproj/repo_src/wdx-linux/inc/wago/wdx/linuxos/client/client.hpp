//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO Parameter Service client application base class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_CLIENT_CLIENT_HPP_
#define INC_WAGO_WDX_LINUXOS_CLIENT_CLIENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/client/client_option.hpp"

#include <wago/wdx/parameter_service_backend_i.hpp>
#include <wc/exit.h>

#include <memory>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace client {

//------------------------------------------------------------------------------
/// \brief Argument structure for init method
///
/// \see abstract_client::init()
//------------------------------------------------------------------------------
struct init_args
{
    //------------------------------------------------------------------------------
    /// Backend to use in initialization.
    ///
    /// \remark Backend reference can be safely stored inside client for later use.
    //------------------------------------------------------------------------------
    wdx::parameter_service_backend_i &backend;
};

//------------------------------------------------------------------------------
/// \brief Abstract client class as base for provider daemons
///
/// Abstract client class as base for provider daemons as clients for WAGO Parameter Service.
///
/// Usage example (simplified without any error handling):
/// \code{.cpp}
/// class my_client final : public wago::wdx::linuxos::client::abstract_client
/// {
/// public:
///     my_client() = default;
///     ~my_client() override = default;
///
///     std::string get_name() override
///     {
///         return "My Parameter Service Client App";
///     }
///
///     void init(init_args args) override;
///     {
///         a_provider = std::make_unique<my_provider>();
///         wago::wdx::linuxos::client::check_register(args.backend.register_parameter_provider(a_provider.get()), [] (auto const &error_message) {
///            wc_log_format(fatal, "Failed to initialize client: %s", error_message.c_str());
///            wc_exit(init_fails_exit_status);
///         });
///     }
///
/// private:
///     std::unique_ptr<wdx::parameter_provider_i> a_provider = nullptr;
/// };
///
/// std::unique_ptr<wago::wdx::linuxos::client::abstract_client> wago::wdx::linuxos::client::create_client()
/// {
///     return std::make_unique<my_client>();
/// }
/// \endcode
///
/// \see example_client.cpp
//------------------------------------------------------------------------------
class abstract_client
{
protected:
    // Keep this in sync with init_fails_exit_status in main.cpp
    static constexpr int init_fails_exit_status = WC_EXIT_ERROR_FIRST;

public:
    abstract_client() = default;

    //------------------------------------------------------------------------------
    /// Use destructor to clean up your providers. It is not required to access
    /// the backend to unregister providers here. Provider unregistration is done
    /// automatically by backend proxy before this destructor is called.
    ///
    /// \remark: Any calls to the backend done from within this destructor may throw
    ///          an exception, because the connection may already have been cleaned up.
    ///          Therefore, calls must be wrapped in try-catch block.
    //------------------------------------------------------------------------------
    virtual ~abstract_client() noexcept = default;

    //------------------------------------------------------------------------------
    /// Name for this client (used for log output).
    ///
    /// \remark
    ///   wago::wdx::parameter_provider_i::get_name() may be used in case of a single provider.
    ///
    /// \return
    ///   Name for this client as string.
    //------------------------------------------------------------------------------
    virtual std::string get_name() = 0;

    //------------------------------------------------------------------------------
    /// Get additional options for this client.
    ///
    /// \remark
    ///   Only upper case characters allowed for short option character in
    ///   additional client options. Lower case characters are reserved for
    ///   general client options implemented by libwdxlinuxosclient.
    ///
    /// \return
    ///   Vector with additional client options.
    //------------------------------------------------------------------------------
    virtual std::vector<client_option> get_additional_options()
    {
        return {};
    }

    //------------------------------------------------------------------------------
    /// Init method to prepare and register providers.
    ///
    /// \remark: Do NOT call by your self!
    //------------------------------------------------------------------------------
    virtual void init(init_args args) = 0;
};


//------------------------------------------------------------------------------
/// Factory function to build a specific client derived from abstract_client.
///
/// \remark
///   This function should not do anything beyond instantiation of abstract_client derivative.
///
/// \return
///   Created client instance.
//------------------------------------------------------------------------------
std::unique_ptr<abstract_client> create_client();


} // Namespace client
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_CLIENT_CLIENT_HPP_
//---- End of source file ------------------------------------------------------
