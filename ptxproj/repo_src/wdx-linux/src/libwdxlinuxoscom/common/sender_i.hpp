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
///  \brief    Message sender interface for managed objects.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_SENDER_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_SENDER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <vector>
#include <string>
#include <functional>
#include <limits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using message_data = std::vector<uint8_t>;
using send_handler = std::function<void(std::string error_message)>;

class managed_object;

/// Message based I/O interface (sender part) used for inter process communication.
class sender_i
{
    WC_INTERFACE_CLASS(sender_i)

public:
    /// Maximum data amount for sending.
    static constexpr ssize_t const max_send_data = std::numeric_limits<ssize_t>::max();

    //------------------------------------------------------------------------------
    /// Send the given message data.
    ///
    /// \param sender
    ///   Managed object who is the sender of this data.
    /// \param message
    ///   Message data to send.
    /// \param handler
    ///   Handler to call after message is send (success or error).
    //------------------------------------------------------------------------------
    virtual void send(managed_object const  &sender,
                      message_data   const &&message,
                      send_handler           handler) = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_SENDER_I_HPP_
//---- End of source file ------------------------------------------------------
