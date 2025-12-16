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
///  \brief    Linux file abstraction class.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSFILE_LINUX_FILE_HPP_
#define SRC_LIBWDXLINUXOSFILE_LINUX_FILE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_i.hpp"

#include <wc/std_includes.h>

#include <string>
#include <sys/types.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {

using file_descriptor     = int;
using file_descriptor_ptr = std::shared_ptr<file_descriptor>;


class linux_file final : public file_i
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(linux_file)

public:
    //------------------------------------------------------------------------------
    /// Constructs a new Linux file abstraction for read access.
    ///
    /// \param file_path
    ///   File path for file to read in Linux file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    /// \param keep_file_open
    ///   Keep file permanently open for read operations.
    //------------------------------------------------------------------------------
    linux_file(std::string const &file_path,
               bool               no_empty_file_on_disk,
               bool               keep_file_open);

    //------------------------------------------------------------------------------
    /// Constructs a new Linux file abstraction for write access.
    ///
    /// \param file_path
    ///   File path for the final file in Linux file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    /// \param file_capacity
    ///   File size for the final file in Linux file system.
    /// \param file_mode
    ///   File mode for the final file in Linux file system.
    ///   Mode "0000" means use of directory default.
    /// \param file_owner
    ///   File owner for the final file in Linux file system.
    ///   Will only be applied if not empty.
    //------------------------------------------------------------------------------
    linux_file(std::string const &file_path,
               bool               no_empty_file_on_disk,
               uint64_t           file_capacity,
               mode_t      const  file_mode,
               std::string const &file_owner);

    //------------------------------------------------------------------------------
    /// Constructs a new Linux file abstraction for write access.
    ///
    /// \param file_path
    ///   File path for the final file in Linux file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    /// \param file_capacity
    ///   File size for the final file in Linux file system.
    /// \param file_mode
    ///   File mode for the final file in Linux file system.
    ///   Mode "0000" means use of directory default.
    /// \param file_owner_id
    ///   User ID of owner for the final file in Linux file system.
    /// \param file_group_id
    ///   Group ID of group for the final file in Linux file system.
    //------------------------------------------------------------------------------
    linux_file(std::string const &file_path,
               bool               no_empty_file_on_disk,
               uint64_t           file_capacity,
               mode_t      const  file_mode,
               uid_t       const  file_owner_id,
               gid_t       const  file_group_id);

    //------------------------------------------------------------------------------
    /// Destructs class, representing a Linux file.
    //------------------------------------------------------------------------------
    ~linux_file() noexcept override;

    //------------------------------------------------------------------------------
    /// Move constructor to support move operations.
    ///
    /// \param src
    ///   Source file provider to move.
    //------------------------------------------------------------------------------
    linux_file(linux_file &&src) noexcept;

    // Not supported because of const members
    linux_file & operator=(linux_file &&src) = delete;

    // Interface file_i
    uint64_t get_file_size() override;
    std::vector<uint8_t> read(uint64_t offset,
                              size_t   length) override;
    void write(uint64_t                    offset,
               std::vector<uint8_t> const &data) override;
    uint64_t get_write_file_size() override;
    std::vector<uint8_t> read_write_file(uint64_t offset,
                                         size_t   length) override;
    file_descriptor_ptr get_write_file_fd() override;
    void store() override;

private:
    linux_file(std::string const &file_path,
               bool               no_empty_file_on_disk,
               uint64_t           file_capacity,
               mode_t      const  file_mode);

    std::string         const m_file_path;
    bool                const m_no_empty_file_on_disk;
    uint64_t            const m_file_capacity;
    std::string               m_directory_path;
    std::string               m_temp_file_path;
    file_descriptor_ptr       m_open_file;
    bool                      m_keep_file_open;
    bool                      m_readonly;
};


} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSFILE_LINUX_FILE_HPP_
//---- End of source file ------------------------------------------------------

