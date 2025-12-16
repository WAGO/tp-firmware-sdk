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
///  \brief    System abstraction implementation for hostname.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace sal {

class hostname_impl : public hostname
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(hostname_impl)
public:
    hostname_impl() noexcept = default;
    ~hostname_impl() noexcept override = default;

    int gethostname(char *out, size_t n) const noexcept override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static hostname_impl default_hostname;

hostname *hostname::instance = &default_hostname;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

int hostname_impl::gethostname(char *out, size_t n) const noexcept
{
    errno = 0;
    return ::gethostname(out, n);
}


} // Namespace sal
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
