//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Boost ASIO IPC adapter.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_ADAPTER_HPP_
#define SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_ADAPTER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "common/adapter_i.hpp"

#include <wc/structuring.h>
#include <boost/asio.hpp>

#include <queue>
#include <atomic>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using protocol      = boost::asio::local::stream_protocol;
using send_element  = std::tuple<message_data, uint32_t, send_handler>;
using send_queue    = std::queue<send_element>;
using receive_count = std::atomic<signed>;
using close_handler = std::function<void()>;
using exit_marker   = std::shared_ptr<std::atomic<bool>>;
using exit_mutex    = std::shared_ptr<std::mutex>;

class asio_adapter final : public adapter_i
{
private:
    protocol::socket socket_m;
    uint32_t         current_in_message_size_m;
    message_data     current_in_message_m;
    std::mutex       current_in_mutex_m;
    send_queue       out_queue_m;
    receive_count    in_count_m;
    std::mutex       out_mutex_m;
    close_handler    close_handler_m;
    exit_marker      exit_marker_m;
    exit_mutex       exit_mutex_m;

    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(asio_adapter)

public:
    explicit asio_adapter(boost::asio::io_context &context);
    ~asio_adapter() noexcept override;

    protocol::socket & get_socket();
    void close(close_handler handler);
    bool is_closed();
    void reinit();

    // Interface adapter_i
    void send(message_data &&message,
              send_handler   handler) override;
    void receive(receive_handler handler) override;
    connection_info get_connection_info() const override;

private:
    void start_write();
    void receive_header(receive_handler handler);
    void receive_body(receive_handler handler);
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_ADAPTER_HPP_
//---- End of source file ------------------------------------------------------
