//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    FCGI server for WAGO auth service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_FCGI_SERVER_HPP_
#define SRC_LIBAUTHSERV_FCGI_SERVER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/server_i.hpp"
#include <wc/structuring.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {
  class request_handler_i;
}
}

namespace authserv {
namespace fcgi {

using authserv::http::request_handler_i;


class server final : public server_i
{
private:
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(server)

public:
    server(std::string       const &fcgi_socket,
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
    request_handler_i       &handler_m;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace fcgi
} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_FCGI_SERVER_HPP_
//---- End of source file ------------------------------------------------------
