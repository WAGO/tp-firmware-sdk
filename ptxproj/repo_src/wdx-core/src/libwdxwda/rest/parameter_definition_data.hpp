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
///  \brief    Container for parameter definition data from the core API.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_REST_PARAMETER_DEFINITION_DATA_HPP_
#define SRC_LIBWDXWDA_REST_PARAMETER_DEFINITION_DATA_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/responses.hpp"
#include <wc/std_includes.h>

#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {
namespace rest {

class parameter_definition_data {
private:
    wdx::parameter_instance_path               const id_m;
    std::shared_ptr<wdx::parameter_definition> const definition_m;

public:
    parameter_definition_data(wdx::parameter_response const &response);

    wdx::parameter_instance_path               get_associated_parameter() const;
    std::shared_ptr<wdx::parameter_definition> get_definition() const;
};


} // Namespace rest
} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_REST_PARAMETER_DEFINITION_DATA_HPP_
//---- End of source file ------------------------------------------------------
