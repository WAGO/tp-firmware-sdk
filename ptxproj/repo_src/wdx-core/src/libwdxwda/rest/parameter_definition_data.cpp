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
///  \brief    Implementation of parameter definition data class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "parameter_definition_data.hpp"
#include "http/http_exception.hpp"
#include <wc/assertion.h>

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
parameter_definition_data::parameter_definition_data(wdx::parameter_response const &response)
: id_m(response.path)
, definition_m(response.definition)
{
    WC_ASSERT(definition_m.get() != nullptr);
    if(definition_m.get() == nullptr)
    {
        throw http::http_exception("Unexpected empty parameter definition.", http::http_status_code::internal_server_error);
    }
}

wdx::parameter_instance_path parameter_definition_data::get_associated_parameter() const
{
    return id_m;
}

std::shared_ptr<wdx::parameter_definition> parameter_definition_data::get_definition() const
{
    return definition_m;
}


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
