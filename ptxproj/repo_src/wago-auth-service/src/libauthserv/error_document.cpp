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
///  \brief    Implementation of error document response.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "error_document.hpp"
#include <nlohmann/json.hpp>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {
std::string const content_type = "application/json;charset=UTF-8"; // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Any unexcpected exception in the init code will be noticed in the correspondig unit-tests."
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
error_document::error_document(error_document const &) = default;

error_document::error_document(http::http_status_code        status, 
                               std::string            const &type,
                               std::string            const &description,
                               std::string            const &uri)
: status_code_m(status)
{
    auto json_content = nlohmann::json({
        {"error", type}
    });

    if (!description.empty())
    {
        json_content.emplace("error_description", description);
    }
    if (!uri.empty())
    {
        json_content.emplace("error_uri", uri);
    }

    content_m = json_content.dump();
    content_length_m = std::to_string(content_m.length());
}

error_document::~error_document() noexcept = default;

http::http_status_code error_document::get_status_code() const
{
    return status_code_m;
}

http::headermap const & error_document::get_response_header() const
{
    return response_header_m;
}

std::string const & error_document::get_content_type() const
{
    return content_type;
}

std::string const & error_document::get_content_length() const
{
    return content_length_m;
}

std::string error_document::get_content() const
{
    return content_m;
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
