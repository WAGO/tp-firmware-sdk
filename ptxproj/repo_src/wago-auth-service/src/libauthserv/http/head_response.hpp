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
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_HTTP_HEAD_RESPONSE_HPP_
#define SRC_LIBAUTHSERV_HTTP_HEAD_RESPONSE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/http/response_i.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace http {

class head_response : public response_i
{
private:
    http_status_code const status_code_m;
    headermap              response_header_m;
    std::string            empty_m;

public:
    explicit head_response(http_status_code const status_code);
    head_response(http_status_code const status_code,
                  headermap              response_header);
    ~head_response() noexcept override;

    http_status_code         get_status_code()     const override;
    headermap        const & get_response_header() const override;
    std::string      const & get_content_type()    const override;
    std::string      const & get_content_length()  const override;
    std::string              get_content()         const override;

    void set_content_type(  std::string const &content_type);
    void set_content_length(std::string const &content_length);
    void set_header(        std::string const &header_name,
                            std::string const &header_value);
};


} // Namespace http
} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_HTTP_HEAD_RESPONSE_HPP_
//---- End of source file ------------------------------------------------------
