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

#ifndef INC_WAGO_WDX_PARAMETER_EXCEPTION_HPP_
#define INC_WAGO_WDX_PARAMETER_EXCEPTION_HPP_

#include <stdexcept>
#include "wago/wdx/status_codes.hpp"

namespace wago {
namespace wdx {

/**
 Meant to stay inside the parameter service.
 It could bubble outside, but that would be a bug.
 */
class parameter_exception : public std::runtime_error
{
public:
    parameter_exception(std::string const &message);
    parameter_exception(status_codes error);
    parameter_exception(status_codes error, std::string const &message);

    status_codes get_error_code() const;

private:
    status_codes error_code;
};

}
}
#endif // INC_WAGO_WDX_PARAMETER_EXCEPTION_HPP_
