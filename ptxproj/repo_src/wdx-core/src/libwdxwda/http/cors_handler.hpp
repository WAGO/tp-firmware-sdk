//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility functions to handle CORS requests.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_HTTP_CORS_HANDLER_HPP_
#define SRC_LIBWDXWDA_HTTP_CORS_HANDLER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/http/request_i.hpp"

#include <memory>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace http {

class cors_handler
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(cors_handler)

public:
    static std::unique_ptr<response_i> get_options_response(request_i                      &request,
                                                            std::vector<http_method> const &allowed_methods);
    static void add_cors_response_headers(request_i                      &request, 
                                          std::vector<http_method> const &allowed_methods,
                                          char const allowed_headers[],
                                          char const exposed_headers[]);
    static std::string get_allow_header(std::vector<http_method> const &allowed_methods);
};

} // Namespace http
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_HTTP_CORS_HANDLER_HPP_
//---- End of source file ------------------------------------------------------
