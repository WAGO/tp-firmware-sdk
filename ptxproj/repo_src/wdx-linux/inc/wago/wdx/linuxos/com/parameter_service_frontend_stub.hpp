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
///  \brief    Frontend stub for WAGO parameter service.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_FRONTEND_STUB_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_FRONTEND_STUB_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "api.h"

#include <wago/wdx/parameter_service_frontend_i.hpp>
#include <wago/wdx/unauthorized.hpp>
#include <wc/trace.h>
#include <wc/structuring.h>

#include <memory>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class parameter_service_frontend_stub_impl;

//------------------------------------------------------------------------------
/// \brief Backend interface stub component.
///
/// This frontend interface stub component is the counter part for the frontend interface proxy component
/// to establish communication to the WAGO Parameter Service frontend interface
/// \link wdx::parameter_service_frontend_i \endlink.
//------------------------------------------------------------------------------
class parameter_service_frontend_stub final : public driver_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(parameter_service_frontend_stub)

private:
    std::unique_ptr<parameter_service_frontend_stub_impl> impl;

public:
    //------------------------------------------------------------------------------
    /// Constructs a frontend stub.
    ///
    /// \param connection_name
    ///   Connection name to identify this connection in (log) messages.
    /// \param original_frontend
    ///   Unauthorized interface to use for providing frontend via remote proxy components.
    /// \param socket_path
    ///   Socket to create for proxy connections.
    /// \param socket_owner
    ///   Owner for socket to be set.
    /// \param socket_group
    ///   Group for socket to be set.
    /// \param channel
    ///   Trace channel to start/stop on incoming IPC calls.
    //------------------------------------------------------------------------------
    WDXLINUXOSCOM_API parameter_service_frontend_stub(std::string                                          const &connection_name,
                                                      wdx::unauthorized<wdx::parameter_service_frontend_i>       &original_frontend,
                                                      std::string                                          const &socket_path,
                                                      uid_t                                                       socket_owner,
                                                      gid_t                                                       socket_group,
                                                      wc_trace_channels                                    const  channel);

    WDXLINUXOSCOM_API parameter_service_frontend_stub(parameter_service_frontend_stub&&) noexcept;
    WDXLINUXOSCOM_API parameter_service_frontend_stub& operator=(parameter_service_frontend_stub&&) noexcept;
    WDXLINUXOSCOM_API ~parameter_service_frontend_stub() noexcept override;

    // Interface driver_i
    WDXLINUXOSCOM_API bool run_once(uint32_t timeout_ms = no_wait) override; // NOLINT(google-default-arguments)
    WDXLINUXOSCOM_API bool run() override;
    WDXLINUXOSCOM_API void stop() override;

    // Experimental
    WDXLINUXOSCOM_API std::vector<int> get_native_handles();
};


//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_PARAMETER_SERVICE_FRONTEND_STUB_HPP_
//---- End of source file ------------------------------------------------------
