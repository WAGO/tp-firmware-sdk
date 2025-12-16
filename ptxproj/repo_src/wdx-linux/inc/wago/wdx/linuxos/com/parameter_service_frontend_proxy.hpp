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
///  \brief    Frontend proxy for WAGO parameter service.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_FRONTEND_PROXY_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_FRONTEND_PROXY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "api.h"

#include <wago/wdx/parameter_service_frontend_i.hpp>
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

//------------------------------------------------------------------------------
/// \brief Frontend interface proxy component.
///
/// This frontend interface proxy component may be used in foreign processes to establish communication to the
/// WAGO Parameter Service frontend interface \link wdx::parameter_service_frontend_i \endlink.
///
/// \note Methods of \link wdx::parameter_service_frontend_i \endlink may use
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
class parameter_service_frontend_proxy final : public driver_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(parameter_service_frontend_proxy)

private:
    std::unique_ptr<parameter_service_frontend_proxy_impl> impl;

public:
    //------------------------------------------------------------------------------
    /// Constructs a frontend proxy.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API explicit parameter_service_frontend_proxy(std::string const &connection_name);

    //------------------------------------------------------------------------------
    /// Constructs a frontend proxy by using a custom socket path for frontend connection.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    /// \param socket_path
    ///   Custom socket path for frontend connection.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API parameter_service_frontend_proxy(std::string const &connection_name,
                                                  std::string const &socket_path);

    WDXLINUXOSCOM_API parameter_service_frontend_proxy(parameter_service_frontend_proxy&&) noexcept;
    WDXLINUXOSCOM_API parameter_service_frontend_proxy& operator=(parameter_service_frontend_proxy&&) noexcept;
    WDXLINUXOSCOM_API ~parameter_service_frontend_proxy() noexcept override;

    // Interface driver_i
    WDXLINUXOSCOM_API bool run_once(uint32_t timeout_ms = no_wait) override; // NOLINT(google-default-arguments)
    WDXLINUXOSCOM_API bool run() override;
    WDXLINUXOSCOM_API void stop() override;

public:
    //------------------------------------------------------------------------------
    /// Method to get a frontend interface reference.
    ///
    /// \return
    ///   Reference to WAGO Parameter Service frontend interface.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API wdx::parameter_service_frontend_i &get_frontend();
};


//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_FRONTEND_PROXY_HPP_
//---- End of source file ------------------------------------------------------
