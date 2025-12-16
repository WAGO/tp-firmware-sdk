//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of file exception.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/linuxos/file/exception.hpp"

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

//------------------------------------------------------------------------------
// Class implementation
//------------------------------------------------------------------------------
exception::exception(wdx::status_codes const  code,
                     std::string       const &message) noexcept
: std::runtime_error(message)
, code_m(code)
{}

wdx::status_codes exception::get_status_code() const noexcept
{
    return code_m;
}

//------------------------------------------------------------------------------
// Function implementation
//------------------------------------------------------------------------------
wdx::linuxos::file::exception get_exception_from_ptr(std::exception_ptr const e_ptr) noexcept
{
    try
    {
        if(e_ptr)
        {
            std::rethrow_exception(e_ptr);
        }
        throw std::runtime_error("Ask to get exception message from nullptr");
    }
    catch(wdx::linuxos::file::exception const &e)
    {
        return e;
    }
    catch(std::exception const &e)
    {
        return wdx::linuxos::file::exception(wdx::status_codes::internal_error, e.what());
    }
}

} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


//---- End of source file ------------------------------------------------------
