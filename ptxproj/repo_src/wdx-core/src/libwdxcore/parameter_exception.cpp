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
#include "wago/wdx/parameter_exception.hpp"

#include <utility>

namespace wago {
namespace wdx {

parameter_exception::parameter_exception(std::string const &message)
: std::runtime_error(message)
, error_code(status_codes::internal_error)
{}

parameter_exception::parameter_exception(status_codes error)
: std::runtime_error(to_string(error))
, error_code(error)
{}

parameter_exception::parameter_exception(status_codes error, std::string const &message)
: std::runtime_error(to_string(error) + ": " + message)
, error_code(error)
{}

status_codes parameter_exception::get_error_code() const
{
    return error_code;
}

}
}
