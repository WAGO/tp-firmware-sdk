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
///  \brief    Implementation of token document response.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "token_document.hpp"

#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {
std::string const token_type   = "Bearer"; // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
std::string const content_type = "application/json;charset=UTF-8"; // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------
void init_document_content(std::string       &content,
                           std::string       &content_length,
                           std::string const &access_token,
                           std::string const &refresh_token,
                           uint32_t           access_token_lifetime_s,
                           std::string const &password_expired = "");
void init_document_headers(http::headermap &response_header);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
token_document::token_document(std::string const &access_token,
                               uint32_t           access_token_lifetime_s)
{
    init_document_content(content_m, content_length_m, access_token, "", access_token_lifetime_s);
    init_document_headers(response_header_m);
}

token_document::token_document(std::string const &access_token,
                               std::string const &refresh_token,
                               uint32_t           access_token_lifetime_s)
{
    init_document_content(content_m, content_length_m, access_token, refresh_token, access_token_lifetime_s);
    init_document_headers(response_header_m);
}

token_document::token_document(std::string const &access_token,
                               std::string const &refresh_token,
                               uint32_t           access_token_lifetime_s,
                               bool               password_expired)
{
    init_document_content(content_m, content_length_m, access_token, refresh_token, access_token_lifetime_s, password_expired ? "true" : "false");
    init_document_headers(response_header_m);
}

http::http_status_code token_document::get_status_code() const
{
    return http::http_status_code::ok;
}

http::headermap const & token_document::get_response_header() const
{
    return response_header_m;
}

std::string const & token_document::get_content_type() const
{
    return content_type;
}

std::string const & token_document::get_content_length() const
{
    return content_length_m;
}

std::string token_document::get_content() const
{
    return content_m;
}

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
void init_document_content(std::string       &content,
                           std::string       &content_length,
                           std::string const &access_token,
                           std::string const &refresh_token,
                           uint32_t           access_token_lifetime_s,
                           std::string const &password_expired)
{
    WC_ASSERT(!access_token.empty());
    std::string const token_expire_lifetime = std::to_string(access_token_lifetime_s);

    content =                     "{"
                                      "\"token_type\":\""       + token_type            + "\"," +
                                      "\"access_token\":\""     + access_token          + "\"," +
    (refresh_token.empty() ? "" :     "\"refresh_token\":\""    + refresh_token         + "\",") +
    (password_expired.empty() ? "" :  "\"password_expired\":"   + password_expired        + ",") +
                                      "\"expires_in\":"         + token_expire_lifetime +   ""
                                  "}";
    content_length = std::to_string(content.length());
}

void init_document_headers(http::headermap &response_header)
{
    // Add headers to protect sensible information as required by RFC 6749
    // (https://www.rfc-editor.org/rfc/rfc6749#section-5.1)
    response_header.emplace("Cache-Control", "no-store");
    response_header.emplace("Pragma",        "no-cache");
}
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
