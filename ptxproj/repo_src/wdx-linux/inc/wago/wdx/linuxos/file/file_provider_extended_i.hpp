//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Extended file provider interface for file providers
///            with completion, validation and write finish method.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   Rhö:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_FILE_FILE_PROVIDER_EXTENDED_I_HPP_
#define INC_WAGO_WDX_LINUXOS_FILE_FILE_PROVIDER_EXTENDED_I_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wago/wdx/file_transfer/file_provider_i.hpp>
#include <wago/future.hpp>
#include <wc/structuring.h>

#include <functional>
#include <istream>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

/// Type definition for a file validator.
using file_validator = std::function<bool(std::basic_istream<uint8_t>&)>;


class WDXLINUXOSFILE_API file_provider_extended_i : public wdx::file_provider_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(file_provider_extended_i)

public:
    // NOLINTNEXTLINE(modernize-use-equals-default): We want to use original constructor signature
    using wdx::file_provider_i::file_provider_i;

    //------------------------------------------------------------------------------
    /// Destructs file provider and removes any temporarily stored data.
    //------------------------------------------------------------------------------
    ~file_provider_extended_i() noexcept override = default;

    //------------------------------------------------------------------------------
    /// Move constructor to support move operations.
    ///
    /// \param src
    ///   Source file provider to move.
    //------------------------------------------------------------------------------
    file_provider_extended_i(file_provider_extended_i &&src) noexcept = default;

    //------------------------------------------------------------------------------
    /// Move assignment operator to support move operations.
    ///
    /// \param src
    ///   Source file provider to move.
    //------------------------------------------------------------------------------
    file_provider_extended_i & operator=(file_provider_extended_i &&src) noexcept = default;

    //------------------------------------------------------------------------------
    /// Check if file is considered complete. A file is complete when either the 
    /// whole capacity is completely written by previous write calls or the file is
    /// readonly.
    ///
    /// \remark
    ///   Sets std::logic_error on returned future when no create call was issued.
    ///
    /// \return
    ///   True, if file is considered complete.
    //------------------------------------------------------------------------------
    virtual future<bool> is_complete() const = 0;

    //------------------------------------------------------------------------------
    /// Validates the written data with help of a validator function.
    ///
    /// \remark The validation checks if the file content is completely written.
    ///         Call \see is_complete before if necessary.
    ///
    /// \remark Exceptions thrown by validator are not handled by validate,
    ///         but will bubble out (e.g. for own fail handling).
    ///
    /// \remark
    ///   Sets std::logic_error on returned future when no create call was issued.
    ///
    /// \remark
    ///   Sets std::runtime_error on returned future when validator returns false.
    ///
    /// \param validator
    ///   Function to use for file content validation.
    //------------------------------------------------------------------------------
    virtual future<void> validate(file_validator validator) const = 0;

    //------------------------------------------------------------------------------
    /// Moves written data to final destination and switches to readonly mode.
    ///
    /// \throws
    ///   Throws std::logic_error when no create call was issued or file is readonly.
    ///   Throws std::runtime_error on any other failure.
    ///
    /// \remark Call to finish will fail if no successful create call was done.
    //------------------------------------------------------------------------------
    virtual future<void> finish() = 0;
};

} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // INC_WAGO_WDX_LINUXOS_FILE_FILE_PROVIDER_EXTENDED_I_HPP_

//---- End of source file ------------------------------------------------------
