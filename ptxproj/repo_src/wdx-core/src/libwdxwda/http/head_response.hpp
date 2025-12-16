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
///  \brief    HTTP response with header information only.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_HTTP_HEAD_RESPONSE_HPP_
#define SRC_LIBWDXWDA_HTTP_HEAD_RESPONSE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/response_i.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

class head_response : public response_i
{
private:
    http_status_code    const status_code_m;
    map<string, string>       response_header_m;
    string                    empty_m;

public:
    explicit head_response(http_status_code const status_code);
    head_response(http_status_code    const  status_code,
                  map<string, string> const &response_header);
    ~head_response() noexcept override = default;

    http_status_code            get_status_code()     const override;
    map<string, string> const & get_response_header() const override;
    string              const & get_content_type()    const override;
    string              const & get_content_length()  const override;
    bool                        has_content()         const override;
    string                      get_content()         const override;

    void set_content_type(string const & content_type);
    void set_content_length(string const & content_length); 
    void set_header(string const & header_name, string const & header_value);
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_HTTP_HEAD_RESPONSE_HPP_
//---- End of source file ------------------------------------------------------
