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
///  \brief    Implementation of data stream based on a std::vector as data buffer.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "data_stream.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// data_stream class implementation
//------------------------------------------------------------------------------
data_stream::memory_buffer::memory_buffer(std::vector<uint8_t> &buffer)
: buffer_m(buffer)
{
    uint8_t * const data_internal = buffer_m.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    setg(data_internal, data_internal, data_internal + buffer_m.size());
}

std::streamsize data_stream::memory_buffer::xsputn(uint8_t         const *data,
                                                   std::streamsize        n)
{
    ssize_t const read_offset = gptr() - buffer_m.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    buffer_m.insert(buffer_m.end(), data, data + n);
    set_read_pointers(read_offset);
    return n;
}

data_stream::memory_buffer::int_type data_stream::memory_buffer::overflow(int_type const c)
{
    uint8_t const new_byte = (c == traits_type::eof()) ? 0 : static_cast<uint8_t>(c);
    ssize_t const read_offset = gptr() - buffer_m.data();
    buffer_m.push_back(new_byte);
    set_read_pointers(read_offset);
    return new_byte;
}

void data_stream::memory_buffer::set_read_pointers(ssize_t const current_pos_offset)
{
    uint8_t * const data_internal = buffer_m.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint8_t * const data_next = data_internal + current_pos_offset;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    setg(data_internal, data_next, data_internal + buffer_m.size());
}

data_stream::data_stream(std::vector<uint8_t> &buffer)
: std::basic_iostream<uint8_t>(&buffer_m)
, buffer_m(buffer)
{
    rdbuf(&buffer_m);
}

//------------------------------------------------------------------------------
// data_input_stream class implementation
//------------------------------------------------------------------------------
data_input_stream::memory_buffer::memory_buffer(std::vector<uint8_t> const &buffer)
: buffer_m(buffer)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto * const data_internal = const_cast<uint8_t *>(buffer_m.data()); // parasoft-suppress CERT_CPP-EXP55-a-2 "We are removing const qualifier here, because setg expect non-const ptrs even for read"
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    setg(data_internal, data_internal, data_internal + buffer_m.size());
}

data_input_stream::data_input_stream(std::vector<uint8_t> const &buffer)
: std::basic_istream<uint8_t>(&buffer_m)
, buffer_m(buffer)
{
    rdbuf(&buffer_m);
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
