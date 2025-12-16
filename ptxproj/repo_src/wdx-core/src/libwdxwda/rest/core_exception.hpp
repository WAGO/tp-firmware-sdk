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
///  \brief    Exception object to represent an error which is deliverable through
///            our REST-API responses and carries an additional core status code.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_CORE_EXCEPTION_HPP_
#define SRC_LIBWDXWDA_REST_CORE_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "http/http_exception.hpp"
#include "status.hpp"
#include "core_error.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class core_exception : public http::http_exception
{
private:
    core_error const error_m;

public:
    core_exception(core_error const &error);

    core_status_code   get_core_status_code()   const;
    domain_status_code get_domain_status_code() const;
    std::string        get_title()              const override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_CORE_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
