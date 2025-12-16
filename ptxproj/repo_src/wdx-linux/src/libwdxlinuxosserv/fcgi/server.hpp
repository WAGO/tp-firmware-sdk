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
///  \brief    FCGI server for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_FCGI_SERVER_HPP_
#define SRC_LIBWDXLINUXOSSERV_FCGI_SERVER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/linuxos/serv/server_i.hpp>
#include <wago/wdx/wda/trace_routes.hpp>
#include <wc/structuring.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {
  class request_handler_i;
}
}
}

namespace wdx {
namespace linuxos {
namespace serv {
namespace fcgi {

using wdx::wda::http::request_handler_i;
using wdx::wda::trace_route;


class server final : public server_i
{
private:
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(server)

public:
    server(std::string       const &fcgi_socket,
           trace_route       const  route,
           request_handler_i       &handler);
    ~server() noexcept override;

    // re-declare interface extension 'receive_next(void)' to make it 
    // available on this class. Further details on this topic:
    // https://stackoverflow.com/a/9995567
    using server_i::receive_next;
    bool receive_next(uint32_t timeout) override;

    int get_native_handle() const override;

private:
    static constexpr int count_of_waiting_sockets = 5;

    int                      fcgi_socket_m;
    trace_route       const  route_m;
    request_handler_i       &handler_m;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace fcgi
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_FCGI_SERVER_HPP_
//---- End of source file ------------------------------------------------------
