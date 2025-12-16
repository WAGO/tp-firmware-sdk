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
///  \brief    Exception object to represent data errors which is deliverable through
///            our REST-API responses and carries an additional error information.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_DATA_EXCEPTION_HPP_
#define SRC_LIBWDXWDA_REST_DATA_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "http/http_exception.hpp"
#include "data_error.hpp"

#include <string>
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using http::http_status_code;
using data_error_vector = std::vector<std::shared_ptr<data_error>>;

class data_exception : public http::http_exception
{
private:
    data_error_vector const data_errors_m;

public:
    data_exception(std::string      const &message,
                   data_error       const &error,
                   http_status_code const  http_status);

    data_exception(std::string      const &message,
                   data_error       const &error);

    data_exception(std::string       const &message,
                   data_error_vector const &errors,
                   http_status_code  const  http_status);

    data_exception(std::string       const &message,
                   data_error_vector const &errors);

    data_error_vector const &get_errors() const;
    std::string              get_title()  const override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_DATA_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
