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
///  \brief    Backend connection handler for WAGO parameter service.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_COM_NOTIFICATION_MANAGER_I_HPP_
#define INC_WAGO_WDX_LINUXOS_COM_NOTIFICATION_MANAGER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/structuring.h>

#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
/// \brief Notification manager interface.
///
/// This interface is used to manage registered notifiers.
//------------------------------------------------------------------------------
template<typename NotifierType>
class WDXLINUXOSCOM_API notification_manager_i
{
    WC_INTERFACE_CLASS(notification_manager_i)

public:
    //------------------------------------------------------------------------------
    /// Notifier type for current manager specialization.
    //------------------------------------------------------------------------------
    using notifier_type   = std::function<NotifierType>;

    //------------------------------------------------------------------------------
    /// Notifier handle, used to manage registered notifications (e.g. remove notifier).
    //------------------------------------------------------------------------------
    using notifier_handle = size_t;

    //------------------------------------------------------------------------------
    /// Adds a notifier for (re-)established connection.
    ///
    /// \param notifier
    ///   Notifier to call when a connection is (re-)established.
    ///
    /// \return
    ///   @copydoc notifier_handle
    //------------------------------------------------------------------------------
    virtual notifier_handle add_notifier(notifier_type notifier) = 0;

    //------------------------------------------------------------------------------
    /// Removes a registered notifier.
    ///
    /// \param handle
    ///   Handle corresponding to notifier to remove.
    //------------------------------------------------------------------------------
    virtual void            remove_notifier(notifier_handle handle) = 0;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_COM_NOTIFICATION_MANAGER_I_HPP_
//---- End of source file ------------------------------------------------------
