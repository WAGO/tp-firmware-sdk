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
///  \brief    Boost ASIO server for asynchronous I/O operations.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_SERVER_HPP_
#define SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_SERVER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "asio_context.hpp"
#include "asio_adapter.hpp"
#include "common/receiver_i.hpp"
#include "common/server_i.hpp"

#include <wc/structuring.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using protocol = boost::asio::local::stream_protocol;
using boost::system::error_code;

class asio_server final : public server_i, public asio_context
{
public:
    static constexpr uid_t const invalid_uid = std::numeric_limits<uid_t>::max();
    static constexpr gid_t const invalid_gid = std::numeric_limits<gid_t>::max();

private:
    using connection_map = std::map<std::shared_ptr<asio_adapter>, std::shared_ptr<receiver_i>>;

    std::string        const connection_name_m;
    protocol::acceptor       acceptor_m;
    connection_map           connections_m;
    std::mutex               connections_mutex_m;

    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(asio_server)

public:
    asio_server(std::string const &connection_name,
                std::string const &socket,
                uid_t              socket_owner = invalid_uid,
                gid_t              socket_group = invalid_gid);
    ~asio_server() noexcept override;

    bool run_once(uint32_t timeout_ms = no_wait) override; // NOLINT(google-default-arguments)
    bool run() override;
    void stop() override;

    void start(connection_handler<receiver_i> handler) override;
    std::vector<int> get_native_handles() override;

private:
    void do_accept(connection_handler<receiver_i>        handler);
    void on_accept(error_code                     const &code,
                   std::shared_ptr<asio_adapter>         new_connection,
                   connection_handler<receiver_i>        handler);

    std::shared_ptr<receiver_i> get_receiver(std::shared_ptr<asio_adapter> adapter) override;
    void on_disconnect(std::shared_ptr<asio_adapter> adapter) override;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_SERVER_HPP_
//---- End of source file ------------------------------------------------------
