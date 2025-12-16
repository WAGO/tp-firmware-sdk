//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of errno utils.
///
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#include "errno_utils.hpp"
#include <cstring>

namespace wago {
namespace authserv {

std::string errno_to_string(int errno_num)
{
    char message_buffer[1024];
    return std::string(strerror_r(errno_num, message_buffer, sizeof(message_buffer)));
}

} // authserv
} // wago
