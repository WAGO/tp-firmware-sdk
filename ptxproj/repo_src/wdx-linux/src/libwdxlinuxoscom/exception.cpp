//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of WAGO Parameter Service communication base 
///            exception class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/com/exception.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
exception::exception(string const &message) noexcept
: std::runtime_error(message)
{ }

exception::exception(std::exception const &) noexcept
: exception("Unknown exception occurred")
{ }

exception::exception(std::runtime_error const &e) noexcept
: exception(e.what())
{ }

remote_exception::remote_exception(string const &message) noexcept
: std::runtime_error(message)
{ }


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
