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
///  \brief    Token document response for WAGO auth service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_TOKEN_DOCUMENT_HPP_
#define SRC_LIBAUTHSERV_TOKEN_DOCUMENT_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/response_i.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {


class token_document : public http::response_i
{
private:
    http::headermap response_header_m;
    std::string     content_m;
    std::string     content_length_m;

public:
    token_document(std::string const &access_token,
                   uint32_t           access_token_lifetime_s);
    token_document(std::string const &access_token,
                   std::string const &refresh_token,
                   uint32_t           access_token_lifetime_s);
    token_document(std::string const &access_token,
                   std::string const &refresh_token,
                   uint32_t           access_token_lifetime_s,
                   bool               password_expired);
    http::http_status_code         get_status_code()     const override;
    http::headermap        const & get_response_header() const override;
    std::string            const & get_content_type()    const override;
    std::string            const & get_content_length()  const override;
    std::string                    get_content()         const override;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_TOKEN_DOCUMENT_HPP_
//---- End of source file ------------------------------------------------------
