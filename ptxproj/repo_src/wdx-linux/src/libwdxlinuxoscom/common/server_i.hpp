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
///  \brief    Server context interface for asynchronous I/O operations.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_SERVER_I_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_SERVER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "adapter_i.hpp"
#include "receiver_i.hpp"

#include <wc/structuring.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

template<class T>
using connection_handler = std::function<std::unique_ptr<T>(std::unique_ptr<adapter_i> adapter)>;

class server_i : public driver_i
{
    WC_INTERFACE_CLASS(server_i)

public:
    virtual void start(connection_handler<receiver_i> handler) = 0;
    virtual std::vector<int> get_native_handles() = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_SERVER_I_HPP_
//---- End of source file ------------------------------------------------------
