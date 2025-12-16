//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------

#ifndef SRC_COMMON_STRING_UTILS_HPP_
#define SRC_COMMON_STRING_UTILS_HPP_

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace wago {
namespace wdx {
namespace linuxos {

/**
 * Strict conversion of string to integer.
 * In contrast to std::stoi, only beginning +/- and digits are allowed.
*/
inline bool convert_to_int(std::string const &s, int &result)
{
    try
    {
        size_t processed_chars;
        result = stoi(s, &processed_chars);
        return (processed_chars == s.size()); // detect if stoi ignored trailing non-digits
    }
    catch(...)
    {
        return false;
    }
}

/**
 * Strict conversion of string to integer.
 * In contrast to std::stoi, only beginning +/- and digits are allowed.
*/
inline bool convert_to_uint32(std::string const &s, uint32_t &result)
{
    try
    {
        size_t processed_chars;
        if(s.size() > 0 && s[0] == '-')
        {
            return false;
        }
        unsigned long stoul_result = std::stoul(s, &processed_chars);
        if(stoul_result > std::numeric_limits<std::uint32_t>::max())
        {
            return false;
        }
        result = static_cast<uint32_t>(stoul_result);
        return (processed_chars == s.size()); // detect if stoul ignored trailing non-digits
    }
    catch(...)
    {
        return false;
    }
}

/**
 * Strict conversion of string to integer.
 * In contrast to std::stoi, only beginning +/- and digits are allowed.
*/
inline bool convert_to_uint64(std::string const &s, uint64_t &result)
{
    try
    {
        size_t processed_chars;
        if(s.size() > 0 && s[0] == '-')
        {
            return false;
        }
        unsigned long long stoull_result = std::stoull(s, &processed_chars);
        if(stoull_result > std::numeric_limits<std::uint64_t>::max())
        {
            return false;
        }
        result = static_cast<uint64_t>(stoull_result);
        return (processed_chars == s.size()); // detect if stoul ignored trailing non-digits
    }
    catch(...)
    {
        return false;
    }
}
/**
 * Strict conversion of string to integer.
 * In contrast to std::stoi, only beginning +/- and digits are allowed.
*/
inline bool convert_to_int64(std::string const &s, int64_t &result)
{
    try
    {
        size_t processed_chars;
        long long stoll_result = std::stoll(s, &processed_chars);
        if(    stoll_result > std::numeric_limits<std::int64_t>::max()
            || stoll_result < std::numeric_limits<std::int64_t>::min())
        {
            return false;
        }
        result = static_cast<int64_t>(stoll_result);
        return (processed_chars == s.size()); // detect if stoll ignored trailing non-digits
    }
    catch(...)
    {
        return false;
    }
}

inline void to_lower(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
}

inline std::string tolower_copy(std::string s)
{
    to_lower(s);
    return s;
}

// https://stackoverflow.com/a/217605

// trim from start (in place)
inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
    {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
inline std::string ltrim_copy(std::string s)
{
    ltrim(s);
    return s;
}

// trim from end (copying)
inline std::string rtrim_copy(std::string s)
{
    rtrim(s);
    return s;
}

// trim from both ends (copying)
inline std::string trim_copy(std::string s)
{
    trim(s);
    return s;
}

inline std::vector<std::string> split_string(std::string const &s, char const &delimiter)
{
    //https://stackoverflow.com/a/14266139

    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while(getline(ss, item, delimiter))
    {
        result.push_back(item);
    }
    return result;
}

inline bool starts_with(std::string const &s, std::string const &prefix)
{
    return s.rfind(prefix, 0) == 0;
}

} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_COMMON_STRING_UTILS_HPP_
