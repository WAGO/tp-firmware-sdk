//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of WAGO Auth Service base exception class.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/exception.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
exception::exception(std::exception const &)
: exception("Unknown exception occurred")
{ }

exception::exception(std::runtime_error const &e)
: exception(e.what())
{ }

exception::exception(std::string const &message)
: std::runtime_error(message)
{ }


} // Namespace paramserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
