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
///  \brief    Token handler to build and unpack Bearer tokens.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_TOKEN_HANDLER_HPP_
#define SRC_LIBAUTHSERV_TOKEN_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/settings_store_i.hpp"
#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class token_handler
{
public:
    explicit token_handler(settings_store_i &settings_store);

    ~token_handler() noexcept;
    std::string build_token(      std::string const &payload);
    std::string get_token_payload(std::string const &token);

    //------------------------------------------------------------------------------
    ///
    /// \remark regenerates the key used to generate all tokens
    ///
    /// Note: all previous generated tokens become invalid
    //------------------------------------------------------------------------------
    void regenerate_key();

private:
    struct handler_impl;
    std::unique_ptr<handler_impl> impl;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_TOKEN_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
