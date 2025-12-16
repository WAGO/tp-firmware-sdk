//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of method argument definition data class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "method_arg_definition_data.hpp"
#include "http/http_exception.hpp"

#include "wago/wdx/wdmm/method_definition.hpp"
#include "wda_ipc/representation.hpp"

#include <wc/assertion.h>

#include <algorithm>
#include <stdexcept>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

namespace
{
    std::string lowercase(std::string const& str)
    {
        std::string lower_str = str;
        std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
        return lower_str;
    }
}

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
method_arg_definition_data::method_arg_definition_data(wdx::parameter_response const &resp, 
                                                       std::string             const &arg_name,
                                                       arg_type                const &arg_type_)
: method_definition_id_m(resp.path)
, arg_name_m(lowercase(arg_name)) // TODO: save as lowercase?
, arg_type_m(arg_type_)
, definition_m(std::dynamic_pointer_cast<method_definition>(resp.definition))
, arg_definition_m(nullptr)
{
    WC_ASSERT(definition_m.get() != nullptr);
    if(definition_m.get() == nullptr)
    {
        throw http::http_exception("Unexpected empty method definition.", http::http_status_code::internal_server_error);
    }

    auto arg_it = (arg_type_m == arg_type::in )
        ? std::find_if(definition_m->in_args.begin(), definition_m->in_args.end(), [lower_arg_name=arg_name_m](auto const &arg) {
            return lowercase(arg.name) == lower_arg_name;
        })
        : std::find_if(definition_m->out_args.begin(), definition_m->out_args.end(), [lower_arg_name=arg_name_m](auto const &arg) {
            return lowercase(arg.name) == lower_arg_name;
        });
    auto arg_it_end = (arg_type_m == arg_type::in ) ? definition_m->in_args.end() : definition_m->out_args.end();
    if (arg_it == arg_it_end)
    {
        throw std::out_of_range("argument not found");
    }
    arg_definition_m = &*arg_it;
}

std::string method_arg_definition_data::get_id() const
{
    return get_associated_method_definition() + "-" + get_lowercase_arg_name();
}

method_arg_definition_data::arg_type method_arg_definition_data::get_arg_type() const
{
    return arg_type_m;
}

std::string method_arg_definition_data::get_lowercase_arg_name() const
{
    return arg_name_m;
}
std::string method_arg_definition_data::get_associated_method_definition() const
{
    return wda_ipc::to_string(method_definition_id_m);
}

wdx::method_argument_definition* method_arg_definition_data::get_argument_definition() const
{
    return arg_definition_m;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
