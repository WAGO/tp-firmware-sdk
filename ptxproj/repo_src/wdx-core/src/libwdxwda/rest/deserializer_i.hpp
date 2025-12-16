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
///  \brief    Deserialization interface
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_DESERIALIZER_I_HPP_
#define SRC_LIBWDXWDA_REST_DESERIALIZER_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "method_invocation.hpp"
#include "create_monitoring_list_request.hpp"

#include "wago/wdx/requests.hpp"
#include <wc/structuring.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

using std::string;

class deserializer_i
{
    WC_INTERFACE_CLASS(deserializer_i)

public:
    virtual string const & get_content_type() const = 0;

    virtual void deserialize(method_invocation_in_args       &request,
                             string                    const &deserializable) const = 0;

    virtual void deserialize(create_monitoring_list_request       &request,
                             string                         const &deserializable) const = 0;

    virtual void deserialize(wdx::value_path_request  &parameter_value,
                             string             const &deserializable) const = 0;

    virtual void deserialize(std::vector<wdx::value_path_request>       &parameter_values,
                             string                               const &deserializable) const = 0;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_DESERIALIZER_I_HPP_
//---- End of source file ------------------------------------------------------
