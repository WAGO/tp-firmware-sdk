//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
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
#include <limits.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {

namespace {

// Check if char is alphanumeric
bool isalphanumeric(char c)
{
    // Type char is special and may be signed or unsigned, depending on compiler and system architecture
    // See also: https://stackoverflow.com/a/2054941
#if CHAR_MIN < 0
    return (c >= 0 && (::isalnum(c) != 0));
#else
    return (::isalnum(c) != 0);
#endif
}

// Converts a hex character to its integer value
char from_hex(char c)
{
  return static_cast<char>(::isdigit(c) ? c - '0' : ::tolower(c) - 'a' + 10);
}

// Converts an integer value to its hex character
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

}

// Implementation of an URL decoder, which is heavily inspired by:
// http://www.geekhideout.com/urlcode.shtml (Public Domain)
std::string decode_url(std::string const &raw_url)
{
    std::string decoded_url;
    for(size_t i = 0; i < raw_url.size(); ++i)
    {
        switch(raw_url.at(i))
        {
        // Allow + to be used for space.
        case '+':
            decoded_url += ' ';
            break;
        case '%':
            if((i + 2) < raw_url.size())
            {
                decoded_url += static_cast<char>(from_hex(raw_url[i+1]) << 4) | from_hex(raw_url[i+2]);
                i += 2; // skip those two positions
                break;
            }
            WC_FALLTHROUGH;
        default:
            decoded_url += raw_url[i];
        }
    }
    return decoded_url;
}

std::string encode_url(std::string const &text)
{
    std::string encoded;
    for(size_t i = 0; i < text.size(); ++i)
    {
        auto c = text.at(i);
        if (isalphanumeric(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            encoded += c;
        }
        else
        {
            encoded += std::string("%") + to_hex(c >> 4) + to_hex(c & 15);
        }
    }
    return encoded;
}


} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
