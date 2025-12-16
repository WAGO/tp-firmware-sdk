//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction for libwdxlinuxosfile to be independent
///            from real system functions (e. g. helpful for tests)
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXLINUXOSFILE_SYSTEM_ABSTRACTION_FILE_HPP_
#define SRC_LIBWDXLINUXOSFILE_SYSTEM_ABSTRACTION_FILE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <string>
#include <vector>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
struct stat;

namespace wago {
namespace wdx {
namespace linuxos {
namespace file {
namespace sal {

using file_descriptor   = int;

class filemanagement
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(filemanagement)

protected:
    static filemanagement *instance;

protected:
    filemanagement() = default;

public:
    static filemanagement &get_instance() noexcept { return *instance; }

    virtual ~filemanagement() noexcept = default;

    virtual file_descriptor open_readonly(std::string const &file_path) noexcept = 0;
    virtual file_descriptor create_and_open_temp_file(std::string const &file_path_prefix,
                                                      std::string const &file_suffix,
                                                      std::string       &result_file_path) = 0;
    virtual void delete_old_temp_files(std::string const &file_path_prefix,
                                       std::string const &file_suffix) = 0;
    virtual int fchmod(file_descriptor fd,
                       mode_t          mode) noexcept = 0;
    virtual int fchown(file_descriptor fd,
                       uid_t           owner,
                       gid_t           group) noexcept = 0;
    virtual off64_t seek_to_offset(file_descriptor fd,
                                   size_t          offset) noexcept = 0;
    virtual ssize_t read(file_descriptor       fd,
                         std::vector<uint8_t> &data) noexcept = 0;
    virtual ssize_t write(file_descriptor             fd,
                          std::vector<uint8_t> const &data) noexcept = 0;
    virtual int close(file_descriptor fd) noexcept = 0;
    virtual int fsync(file_descriptor fd) noexcept = 0;
    virtual int get_file_size(std::string const &file_path,
                              uint64_t          &file_size) noexcept = 0;
    virtual int get_file_permissions(std::string const &file_path,
                                     mode_t            &mode,
                                     uid_t             &uid,
                                     gid_t             &gid) noexcept = 0;
    virtual int get_file_system_space(std::string const &directory_path,
                                      uint64_t          &free_size) noexcept = 0;
    virtual int rename(std::string const &old_path,
                       std::string const &new_path) noexcept = 0;
    virtual int unlink(std::string const &file_path) noexcept = 0;
    virtual void get_user_info(std::string const &name,
                               uid_t             &uid,
                               gid_t             &gid) = 0;
};


} // Namespace sal
} // Namespace wdx
} // Namespace linuxos
} // Namespace file {
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSFILE_SYSTEM_ABSTRACTION_FILE_HPP_
//---- End of source file ------------------------------------------------------
