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
///  \brief    HTTP response with header information only.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "head_response.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

head_response::head_response(http_status_code const  status_code)
: status_code_m(status_code)
{ }

head_response::head_response(http_status_code const status_code,
                             headermap              response_header)
: status_code_m(status_code)
, response_header_m(std::move(response_header))
{ }

head_response::~head_response() noexcept = default;

http_status_code head_response::get_status_code() const
{
    return status_code_m;
}

headermap const & head_response::get_response_header() const
{
    return response_header_m;
}

std::string const & head_response::get_content_type() const
{
    return response_header_m.count("Content-Type") > 0
        ? response_header_m.at("Content-Type")
        : empty_m;
}

std::string const & head_response::get_content_length() const
{
    return response_header_m.count("Content-Length") > 0
        ? response_header_m.at("Content-Length")
        : empty_m;
}

std::string head_response::get_content() const
{
    return empty_m;
}

void head_response::set_content_type(std::string const &content_type)
{
    set_header("Content-Type", content_type);
}

void head_response::set_content_length(std::string const &content_length)
{
    set_header("Content-Length", content_length);
}

void head_response::set_header(std::string const & header_name,
                               std::string const &header_value)
{
    response_header_m.emplace(header_name, header_value);
}

} // Namespace http
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
