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
///  \brief    Factory class to create file abstraction classes.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSFILE_FILE_FACTORY_HPP_
#define SRC_LIBWDXLINUXOSFILE_FILE_FACTORY_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_i.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>
#include <memory>
#include <sys/types.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace file {
namespace sal {

class file_factory final
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(file_factory)

public:
    file_factory()  = delete;
    ~file_factory() = delete;

    //------------------------------------------------------------------------------
    /// Constructs a new file abstraction for read access.
    ///
    /// \param file_path
    ///   File path for file to read in file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    /// \param keep_file_open
    ///   Keep file permanently open for read operations.
    //------------------------------------------------------------------------------
    static std::unique_ptr<file_i> create_for_read_access(std::string const &file_path,
                                                          bool               no_empty_file_on_disk,
                                                          bool               keep_file_open);

    //------------------------------------------------------------------------------
    /// Constructs a new file abstraction for write access.
    ///
    /// \param file_path
    ///   File path for the final file in file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    /// \param file_capacity
    ///   File size for the final file in file system.
    /// \param file_mode
    ///   File mode for the final file in file system.
    ///   Mode "0000" means use of directory default.
    /// \param file_owner
    ///   File owner for the final file in file system.
    ///   Will only be applied if not empty.
    //------------------------------------------------------------------------------
    static std::unique_ptr<file_i> create_for_write_access(std::string const &file_path,
                                                           bool               no_empty_file_on_disk,
                                                           uint64_t           file_capacity,
                                                           mode_t      const  file_mode,
                                                           std::string const &file_owner);

    //------------------------------------------------------------------------------
    /// Constructs a new file abstraction for write access.
    ///
    /// \param file_path
    ///   File path for the final file in file system.
    /// \param no_empty_file_on_disk
    ///   An empty file is not stored on disk.
    ///   Read operations for a non-existing file will successfully deliver empty content.
    ///   A write operation for an empty file will remove the target file.
    /// \param file_capacity
    ///   File size for the final file in file system.
    /// \param file_mode
    ///   File mode for the final file in file system.
    ///   Mode "0000" means use of directory default.
    /// \param file_owner_id
    ///   User ID of owner for the final file in file system.
    /// \param file_group_id
    ///   Group ID of group for the final file in file system.
    //------------------------------------------------------------------------------
    static std::unique_ptr<file_i> create_for_write_access(std::string const &file_path,
                                                           bool               no_empty_file_on_disk,
                                                           uint64_t           file_capacity,
                                                           mode_t      const  file_mode,
                                                           uid_t       const  file_owner_id,
                                                           gid_t       const  file_group_id);
};


} // Namespace sal
} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSFILE_FILE_FACTORY_HPP_
//---- End of source file ------------------------------------------------------

