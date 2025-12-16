//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2019-2023 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Base exception to be thrown in WAGO Auth Service library.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_AUTHSERV_EXCEPTION_HPP_
#define INC_WAGO_AUTHSERV_EXCEPTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/authserv/api.h"

#include <string>
#include <stdexcept>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

class exception : public std::runtime_error
{
public:
    WC_INTERFACE_IMPL_CLASS(exception)

public:
    explicit exception(std::string const &message);
    explicit exception(std::exception const &e);
    explicit exception(std::runtime_error const &e);

};

} // Namespace authserv
} // Namespace wago


#endif // INC_WAGO_AUTHSERV_EXCEPTION_HPP_
//---- End of source file ------------------------------------------------------
