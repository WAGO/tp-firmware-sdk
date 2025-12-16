//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Base exception to be thrown in WAGO Parameter Service library.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_EXCEPTION_HPP_
#define INC_WAGO_WDX_WDA_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wago/wdx/parameter_exception.hpp>

#include <string>
#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {

/// WDA base class for exceptions.
using exception = ::wago::wdx::parameter_exception;

} // Namespace wdx

namespace wdx {
namespace wda {

using std::string;

class exception : public std::runtime_error
{
public:
    explicit exception(string const &message);
    explicit exception(std::exception const &e);
    explicit exception(std::runtime_error const &e);

    ~exception() noexcept override = default;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
