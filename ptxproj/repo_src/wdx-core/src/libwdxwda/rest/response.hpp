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
///  \brief    General response with serializable content.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_RESPONSE_HPP_
#define SRC_LIBWDXWDA_REST_RESPONSE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/response_i.hpp"
#include "wago/wdx/wda/http/http_status_code.hpp"
#include "wago/wdx/wda/service_identity_i.hpp"
#include "serializer_i.hpp"
#include "collection_document.hpp"
#include "basic_document.hpp"
#include "core_exception.hpp"
#include "basic_resource.hpp"
#include "method_invocation.hpp"

#include <wc/assertion.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using http::http_status_code;
using http::response_i;

template<class T>
class response : public response_i
{
private:
    http_status_code    const  status_code_m;
    map<string, string>        response_header_m;
    std::string                content_type_m;
    std::string                content_length_m;
    std::string                content_m;

public:
    response(http_status_code const   status_code,
             serializer_i     const  &serializer,
             T                const &&data)
    : status_code_m(status_code)
    , content_type_m(serializer.get_content_type())
    {
        serializer.serialize(content_m, data);
        content_length_m = std::to_string(content_m.size());
        if(!content_m.empty())
        {
            WC_ASSERT(!content_type_m.empty());
            WC_ASSERT(!content_length_m.empty());
            response_header_m.emplace("Content-Type", content_type_m);
            response_header_m.emplace("Content-Length", content_length_m);
        }
    }

    ~response() noexcept override = default;

    http_status_code get_status_code() const override
    {
        return status_code_m;
    }

    map<string, string> const & get_response_header() const override
    {
        return response_header_m;
    }

    string const & get_content_type() const override
    {
        return content_type_m;
    }

    string const & get_content_length() const override
    {
        return content_length_m;
    }

    bool has_content() const override
    {
        return !content_m.empty();
    }

    string get_content() const override
    {
        return content_m;
    }
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_RESPONSE_HPP_
//---- End of source file ------------------------------------------------------
