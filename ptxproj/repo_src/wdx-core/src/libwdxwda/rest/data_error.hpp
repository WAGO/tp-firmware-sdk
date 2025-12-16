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
///  \brief    Error object to represent a data error which is deliverable via
///            REST-API.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_DATA_ERROR_HPP_
#define SRC_LIBWDXWDA_REST_DATA_ERROR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "core_error.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class data_error : public core_error
{
private:
    int    const data_index_m;
public:
    template <class WdaResposne, class WdaRequest>
    data_error(WdaResposne const &response, 
               WdaRequest  const &request,
               int                request_data_index = -1)
    : core_error(response, request)
    , data_index_m(request_data_index)
    {}

    data_error(wdx::status_codes core_status,
               std::string       message,
               uint16_t          domain_status,
               std::string       resource_type_description,
               std::string       resource_id,
               int               request_data_index)
    : core_error(core_status,
                 std::move(message),
                 domain_status,
                 std::move(resource_type_description),
                 std::move(resource_id),
                 true)
    , data_index_m(request_data_index)
    { }

    int         get_data_index() const;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_DATA_ERROR_HPP_
//---- End of source file ------------------------------------------------------
