//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Class template for wrapping system or global functions to make 
///            them replaceable (especially helpful for unit tests).
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWTRACE_SAL_HPP_
#define SRC_LIBWTRACE_SAL_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>

#include <type_traits>
#include <memory>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

namespace wago {
namespace wtrace {
namespace sal {

template <class Interface>
class sysfunc final
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(sysfunc)

private:
    Interface *impl_m;

public:
    constexpr sysfunc(Interface * impl)
    : impl_m(impl)
    { }

    ~sysfunc() = default;

    Interface * operator->() const noexcept
    {
        return impl_m;
    }

    Interface * exchange_impl(Interface *impl)
    {
        auto *old = impl_m;    
        impl_m = impl;
        return old;
    }

    static sysfunc & get_instance() noexcept;
};

} // Namespace sal
} // Namespace wtrace
} // Namespace wago


#endif // SRC_LIBWTRACE_SAL_HPP_
//---- End of source file ------------------------------------------------------
