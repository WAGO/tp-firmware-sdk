//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation of URL encoder.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "url_encode.hpp"

#include "wc/structuring.h"

#include <sstream>
#include <stdexcept>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

namespace {


// Converts a hex character to its integer value
unsigned char from_hex(unsigned char c)
{
    if(::isdigit(c))
    {
        return static_cast<unsigned char>(c - '0');
    }
    auto const lower_c = ::tolower(c);
    if((lower_c >= 'a') && (lower_c <= 'f'))
    {
        return static_cast<unsigned char>(lower_c - 'a' + 10);
    }
    throw std::runtime_error("Invalid URL encoded character");
}

// Converts an integer value to its hex character
char to_hex(unsigned char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

}

// Implementation of an URL decoder, which is heavily inspired by:
// http://www.geekhideout.com/urlcode.shtml (Public Domain)
std::string decode_url(std::string const &raw_text)
{
    std::string decoded_url;
    for(size_t i = 0; i < raw_text.size(); ++i)
    {
        switch(raw_text.at(i))
        {
        // Allow + to be used for space.
        case '+':
            decoded_url += ' ';
            break;
        case '%':
            if((i + 2) < raw_text.size())
            {
                decoded_url += static_cast<char>(from_hex(raw_text[i+1]) << 4) | from_hex(raw_text[i+2]);
                i += 2; // skip those two additional positions
                break;
            }
            WC_FALLTHROUGH;
        default:
            decoded_url += raw_text[i];
        }
    }
    return decoded_url;
}

std::string encode_url(std::string const &encoded_text)
{
    std::string encoded;
    for(size_t i = 0; i < encoded_text.size(); ++i)
    {
        auto c = static_cast<unsigned char>(encoded_text.at(i));
        if(((c >= 0) && (::isalnum(c) != 0)) || (c == '-') || (c == '_') || (c == '.') || (c == '~'))
        {
            encoded += static_cast<char>(c);
        }
        else
        {
            encoded += std::string("%") + to_hex(c >> 4) + to_hex(c & 15);
        }
    }
    return encoded;
}


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
