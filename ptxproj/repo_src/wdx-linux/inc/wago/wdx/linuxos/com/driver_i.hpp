//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Communication driver interface for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_DRIVER_I_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_DRIVER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/std_includes.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
/// \brief Driver interface to drive passiv components
///
/// This driver interface is used to drive passiv components (components without own thread).
//------------------------------------------------------------------------------
class WDXLINUXOSCOM_API driver_i
{
    WC_INTERFACE_CLASS(driver_i)

public:
    static constexpr uint32_t const no_wait = 0U;

    //------------------------------------------------------------------------------
    /// Run at most one task from this component.
    ///
    /// \param timeout_ms
    ///   Timeout value in milliseconds for time to wait for a task to do.
    ///   Zero (default value) means do a task if at least one is to do or return immediately otherwise.
    ///
    /// \return
    ///   True, if a task was done within timeout, false otherwise
    //------------------------------------------------------------------------------
    virtual bool run_once(uint32_t timeout_ms = no_wait) = 0; // NOLINT(google-default-arguments)

    //------------------------------------------------------------------------------
    /// Run all open tasks from this component.
    ///
    /// \brief
    ///   This methods blocks until all open tasks are done, even if this tasks currently not ready
    ///   (e. g. a read request for incoming data is queued).
    ///   In most cases this will result in a blocking behavior.
    ///
    /// \return
    ///   True, if at least one task was done, false otherwise
    //------------------------------------------------------------------------------
    virtual bool run() = 0;

    //------------------------------------------------------------------------------
    /// Abort any currently running run operation immediatly.
    ///
    /// \brief
    ///   This method will cause any run operation to return immediatly. 
    //------------------------------------------------------------------------------
    virtual void stop() = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_DRIVER_I_HPP_
//---- End of source file ------------------------------------------------------
