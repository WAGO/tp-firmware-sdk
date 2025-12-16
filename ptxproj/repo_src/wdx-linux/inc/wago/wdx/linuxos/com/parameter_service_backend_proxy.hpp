//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Backend proxy for WAGO parameter service.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_BACKEND_PROXY_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_BACKEND_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "wago/wdx/linuxos/com/notification_manager_i.hpp"
#include "api.h"

#include <wago/wdx/parameter_service_backend_i.hpp>
#include <wc/trace.h>
#include <wc/structuring.h>

#include <memory>
#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class parameter_service_backend_proxy_impl;

//------------------------------------------------------------------------------
/// Notification manager specialization for WAGO Parameter Service backend.
//------------------------------------------------------------------------------
using backend_notification_manager_i = notification_manager_i<void(wdx::parameter_service_backend_i &backend)>;

//------------------------------------------------------------------------------
/// \brief Backend interface proxy component.
///
/// This backend interface proxy component may be used in foreign processes to establish communication to the
/// WAGO Parameter Service backend interface \link wdx::parameter_service_backend_i \endlink.
///
/// \note Methods of \link wdx::parameter_service_backend_i \endlink may use
///       Exceptions of Type \link wdx::com::exception \endlink when errors
///       occur during communication with the backend. Generally speaking, proxy
///       class methods either throw such an exception or set the exception on
///       the future in case of an asynchronous call. Such exceptions may be
///       handled by either catching them or by setting an exception_notifier
///       on the future. As Communication related errors are often temporary,
///       such a handling is recommended that implements a retry mechanism. The
///       retry should also run the driver before calling the method again to 
///       eliminate some temporary errors.
//------------------------------------------------------------------------------
class parameter_service_backend_proxy final : public driver_i, private backend_notification_manager_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(parameter_service_backend_proxy)

public:
    //------------------------------------------------------------------------------
    /// Connection notifier, called when proxy component is connected and
    /// WAGO Parameter Service backend interface \link wdx::parameter_service_backend_i \endlink is ready for use.
    /// The backend interface will also be provided in notifier for convenience.
    //------------------------------------------------------------------------------
    using connect_notifier = notifier_type;

private:
    std::unique_ptr<parameter_service_backend_proxy_impl> impl;

public:
    //------------------------------------------------------------------------------
    /// Constructs a backend proxy.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API explicit parameter_service_backend_proxy(std::string const &connection_name);

    //------------------------------------------------------------------------------
    /// Constructs a backend proxy by using a custom socket path for backend connection.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    /// \param socket_path
    ///   Custom socket path for backend connection.
    /// \param channel
    ///   Trace channel to start/stop on incoming IPC calls for providers.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API parameter_service_backend_proxy(std::string       const &connection_name,
                                                      std::string       const &socket_path,
                                                      wc_trace_channels const  channel = wc_trace_channels::invalid);

    WDXLINUXOSCOM_API parameter_service_backend_proxy(parameter_service_backend_proxy&&) noexcept;
    WDXLINUXOSCOM_API parameter_service_backend_proxy& operator=(parameter_service_backend_proxy&&) noexcept;
    WDXLINUXOSCOM_API ~parameter_service_backend_proxy() noexcept override;

    // Interface driver_i
    WDXLINUXOSCOM_API bool run_once(uint32_t timeout_ms = no_wait) override; // NOLINT(google-default-arguments)
    WDXLINUXOSCOM_API bool run() override;
    WDXLINUXOSCOM_API void stop() override;

private:
    // Interface notification_manager_i
    notifier_handle add_notifier(connect_notifier notifier) override;
    void            remove_notifier(notifier_handle handle) override;

public:
    //------------------------------------------------------------------------------
    /// Method to get a backend interface reference.
    ///
    /// \return
    ///   Reference to WAGO Parameter Service backend interface.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API wdx::parameter_service_backend_i &get_backend();

    //------------------------------------------------------------------------------
    /// Method to get a backend notification manager interface reference.
    ///
    /// \return
    ///   Reference to WAGO Parameter Service backend notification manager interface.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API backend_notification_manager_i &get_notification_manager();
};


//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_BACKEND_PROXY_HPP_
//---- End of source file ------------------------------------------------------
