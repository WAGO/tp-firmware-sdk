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
///
///  \brief    System abstraction for libwagologging to be independent
///            from real system functions (e. g. helpful for tests)
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWAGOLOGGING_SYSTEM_ABSTRACTION_HPP_
#define SRC_LIBWAGOLOGGING_SYSTEM_ABSTRACTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>
#include <wc/std_includes.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace logging {
namespace sal {

// syslog abstraction (replacement for syslog.h)
class syslog
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(syslog)

public:
    static int const  log_debug;
    static int const  log_info;
    static int const  log_notice;
    static int const  log_warning;
    static int const  log_error;
    static int const  log_critical;

protected:
    static syslog    *instance;

protected:
    syslog() noexcept = default;

public:
    static syslog &get_instance() noexcept { return *instance; }

    virtual ~syslog() noexcept = default;
    virtual void openlog(char const *ident,
                         int  const  option,
                         int  const  facility) = 0;
    virtual void closelog() = 0;
    virtual void log(int  const  priority,
                     char const *message) = 0;
};

#ifdef SYSTEMD_INTEGRATION
// syslog abstraction (replacement for syslog.h)
class journal
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(journal)

public:
    static int const  log_debug;
    static int const  log_info;
    static int const  log_notice;
    static int const  log_warning;
    static int const  log_error;
    static int const  log_critical;
    static int const  log_alert;
    // Emergency level should not be used by applications;

protected:
    static journal   *instance;

protected:
    journal() noexcept = default;

public:
    static journal &get_instance() noexcept { return *instance; }

    virtual ~journal() noexcept = default;
    virtual void log(int  const  priority,
                     char const *message) = 0;
};

#endif // SYSTEMD_INTEGRATION


} // Namespace sal
} // Namespace logging
} // Namespace wago


#endif // SRC_LIBWAGOLOGGING_SYSTEM_ABSTRACTION_HPP_
//---- End of source file ------------------------------------------------------

