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
///  \brief    Boost ASIO client for asynchronous I/O operations.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_CLIENT_HPP_
#define SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_CLIENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio_context.hpp"
#include "protected_adapter.hpp"
#include "common/client_i.hpp"

#include <wc/structuring.h>

#include <string>
#include <vector>
#include <memory>
#include <atomic>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

class file_notifier;

using protocol = boost::asio::local::stream_protocol;
using boost::system::error_code;

class asio_client final : public client_i, public asio_context
{
private:
    std::string                    const connection_name_m;
    protocol::endpoint                   endpoint_m;
    std::shared_ptr<asio_adapter>        connection_m;
    std::unique_ptr<file_notifier>       socket_notifier_m;
    std::atomic<bool>                    is_connected_m;
    std::shared_ptr<receiver_i>          receiver_m;
    std::shared_ptr<std::mutex>          mutex_m;
    connect_notifier                     on_connect_m;

    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(asio_client)

public:
    asio_client(std::string const &connection_name,
                std::string const &socket_path);
    asio_client(std::string                 const &connection_name,
                std::string                 const &socket_path,
                std::shared_ptr<io_context>        context);
    ~asio_client() noexcept override;

    std::shared_ptr<io_context> &get_context();

    // Interface client_i
    bool is_connected() const override;

    // Interface driver_i
    bool run_once(uint32_t timeout_ms = no_wait) override; // NOLINT(google-default-arguments)
    bool run() override;
    void stop() override;

private:
    void wait_for_socket();
    void do_connect_socket();
    void do_connect_internal();
    std::unique_ptr<adapter_i> do_connect(connect_notifier on_connect) override;
    void set_receiver(std::unique_ptr<receiver_i> receiver) override;
    std::shared_ptr<receiver_i> get_receiver(std::shared_ptr<asio_adapter> adapter) override;
    void on_disconnect(std::shared_ptr<asio_adapter> adapter) override;
    void try_reconnect();
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_CLIENT_HPP_
//---- End of source file ------------------------------------------------------
