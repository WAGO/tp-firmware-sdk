//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2021-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    WAGO parameter service settings store interface.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_WDA_SETTINGS_STORE_I_HPP_
#define INC_WAGO_WDX_WDA_SETTINGS_STORE_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/wdx/wda/api.h"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace wda {

class settings_store_i
{
    WC_INTERFACE_CLASS(settings_store_i)

public:
    static constexpr char const allow_unauthenticated_requests_for_scan_devices[] = "allow-unauth-scan-devices";
    static constexpr char const file_api_upload_id_timeout[]                      = "file-api-upload-id-timeout";
    static constexpr char const run_result_timeout[]                              = "run-result-timeout";
    static constexpr char const oauth2_origin[]                                   = "oauth2-origin";
    static constexpr char const oauth2_token_path[]                               = "oauth2-token-path";
    static constexpr char const oauth2_verify_access_path[]                       = "oauth2-verify-access-path";
    static constexpr char const oauth2_client_id[]                                = "oauth2-client-id";
    static constexpr char const oauth2_client_secret[]                            = "oauth2-client-secret";

public:
    /// Get a settings value by key.
    ///
    /// \throw
    ///   std::out_of_range if unknown key is requested.
    /// \throw
    ///   std::runtime_error if value can not be discovered (e.g. config not available).
    ///
    /// \param key
    ///   Key to write value for.
    ///
    /// \return
    ///   Current settings value for given key.
    virtual std::string get_setting(std::string const &key) const = 0;

    /// Writes a new a settings value for given key.
    ///
    /// \throw
    ///   std::out_of_range if unknown key is requested to write value for.
    /// \throw
    ///   std::invalid_argument if value is not valid for given key.
    ///
    /// \param key
    ///   Key to write value for.
    /// \param value
    ///   New value to write.
    virtual void write_setting(std::string const &key,
                               std::string const &value) = 0;
};


} // Namespace wda
} // Namespace wdx
} // Namespace wago


#endif // INC_WAGO_WDX_WDA_SETTINGS_STORE_I_HPP_
//---- End of source file ------------------------------------------------------
