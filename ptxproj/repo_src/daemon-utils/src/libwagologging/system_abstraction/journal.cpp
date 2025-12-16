//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for systemd journal.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/structuring.h>

#ifdef SYSTEMD_INTEGRATION
#include <systemd/sd-journal.h>

#include <stdexcept>
#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace logging {
namespace sal {

class journal_impl : public journal
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(journal_impl)

public:
    journal_impl() noexcept = default;
    ~journal_impl() noexcept override = default;

public:
    void log(int  const  priority,
             char const *message) override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static journal_impl default_journal;

journal       *journal::instance     = &default_journal;
int     const  journal::log_debug    = LOG_DEBUG;
int     const  journal::log_info     = LOG_INFO;
int     const  journal::log_notice   = LOG_NOTICE;
int     const  journal::log_warning  = LOG_WARNING;
int     const  journal::log_error    = LOG_ERR;
int     const  journal::log_critical = LOG_CRIT;
int     const  journal::log_alert    = LOG_ALERT;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
void journal_impl::log(int  const  priority,
                       char const *message)
{
    // Journal's send function can take many meta data fields
    // See: https://0pointer.de/blog/projects/journal-submit.html
    int const journal_result = ::sd_journal_send("MESSAGE=%s", message,
                                                 "PRIORITY=%i", priority,
                                                 nullptr);

    if(journal_result < 0)
    {
        throw std::runtime_error("Failed to send journal message (" + std::to_string(journal_result) + ")");
    }
}


} // Namespace sal
} // Namespace logging
} // Namespace wago

#endif // SYSTEMD_INTEGRATION


//---- End of source file ------------------------------------------------------
