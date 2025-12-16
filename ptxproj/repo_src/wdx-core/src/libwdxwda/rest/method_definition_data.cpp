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
///  \brief    Implementation of method definition data class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "method_definition_data.hpp"
#include "http/http_exception.hpp"

#include "wago/wdx/wdmm/parameter_definition.hpp"
#include "wago/wdx/wdmm/method_definition.hpp"
#include "wda_ipc/representation.hpp"
#include <wc/assertion.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
method_definition_data::method_definition_data(wdx::parameter_response const &resp)
: id_m(resp.path)
, definition_m(std::dynamic_pointer_cast<method_definition>(resp.definition))
{
    WC_ASSERT(definition_m.get() != nullptr);
    if(definition_m.get() == nullptr)
    {
        throw http::http_exception("Unexpected empty method definition.", http::http_status_code::internal_server_error);
    }
}

std::string method_definition_data::get_id() const
{
    return wda_ipc::to_string(id_m);
}

std::string method_definition_data::get_path() const
{
    return id_m.parameter_path;
}

std::shared_ptr<wdx::method_definition> method_definition_data::get_method_definition() const
{
    return definition_m;
}

} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
