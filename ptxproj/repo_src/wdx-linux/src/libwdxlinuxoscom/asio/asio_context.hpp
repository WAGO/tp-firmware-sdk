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
///  \brief    Boost ASIO context for asynchronous I/O operations.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_CONTEXT_HPP_
#define SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_CONTEXT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/driver_i.hpp"
#include "common/receiver_i.hpp"
#include "asio_adapter.hpp"
#include "common/definitions.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <mutex>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

using boost::asio::io_context;

class asio_context : public driver_i
{
    friend class asio_client;

protected:
    std::shared_ptr<io_context> context_m;

public:
    explicit asio_context(std::shared_ptr<io_context> context);

    bool run_once(uint32_t timeout_ms = no_wait) override; // NOLINT(google-default-arguments)
    bool run() override;
    void stop() override;

protected:
    void on_receive(message_data                  const &message,
                    std::string                   const &error_message,
                    std::shared_ptr<asio_adapter>        adapter);

private:
    virtual std::shared_ptr<receiver_i> get_receiver(std::shared_ptr<asio_adapter> adapter) = 0;
    virtual void on_disconnect(std::shared_ptr<asio_adapter> adapter) = 0;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_ASIO_ASIO_CONTEXT_HPP_
//---- End of source file ------------------------------------------------------
