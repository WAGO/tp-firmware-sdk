//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO parameter service authentication settings interface.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_AUTH_SETTINGS_I_HPP_
#define SRC_LIBWDXWDA_AUTH_SETTINGS_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

class auth_settings_i
{
    WC_INTERFACE_CLASS(auth_settings_i)

public:
    /// Get a settings value for unauthenticated URLs.
    ///
    /// \return
    ///    Current settings value for unauthenticated URLs.
    virtual std::string get_unauthenticated_urls() const = 0;
};


} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_AUTH_SETTINGS_I_HPP_
//---- End of source file ------------------------------------------------------
