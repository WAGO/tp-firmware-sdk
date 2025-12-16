//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2020-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Data stream based on a std::vector as data buffer.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_DATA_STREAM_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_DATA_STREAM_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <cstdint>

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
using data_ostream  = std::basic_ostream<uint8_t>;
using data_istream  = std::basic_istream<uint8_t>;
using data_iostream = std::basic_iostream<uint8_t>;


class data_stream final : public data_iostream
{
private:
    class memory_buffer : public std::basic_streambuf<uint8_t>
    {
    private:
        std::vector<uint8_t> &buffer_m;

    public:
        explicit memory_buffer(std::vector<uint8_t> &buffer);
        std::streamsize xsputn(uint8_t         const *data,
                               std::streamsize        n) override;
        int_type overflow(int_type c) override;

    private:
        void set_read_pointers(ssize_t current_pos_offset);
    };

public:
    explicit data_stream(std::vector<uint8_t> &buffer);

private:
    memory_buffer buffer_m;
};


class data_input_stream : public data_istream
{
private:
    class memory_buffer : public std::basic_streambuf<uint8_t>
    {
    private:
        std::vector<uint8_t> const &buffer_m;

    public:
        explicit memory_buffer(std::vector<uint8_t> const &buffer);
    };

public:
    explicit data_input_stream(std::vector<uint8_t> const &buffer);

private:
    memory_buffer buffer_m;
};


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_DATA_STREAM_HPP_
//---- End of source file ------------------------------------------------------

