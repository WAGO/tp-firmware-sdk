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
///  \brief    Error document response for WAGO auth service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_ERROR_DOCUMENT_HPP_
#define SRC_LIBAUTHSERV_ERROR_DOCUMENT_HPP_

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


// Error document specified at RFC 6749:
// https://www.rfc-editor.org/rfc/rfc6749#section-5.2
class error_document : public http::response_i
{
public:
    error_document(error_document const &);
    error_document(http::http_status_code        status, 
                   std::string            const &type,
                   std::string            const &description = "",
                   std::string            const &uri = "");
    ~error_document() noexcept override;

    http::http_status_code         get_status_code()     const override;
    http::headermap        const & get_response_header() const override;
    std::string            const & get_content_type()    const override;
    std::string            const & get_content_length()  const override;
    std::string                    get_content()         const override;

private:
    http::headermap              response_header_m;
    http::http_status_code       status_code_m;
    std::string                  content_m;
    std::string                  content_length_m;
};


} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_ERROR_DOCUMENT_HPP_
//---- End of source file ------------------------------------------------------
