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
///            REST-API and occurred in a resource relationship.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_RELATIONSHIP_ERROR_HPP_
#define SRC_LIBWDXWDA_REST_RELATIONSHIP_ERROR_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "data_error.hpp"

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class relationship_error : public data_error
{
private:
    std::string const relationship_name_m;
    int         const relationship_index_m;
public:

    template <class WdaResposne, class WdaRequest>
    relationship_error(WdaResposne const &response, 
                       WdaRequest  const &request,
                       int                request_data_index,
                       std::string        request_relationship,
                       int                request_relationship_index = -1)
    : data_error(response, request, request_data_index)
    , relationship_name_m(request_relationship)
    , relationship_index_m(request_relationship_index)
    { }

    template <class WdaResposne, class WdaRequest>
    relationship_error(WdaResposne const &response, 
                       WdaRequest  const &request,
                       std::string        request_relationship,
                       int                request_relationship_index = -1)
    : data_error(response, request)
    , relationship_name_m(request_relationship)
    , relationship_index_m(request_relationship_index)
    { }

    relationship_error(wdx::status_codes core_status,
                       std::string       message,
                       uint16_t          domain_status,
                       std::string       resource_type,
                       std::string       resource_id,
                       int               request_data_index,
                       std::string       request_relationship,
                       int               request_relationship_index)
    : data_error(core_status,
                 message,
                 domain_status,
                 resource_type,
                 resource_id,
                 request_data_index)
    , relationship_name_m(request_relationship)
    , relationship_index_m(request_relationship_index)
    { }

    std::string get_relationship_name() const;
    int         get_relationship_index() const;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_RELATIONSHIP_ERROR_HPP_
//---- End of source file ------------------------------------------------------
