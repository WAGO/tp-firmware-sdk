//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for syslog.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/structuring.h>

#include <syslog.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace logging {
namespace sal {

class syslog_impl : public syslog
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(syslog_impl)

public:
    syslog_impl() noexcept = default;
    ~syslog_impl() noexcept override = default;

public:
    void openlog(char const *ident,
                 int  const  option,
                 int  const  facility) override;
    void closelog() override;
    void log(int  const  priority,
             char const *message) override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static syslog_impl default_syslog;

syslog       *syslog::instance     = &default_syslog;
int    const  syslog::log_debug    = LOG_DEBUG;
int    const  syslog::log_info     = LOG_INFO;
int    const  syslog::log_notice   = LOG_NOTICE;
int    const  syslog::log_warning  = LOG_WARNING;
int    const  syslog::log_error    = LOG_ERR;
int    const  syslog::log_critical = LOG_CRIT;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
void syslog_impl::openlog(char const *ident,
                          int  const  option,
                          int  const  facility)
{
    return ::openlog(ident, option, facility);
}

void syslog_impl::closelog()
{
    return ::closelog();
}

void syslog_impl::log(int  const  priority,
                      char const *message)
{
    return ::syslog(priority, "%s", message);
}


} // Namespace sal
} // Namespace logging
} // Namespace wago


//---- End of source file ------------------------------------------------------
