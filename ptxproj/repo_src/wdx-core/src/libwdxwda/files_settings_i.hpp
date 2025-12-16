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
///  \brief    WAGO parameter service File API settings interface.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXWDA_FILES_SETTINGS_I_HPP_
#define SRC_LIBWDXWDA_FILES_SETTINGS_I_HPP_

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

class files_settings_i
{
    WC_INTERFACE_CLASS(files_settings_i)

public:
    /// Get the settings value for File API upload timeout.
    ///
    /// \return
    ///    Current settings value for File API upload timeout.
    virtual uint16_t get_upload_id_timeout() const = 0;
};


} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXWDA_FILES_SETTINGS_I_HPP_
//---- End of source file ------------------------------------------------------
