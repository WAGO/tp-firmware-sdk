//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Combined proxy component for WAGO parameter service.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_PROXY_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "wago/wdx/linuxos/com/notification_manager_i.hpp"
#include "api.h"

#include <wago/wdx/parameter_service_frontend_i.hpp>
#include <wago/wdx/file_transfer/parameter_service_file_api_i.hpp>
#include <wago/wdx/parameter_service_backend_i.hpp>
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

class parameter_service_frontend_proxy_impl;
class parameter_service_file_api_proxy_impl;
class parameter_service_backend_proxy_impl;

//------------------------------------------------------------------------------
/// Notification manager specialization for WAGO Parameter Service backend.
//------------------------------------------------------------------------------
using backend_notification_manager_i = notification_manager_i<void(wdx::parameter_service_backend_i &backend)>;

//------------------------------------------------------------------------------
/// \brief Parameter service proxy component.
///
/// This proxy component may be used in foreign processes to establish communication to the
/// WAGO Parameter Service interfaces.
///
/// \note Methods of \link wdx::parameter_service_backend_i \endlink,
///       \link wdx::parameter_service_file_api_i \endlink and 
///       \link wdx::parameter_service_frontend_i \endlink may use
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
class parameter_service_proxy final : public driver_i, private backend_notification_manager_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(parameter_service_proxy)

public:
    //------------------------------------------------------------------------------
    /// Connection notifier, called when proxy component is connected and
    /// WAGO Parameter Service backend interface \link wdx::parameter_service_backend_i \endlink is ready for use.
    /// The backend interface will also be provided in notifier for convenience.
    //------------------------------------------------------------------------------
    using connect_notifier = notifier_type;

private:
    std::unique_ptr<parameter_service_frontend_proxy_impl> frontend_impl;
    std::unique_ptr<parameter_service_file_api_proxy_impl> file_api_impl;
    std::unique_ptr<parameter_service_backend_proxy_impl>  backend_impl;

public:
    //------------------------------------------------------------------------------
    /// Constructs a proxy.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API explicit parameter_service_proxy(std::string const &connection_name);

    //------------------------------------------------------------------------------
    /// Constructs a proxy by using custom socket paths for connections.
    ///
    /// \deprecated use constructor with additional file api socket path instead. 
    ///             get_file_api() will throw if called after using this
    ///             constructor.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    /// \param frontend_socket_path
    ///   Custom socket path for frontend connection.
    /// \param backend_socket_path
    ///   Custom socket path for backend connection.
    ///             
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API parameter_service_proxy(std::string const &connection_name,
                                         std::string const &frontend_socket_path,
                                         std::string const &backend_socket_path);

    //------------------------------------------------------------------------------
    /// Constructs a proxy by using custom socket paths for connections.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    /// \param frontend_socket_path
    ///   Custom socket path for frontend connection.
    /// \param file_api_socket_path
    ///   Custom socket path for file api connection.
    /// \param backend_socket_path
    ///   Custom socket path for backend connection.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API parameter_service_proxy(std::string const &connection_name,
                                         std::string const &frontend_socket_path,
                                         std::string const &file_api_socket_path,
                                         std::string const &backend_socket_path);

    WDXLINUXOSCOM_API parameter_service_proxy(parameter_service_proxy&&) noexcept;
    WDXLINUXOSCOM_API parameter_service_proxy& operator=(parameter_service_proxy&&) noexcept;
    WDXLINUXOSCOM_API ~parameter_service_proxy() noexcept override;

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
    /// Method to get a frontend interface reference.
    ///
    /// \return
    ///   Reference to WAGO Parameter Service frontend interface.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API wdx::parameter_service_frontend_i &get_frontend();

    //------------------------------------------------------------------------------
    /// Method to get a File API interface reference.
    ///
    /// \return
    ///   Reference to WAGO Parameter Service File API interface.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API wdx::parameter_service_file_api_i &get_file_api();

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


#endif // INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_PROXY_HPP_
//---- End of source file ------------------------------------------------------
