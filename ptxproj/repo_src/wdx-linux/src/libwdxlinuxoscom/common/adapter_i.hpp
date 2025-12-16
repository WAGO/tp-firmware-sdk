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
///  \brief    Data (adapter) send/receive interface on message base.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_ADAPTER_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_ADAPTER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>
#include <vector>
#include <functional>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using message_data = std::vector<uint8_t>;

using send_handler    = std::function<void(std::string error_message)>;
using receive_handler = std::function<void(message_data const &message,
                                           std::string         error_message)>;

/// Connection information object
struct connection_info
{
    std::string remote_process_owner;
};

/// I/O interface used for communication on a single connection.
class adapter_i
{
    WC_INTERFACE_CLASS(adapter_i)

public:
    //------------------------------------------------------------------------------
    /// Send the given bytes on this connection.
    ///
    /// \note
    ///   The bytes have to be a complete message to be understood by the counterpart.
    ///   The bytes may not be transmitted using multiple send calls, as data may be receivable as soon as a send call
    ///   returns.
    ///
    /// \param message
    ///   Message to send.
    /// \param handler
    ///   Handler function called when successfully sent or failed to send the message
    //------------------------------------------------------------------------------
    virtual void send(message_data &&message,
                      send_handler   handler) = 0;

    //------------------------------------------------------------------------------
    /// Received a message on this connection.
    ///
    /// \param handler
    ///   Handler function to use for a received message.
    //------------------------------------------------------------------------------
    virtual void receive(receive_handler handler) = 0;

    //------------------------------------------------------------------------------
    /// Get an info object about this connection.
    ///
    /// \return
    ///   Connection information object.
    //------------------------------------------------------------------------------
    virtual connection_info get_connection_info() const = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_ADAPTER_I_HPP_
//---- End of source file ------------------------------------------------------
