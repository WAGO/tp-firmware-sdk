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
///  \brief    Protected IPC adapter.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_ASIO_PROTECTED_ADAPTER_HPP_
#define SRC_LIBWDXLINUXOSCOM_ASIO_PROTECTED_ADAPTER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/adapter_i.hpp"

#include <mutex>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class asio_adapter;

class protected_adapter final : public adapter_i
{
private:
    std::weak_ptr<asio_adapter> adapter_m;
    std::shared_ptr<std::mutex> mutex_m;

public:
    protected_adapter(std::weak_ptr<asio_adapter> adapter,
                      std::shared_ptr<std::mutex> mutex);

    // Interface adapter_i
    void send(message_data &&message,
              send_handler   handler) override;
    void receive(receive_handler handler) override;
    connection_info get_connection_info() const override;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_ASIO_PROTECTED_ADAPTER_HPP_
//---- End of source file ------------------------------------------------------
