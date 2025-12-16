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
///  \brief    Kernel trace system abstraction to be independent
///            from real library functions (e. g. helpful for tests)
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWTRACE_SAL_KTRACE_HPP_
#define SRC_LIBWTRACE_SAL_KTRACE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "sal.hpp"
#include "find_debug_fs.hpp"
#include <fcntl.h>
#include <unistd.h>
//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

namespace wago {
namespace wtrace {
namespace sal {

class ktrace_i
{
    WC_INTERFACE_CLASS(ktrace_i)

public:
    virtual std::string find_debug_fs() const = 0;
    virtual int open(char const *, int) const = 0;
    virtual int close(int) const noexcept = 0;
    virtual int write(int, char const *, size_t) const = 0;
};

using ktrace = sysfunc<ktrace_i>;

} // Namespace sal
} // Namespace wtrace
} // Namespace wago


#endif // SRC_LIBWTRACE_SAL_KTRACE_HPP_
//---- End of source file ------------------------------------------------------
