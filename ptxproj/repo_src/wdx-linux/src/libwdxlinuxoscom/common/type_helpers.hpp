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
///  \brief    Helpers for type traits
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSCOM_COMMON_TYPE_HELPERS_HPP_
#define SRC_LIBWDXLINUXOSCOM_COMMON_TYPE_HELPERS_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <tuple>
#include <type_traits>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace com {
namespace utils {

// Small helper to identify a type as a template class type
// Inspired by:  https://stackoverflow.com/questions/44012938/how-to-tell-if-template-type-is-an-instance-of-a-template-class
template <class, template <class> class>
struct is_instance : public std::false_type {};
template <class T, template <class> class U>
struct is_instance<U<T>, U> : public std::true_type {};

// A helper to call a function with arguments hold by a tuple
// Inspired by: https://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
template<typename Function, typename Tuple, size_t ... I>
auto call_helper(Function f, Tuple &t, std::index_sequence<I ...>)
{
    return f(std::get<I>(t) ...);
}
template<typename Function, typename Tuple>
auto call_helper(Function f, Tuple &t)
{
    static constexpr auto size = std::tuple_size<Tuple>::value;
    return call_helper(f, t, std::make_index_sequence<size>{});
}

} // Namespace utils
} // Namespace com
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSCOM_COMMON_TYPE_HELPERS_HPP_
//---- End of source file ------------------------------------------------------
