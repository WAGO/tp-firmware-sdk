//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Server interface.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_SERV_SERVER_I_HPP_
#define INC_WAGO_WDX_LINUXOS_SERV_SERVER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/serv/api.h"

#include <wc/std_includes.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

/// The native handle which is a file descriptor
using native_handle_type = int;

/// A server implementation using conforming to this interface is
/// responsible to receive and handle and incoming request.
///
/// The implementation of a server is usually technology dependent.
class server_i
{
    WC_INTERFACE_CLASS(server_i)

public:
    /// Get a native handle, (i.e. a file descriptor), if applicable/supported
    /// by the server implementation.
    /// 
    /// \return 
    ///    A handle to be used to check for events by using it in "poll" or "select" 
    ///    system functions.
    virtual native_handle_type get_native_handle() const = 0;

    /// Receive and handle the next request available in the given period of time.
    /// Calls to this method will block until either a request has been handled or
    /// the timeout has been reached.
    /// 
    /// \param timeout_ms
    ///    Milliseconds to wait for a new request to be available. No request will 
    ///    be handled if not occurring in the allowed timespan.
    /// \return
    ///    True, if a request has been handled. False otherwise.
    virtual bool receive_next(uint32_t timeout_ms) = 0;

// Interface extensions
public:
    /// Receive and handle the next request available. Calls to this method will not 
    /// block but instead only handle a request if one is available.
    /// 
    /// \return
    ///    True, if a request has been handled. False otherwise.
    inline bool receive_next()
    {
        return receive_next(0);
    }
};


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_SERV_SERVER_I_HPP_
//---- End of source file ------------------------------------------------------
