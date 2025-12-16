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
///  \brief    Implementation of FCGI specific content stream.
///
///  \author   PEn: WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "content_stream.hpp"
#include "system_abstraction.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace fcgi {


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------

content_buffer::content_buffer(FCGX_Request *request, size_t buffer_size)
: request_m(request)
, current_m(buffer_size)
, current_length_m(0)
{ }

std::streambuf::int_type content_buffer::underflow()
{
    // read string from fcgi buffer
    WC_ASSERT(current_m.size() <= static_cast<size_t>(std::numeric_limits<int>::max()));
    current_length_m = sal::fcgi::get_instance().GetStr(current_m.data(), static_cast<int>(current_m.size()), request_m->in);
    if (current_length_m == 0)
    {
        return std::streambuf::traits_type::eof();
    }
    setg(current_m.data(), current_m.data(), current_m.data() + current_length_m);
    return traits_type::to_int_type(*gptr());
}

std::streambuf::int_type content_buffer::overflow(std::streambuf::int_type WC_UNUSED_PARAM(banane))
{
    throw std::runtime_error("cannot write in buffer");
}

int content_buffer::sync()
{
    // noop as the buffer id read only
    return 0;
}

} // Namespace fcgi
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
