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
///  \brief    Utility for debug logs on IPC object communication
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_DEBUG_LOG_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_DEBUG_LOG_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "managed_object.hpp"
#include "method_id.hpp"

#include <wc/log.h>
#include <map>
#include <vector>
#include <cinttypes>

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

/// Helper to get name of an object by interface
/// specialized implementations are placed inside debug_log.cpp files 
/// in interface specific source folders
template <typename Interface>
char const * debug_log_object_name();

/// Helper to get name of a method by interface and method id
/// specialized implementations are placed inside debug_log.cpp files 
/// in interface specific source folders
template <typename Interface>
char const * debug_log_method_name(method_id_type method_id);

template <typename ... ArgumentTypes>
std::string debug_log_args_list(ArgumentTypes const &... args);

template <typename T>
std::string to_string(T const &);

template<typename T>
std::string to_string(std::vector<T> const &);

template<typename T>
std::string to_string(std::vector<T> const &input_vector)
{
    std::string return_string = "[";
    const std::string add_str = ", ";
    uint8_t iterator = 0;
    for(auto const &element: input_vector)
    {
        return_string += to_string(element) + add_str;
        if(++iterator > 8)
        {
            return_string += ".....";
            break;
        }
    }
    if(return_string.length() > add_str.length())
    {
        return_string = return_string.substr(0, return_string.length() - add_str.length());
    }
    return return_string  + "]";
}

template<typename T>
std::string to_string(std::map<std::string, T> const &);

template<typename T>
std::string to_string(std::map<std::string, T> const &input_map)
{
    std::string return_string = "[";
    const std::string add_str = ", ";
    uint8_t iterator = 0;
    for(auto const &key_value: input_map)
    {
        return_string += to_string(key_value.first) + ":" + to_string(key_value.second) + add_str;
        if(++iterator > 8)
        {
            return_string += ".....";
            break;
        }
    }
    if(return_string.length() > add_str.length())
    {
        return_string = return_string.substr(0, return_string.length() - add_str.length());
    }
    return return_string + "]";
}

template<class T>
std::string debug_log_args_list(T const &args)
{
    return to_string(args);
}

template<>
std::string debug_log_args_list();

template<class T,
         class ...U>
std::string debug_log_args_list(T const &first_t, U const & ... args)
{
    return to_string(first_t) + ", " + debug_log_args_list(args ...);
}

template <typename Interface, typename ... ArgumentTypes>
void debug_log_interface_call(std::string              const &prefix,
                              std::string              const &connection_name,
                              managed_object_id               object_id,
                              uint32_t                        call_id,
                              uint32_t                        method_id,
                              ArgumentTypes            const &... args)
{
    if(wc_get_log_level() == log_level::debug)
    {
        std::string object_name = debug_log_object_name<Interface>();
        std::string method_name = debug_log_method_name<Interface>(method_id);
        std::string method_args = debug_log_args_list(args...); 

        if(method_args.size() > 1024)
        {
            method_args.resize(1024);
        }

        wc_log_format(log_level::debug, "%s: connection=%s object_id=%u call_id=%u method=%s::%s arguments=%s",
                      prefix.c_str(), connection_name.c_str(), object_id, call_id, object_name.c_str(), method_name.c_str(), method_args.c_str());
    }
}

template <typename Interface, typename ... ArgumentTypes>
void debug_log_call_count(std::string       const &prefix,
                          std::string       const &connection_name,
                          managed_object_id        object_id,
                          uint32_t                 call_id,
                          size_t                   count)
{
    if(wc_get_log_level() == log_level::debug)
    {
        std::string object_name = debug_log_object_name<Interface>();

        wc_log_format(log_level::debug, "%s: connection=%s object_id=%u call_id=%u object=%s count=%" PRIuPTR,
                      prefix.c_str(), connection_name.c_str(), object_id, call_id, object_name.c_str(), count);
    }
}


} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_DEBUG_LOG_HPP_
//---- End of source file ------------------------------------------------------
