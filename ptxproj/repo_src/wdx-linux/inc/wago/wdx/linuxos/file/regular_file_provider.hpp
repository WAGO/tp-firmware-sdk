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
///  \brief    File provider for regular files in Linux file system.
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   Rhö:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_WDX_LINUXOS_FILE_REGULAR_FILE_PROVIDER_HPP_
#define INC_WAGO_WDX_LINUXOS_FILE_REGULAR_FILE_PROVIDER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"
#include "wago/wdx/linuxos/file/file_provider_extended_i.hpp"

#include <wc/structuring.h>
#include <istream>
#include <sys/types.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

/// Implementing inner class.
class file_provider_impl;


//------------------------------------------------------------------------------
/// \brief File provider class for regular files.
///
/// \remark This class is thread-safe.
///
/// This file provider represents regular files in Linux files systems.
//------------------------------------------------------------------------------
class WDXLINUXOSFILE_API regular_file_provider: public file_provider_extended_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(regular_file_provider)

public:
    //------------------------------------------------------------------------------
    /// Constructs a file provider for a readonly file.
    ///
    /// \param readonly_file_path
    ///   File path to readonly file.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    /// \param keep_file_open
    ///   Keep file open for read access permanently.
    //------------------------------------------------------------------------------
    regular_file_provider(std::string const &readonly_file_path,
                          bool               no_empty_file_on_disk,
                          bool               keep_file_open = false);

    //------------------------------------------------------------------------------
    /// Constructs a file provider to replace an existing file,
    /// using access rights of currently existing file.
    ///
    /// \param replaced_file_path
    ///   File path to file to replace.
    /// \param file_size_limit
    ///   Size limit for a new file to replace the current one.
    //------------------------------------------------------------------------------
    regular_file_provider(std::string const &replaced_file_path,
                          uint64_t           file_size_limit);

    //------------------------------------------------------------------------------
    /// Constructs a file provider to replace an existing file or create a new one
    /// with given access rights.
    ///
    /// \param file_path
    ///   File path for the final file in Linux file system.
    /// \param file_size_limit
    ///   Size limit for a new file.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    /// \param file_mode
    ///   File mode for the final file in Linux file system.
    ///   Mode "0000" means use of directory default.
    /// \param file_owner
    ///   File owner for the final file in Linux file system.
    ///   Will only be applied if not empty.
    //------------------------------------------------------------------------------
    regular_file_provider(std::string const &new_file_path,
                          uint64_t           file_size_limit,
                          bool               no_empty_file_on_disk,
                          mode_t      const  file_mode,
                          std::string const &file_owner = "");

    //------------------------------------------------------------------------------
    /// Destructs file provider and removes any temporarily stored data.
    //------------------------------------------------------------------------------
    ~regular_file_provider() noexcept override;

    //------------------------------------------------------------------------------
    /// Move constructor to support move operations.
    ///
    /// \param src
    ///   Source file provider to move.
    //------------------------------------------------------------------------------
    regular_file_provider(regular_file_provider &&src) noexcept;

    //------------------------------------------------------------------------------
    /// Move assignment operator to support move operations.
    ///
    /// \param src
    ///   Source file provider to move.
    //------------------------------------------------------------------------------
    regular_file_provider & operator=(regular_file_provider &&src) noexcept;

    std::string const &get_file_path();

    // Interface file_provider_i
    future<wdx::response> create(uint64_t capacity) override;
    future<wdx::response> write(uint64_t             offset,
                                std::vector<uint8_t> data) override;
    future<wdx::file_info_response> get_file_info() override;
    future<wdx::file_read_response> read(uint64_t offset,
                                         size_t   length) override;

    // Interface file_provider_extended_i
    future<bool> is_complete() const override;
    future<void> validate(file_validator validator) const override;
    future<void> finish() override;

private:
    /// Private class implementation.
    std::unique_ptr<file_provider_impl> impl;
};


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago

#endif // INC_WAGO_WDX_LINUXOS_FILE_REGULAR_FILE_PROVIDER_HPP_
//---- End of source file ------------------------------------------------------
