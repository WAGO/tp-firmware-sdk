//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of REST specific data exception class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "data_exception.hpp"

#include <wc/structuring.h>
#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::make_shared;

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static http_status_code get_most_general_status_code(data_error_vector const &errors);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
data_exception::data_exception(string           const &message,
                               data_error       const &error,
                               http_status_code const  http_status)
: data_exception(message, data_error_vector({ make_shared<data_error>(error) }), http_status)
{
    return;
}

data_exception::data_exception(string           const &message,
                               data_error       const &error)
: data_exception(message, error, error.get_http_status_code())
{
    return;
}

data_exception::data_exception(std::string       const &message,
                               data_error_vector const &errors,
                               http_status_code  const  http_status)
: http::http_exception(message, http_status)
, data_errors_m(errors)
{
    WC_ASSERT(!data_errors_m.empty());
    return;
}

data_exception::data_exception(std::string       const &message,
                               data_error_vector const &errors)
: data_exception(message, errors, get_most_general_status_code(errors))
{
    return;
}

data_error_vector const &data_exception::get_errors() const
{
    return data_errors_m;
}

std::string data_exception::get_title() const
{
    return http::http_exception::get_title() + ": Data Error";
}

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

static http_status_code get_most_general_status_code(data_error_vector const &errors)
{
    if (errors.empty())
    {
        return http_status_code::internal_server_error;
    }
    auto http_code = errors.at(0)->get_http_status_code();
    for (auto const &error : errors)
    {
        auto next_http_code = error->get_http_status_code();
        // specific code if equal
        if (http_code != next_http_code)
        {
            // reduce code precision and return the higher one, if not equal then
            auto general_http_code      = http_status_code(static_cast<uint16_t>(http_code)      - (static_cast<uint16_t>(http_code)  % 100));     // parasoft-suppress CERT_CPP-INT50-a-3 "HTTP status codes are exlicitly designed that a general code x00 always exists"
            auto general_next_http_code = http_status_code(static_cast<uint16_t>(next_http_code) - (static_cast<uint16_t>(next_http_code) % 100)); // parasoft-suppress CERT_CPP-INT50-a-3 "HTTP status codes are exlicitly designed that a general code x00 always exists"
            http_code = general_http_code > general_next_http_code ? general_http_code : general_next_http_code;
        }
    }
    return http_code;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
