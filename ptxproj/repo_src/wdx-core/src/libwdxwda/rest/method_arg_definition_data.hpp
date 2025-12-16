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
///  \brief    Wrapper for method argument definition responses from the core 
///            api.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_METHOD_ARG_DEFINITION_DATA_HPP_
#define SRC_LIBWDXWDA_REST_METHOD_ARG_DEFINITION_DATA_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/responses.hpp"
#include "wago/wdx/wdmm/method_argument_definition.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include <wc/compiler.h>

#include <string>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {


class method_arg_definition_data
{
public:
    enum class arg_type {
        in, out
    };

private:
    wdx::parameter_instance_path             method_definition_id_m;
    std::string                              arg_name_m;
    arg_type                                 arg_type_m;
    std::shared_ptr<wdx::method_definition>  definition_m;
    wdx::method_argument_definition         *arg_definition_m;

public:
    // allow implicit conversion
    method_arg_definition_data(wdx::parameter_response const &resp, 
                               std::string             const &arg_name,
                               arg_type                const &arg_type);
    
    std::string get_id() const;
    arg_type    get_arg_type() const;
    std::string get_lowercase_arg_name() const;
    std::string get_associated_method_definition() const;
    wdx::method_argument_definition * get_argument_definition() const;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// macros
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_METHOD_ARG_DEFINITION_DATA_HPP_
//---- End of source file ------------------------------------------------------
