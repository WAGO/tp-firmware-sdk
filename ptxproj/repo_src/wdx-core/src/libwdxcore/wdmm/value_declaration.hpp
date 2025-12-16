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
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXCORE_WDMM_VALUE_DECLARATION_HPP_
#define SRC_LIBWDXCORE_WDMM_VALUE_DECLARATION_HPP_

#include <memory>
#include "wago/wdx/parameter_instance_id.hpp"
#include "wago/wdx/parameter_value.hpp"


namespace wago {
namespace wdx {
inline namespace wdmm {

struct value_declaration
{
    parameter_instance_id id;
    std::shared_ptr<parameter_value> value;
    bool status_unavailable_if_not_provided = false;
};

}}}
#endif // SRC_LIBWDXCORE_WDMM_VALUE_DECLARATION_HPP_
