//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Service identity interface providing basic service identification.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_SERVICE_IDENTITY_I_HPP_
#define INC_WAGO_WDX_WDA_SERVICE_IDENTITY_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/api.h"

#include <wc/std_includes.h>
#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

/// Service identity interface to retrieve general service information.
class service_identity_i 
{
    WC_INTERFACE_CLASS(service_identity_i)

public:
    //------------------------------------------------------------------------------
    /// Method to get the service ID string
    /// (e. g. for configuration purposes).
    ///
    /// \return
    ///   Null terminated C string with service ID.
    //------------------------------------------------------------------------------
    virtual char const * get_id()             const = 0;

    //------------------------------------------------------------------------------
    /// Method to get the service name.
    ///
    /// \return
    ///   Null terminated C string with service name.
    //------------------------------------------------------------------------------
    virtual char const * get_name()           const = 0;

    //------------------------------------------------------------------------------
    /// Method to get the service version.
    ///
    /// \return
    ///   Null terminated C string with service version in a human readable format.
    //------------------------------------------------------------------------------
    virtual char const * get_version_string() const = 0;

    //------------------------------------------------------------------------------
    /// Method to get the service version major number.
    ///
    /// \return
    ///   Service version major number.
    //------------------------------------------------------------------------------
    virtual uint16_t     get_major_version()  const = 0;

    //------------------------------------------------------------------------------
    /// Method to get the service version minor number.
    ///
    /// \return
    ///   Service version minor number.
    //------------------------------------------------------------------------------
    virtual uint16_t     get_minor_version()  const = 0;

    //------------------------------------------------------------------------------
    /// Method to get the service version bugfix number.
    ///
    /// \return
    ///   Service version bugfix number.
    //------------------------------------------------------------------------------
    virtual uint16_t     get_bugfix_version() const = 0;

    //------------------------------------------------------------------------------
    /// Method to get the service version revision (version extension after hyphen "-").
    ///
    /// \return
    ///   Service version revision.
    //------------------------------------------------------------------------------
    virtual char const * get_revision_string() const = 0;
};


} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_SERVICE_IDENTITY_I_HPP_
//---- End of source file ------------------------------------------------------
