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
///  \brief    Implementation of verify document response.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "verify_document.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {
std::string const content_type = "application/json;charset=UTF-8";  // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."

//------------------------------------------------------------------------------
// internal function prototypes
//------------------------------------------------------------------------------
void init_document_content_invalid_token(std::string &content,
                                         std::string &content_length);

void init_document_content_valid_token(std::string       &content,
                                       std::string       &content_length,
                                       std::string const &username,
                                       std::string const &scopes,
                                       uint32_t           expire);
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
verify_document::verify_document()
{
    init_document_content_invalid_token(content_m, content_length_m);
}

verify_document::verify_document(std::string const &username,
                                 std::string const &scopes,
                                 uint32_t           expire)
{
    init_document_content_valid_token(content_m, content_length_m, username, scopes, expire);
}

verify_document::~verify_document() noexcept = default;

http::http_status_code verify_document::get_status_code() const
{
    return http::http_status_code::ok;
}

http::headermap const & verify_document::get_response_header() const
{
    return response_header_m;
}

std::string const & verify_document::get_content_type() const
{
    return content_type;
}

std::string const & verify_document::get_content_length() const
{
    return content_length_m;
}

std::string verify_document::get_content() const
{
    return content_m;
}

namespace {

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
void init_document_content_invalid_token(std::string &content,
                                         std::string &content_length)
{
    content = "{"
                  "\"active\":false"
              "}";
    content_length = std::to_string(content.length());
}

void init_document_content_valid_token(std::string       &content,
                                       std::string       &content_length,
                                       std::string const &username,
                                       std::string const &scopes,
                                       uint32_t           expire)
{
    content = "{"
                  "\"active\":"       "true"                     ","
                  "\"username\":\"" + username               + "\","
                  "\"scope\":\""    + scopes                 + "\","
                  "\"expires_in\":" + std::to_string(expire) +   ""
              "}";
    content_length = std::to_string(content.length());
}
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
