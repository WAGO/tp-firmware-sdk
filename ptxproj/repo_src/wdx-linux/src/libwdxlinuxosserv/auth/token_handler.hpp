//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Token handler to build and unpack Bearer tokens.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSSERV_AUTH_TOKEN_HANDLER_HPP_
#define SRC_LIBWDXLINUXOSSERV_AUTH_TOKEN_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace auth {

class token_handler
{
public:
    explicit token_handler(uint32_t const key_lifetime_s);
    ~token_handler() noexcept;

    std::string build_token(      std::string const &payload);
    std::string get_token_payload(std::string const &token);

private:
    struct handler_impl;
    std::unique_ptr<handler_impl> impl;

    void rotate_keys();
};


} // Namespace auth
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_AUTH_TOKEN_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
