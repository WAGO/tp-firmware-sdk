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
///  \brief    Message receiver interface.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_RECEIVER_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_RECEIVER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using message_data = std::vector<uint8_t>;

/// Message based I/O interface (receiver part) used for inter process communication.
class receiver_i {
    WC_INTERFACE_CLASS(receiver_i)

public:
    //------------------------------------------------------------------------------
    /// Receive message data.
    ///
    /// \param message
    ///   Message with received data.
    //------------------------------------------------------------------------------
    virtual void receive(message_data message) = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_RECEIVER_I_HPP_
//---- End of source file ------------------------------------------------------
