//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for PAM.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace sal {

class pam_impl : public pam
{
public:
    pam_impl() noexcept = default;
    ~pam_impl() noexcept override = default;

    int start(char         const  *service_name,
              char         const  *user,
              pam_conv     const  *pam_conversation,
              pam_handle_t       **pamh) const noexcept;
    int end(pam_handle_t *pamh,
            int           pam_status) const noexcept;
    int authenticate(pam_handle_t *pamh,
                     int           pam_status) const noexcept;
    int acct_mgmt(pam_handle_t *pamh,
                  int           pam_status) const noexcept;
    const char * strerror(pam_handle_t *pamh,
                          int           errnum) const noexcept;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static pam_impl default_pam;

pam *pam::instance = &default_pam;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
int pam_impl::start(char         const  *service_name,
                    char         const  *user,
                    pam_conv     const  *pam_conversation,
                    pam_handle_t       **pamh) const noexcept
{
    return ::pam_start(service_name, user, pam_conversation, pamh);
}

int pam_impl::end(pam_handle_t *pamh,
                  int           pam_status) const noexcept
{
    return ::pam_end(pamh, pam_status);
}

int pam_impl::authenticate(pam_handle_t *pamh,
                           int           pam_status) const noexcept
{
    return ::pam_authenticate(pamh, pam_status);
}

int pam_impl::acct_mgmt(pam_handle_t *pamh,
                        int           pam_status) const noexcept
{
    return ::pam_acct_mgmt(pamh, pam_status);
}

char const * pam_impl::strerror(pam_handle_t *pamh,
                                int           errnum) const noexcept
{
    return ::pam_strerror(pamh, errnum);
}

} // Namespace sal
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
