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
#ifndef SRC_LIBWDXWDA_REST_ATTRIBUTE_ERROR_HPP_
#define SRC_LIBWDXWDA_REST_ATTRIBUTE_ERROR_HPP_

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

class attribute_error : public data_error
{
private:
    std::string const attribute_name_m;
public:
    template <class WdaResposne, class WdaRequest>
    attribute_error(WdaResposne const &response, 
                    WdaRequest  const &request,
                    int                request_data_index,
                    std::string        request_attribute)
    : data_error(response, request, request_data_index)
    , attribute_name_m(std::move(request_attribute))
    {}

    template <class WdaResposne, class WdaRequest>
    attribute_error(WdaResposne const &response, 
                    WdaRequest  const &request,
                    std::string        request_attribute)
    : data_error(response, request)
    , attribute_name_m(std::move(request_attribute))
    {}

    attribute_error(wdx::status_codes core_status,
                    std::string       message,
                    uint16_t          domain_status,
                    std::string       resource_type,
                    std::string       resource_id,
                    int               request_data_index,
                    std::string       request_attribute)
    : data_error(core_status,
                 std::move(message),
                 domain_status,
                 std::move(resource_type),
                 std::move(resource_id),
                 request_data_index)
    , attribute_name_m(std::move(request_attribute))
    {}

    std::string get_attribute_name() const;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_ATTRIBUTE_ERROR_HPP_
//---- End of source file ------------------------------------------------------
