//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    System abstraction implementation for file operations.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"
#include "errno_utils.hpp"

#include <wago/privileges/user.hpp>
#include <wago/privileges/group.hpp>

#include <fstream>
#include <cstring>

#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace sal {

class filesystem_impl : public filesystem
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(filesystem_impl)

public:
    filesystem_impl() noexcept = default;
    ~filesystem_impl() noexcept override = default;

    bool is_directory_existing(std::string const &file_path) const noexcept override;
    bool is_socket_existing(std::string const &file_path) const noexcept override;
    bool is_file_existing(std::string const &file_path) const noexcept override;

    std::unique_ptr<std::iostream> open_stream(std::string             const &file_path,
                                               std::ios_base::openmode        mode) const override;

    std::vector<std::string> glob(std::string const &glob_pattern) const override;

    int stat(char const  *file,
             struct stat *buf) const noexcept override;
    int poll(pollfd *fds,
             nfds_t  nfds,
             int     timeout) const noexcept override;
    int chown(char  const *file,
              uid_t        owner,
              gid_t        group) const noexcept override;
    int chown(char const *file,
              char const *owner,
              char const *group) const noexcept override;
    int chmod(char const *path,
              mode_t      mode) const noexcept override;
    int fcntl(int fd,
              int cmd,
              int first_arg) const noexcept override;
    int close(int fd) const noexcept override;
    int rename(char const *old_name, char const *new_name) const noexcept override;
    int unlink(char const *filename) const noexcept override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static filesystem_impl default_filesystem;

filesystem *filesystem::instance = &default_filesystem;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
bool filesystem_impl::is_directory_existing(std::string const &file_path) const noexcept
{
    struct stat stat_buffer; // Struct will be initialized within stat call NOLINT(cppcoreguidelines-pro-type-member-init)
    auto result = (filesystem_impl::stat(file_path.c_str(), &stat_buffer) == 0);

    return result && S_ISDIR(stat_buffer.st_mode);
}

bool filesystem_impl::is_socket_existing(std::string const &file_path) const noexcept
{
    struct stat stat_buffer; // Struct will be initialized within stat call NOLINT(cppcoreguidelines-pro-type-member-init)
    auto result = (filesystem_impl::stat(file_path.c_str(), &stat_buffer) == 0);

    return result && S_ISSOCK(stat_buffer.st_mode);
}

bool filesystem_impl::is_file_existing(std::string const &file_path) const noexcept
{
    struct stat stat_buffer;
    return (filesystem_impl::stat(file_path.c_str(), &stat_buffer) == 0);
}

std::unique_ptr<std::iostream> filesystem_impl::open_stream(std::string             const &file_path,
                                                            std::ios_base::openmode        mode) const
{
    auto file_stream = std::make_unique<std::fstream>(file_path, mode);
    if(!file_stream->is_open()) {
        return std::unique_ptr<std::iostream>();
    }
    return file_stream;
}

std::vector<std::string> filesystem_impl::glob(std::string const &glob_pattern) const
{
    std::vector<std::string> paths;
    glob_t glob_result;
    
    errno = 0;
    auto glob_status = ::glob(glob_pattern.c_str(), GLOB_ERR, nullptr, &glob_result);
    if (errno != 0) // parasoft-suppress CERT_C-ERR30-a "errno may be set by internal read call as documented in header glob.h"
    {
        ::globfree(&glob_result);
        throw std::runtime_error("glob \"" + glob_pattern + "\" failed. errno=" + errno_to_string(errno));
    }
    else if ((glob_status != 0) && (glob_status != GLOB_NOMATCH))
    {
        ::globfree(&glob_result);
        throw std::runtime_error("glob \"" + glob_pattern + "\" failed. status=" + std::to_string(glob_status));
    }
    
    for (size_t i = 0; i < glob_result.gl_pathc; ++i)
    {
        paths.push_back(glob_result.gl_pathv[i]);
    }
    ::globfree(&glob_result);
    return paths;
}

int filesystem_impl::stat(char const  *file,
                          struct stat *buf) const noexcept
{
    errno = 0;
    return ::stat(file, buf);
}

int filesystem_impl::poll(pollfd *fds,
                          nfds_t  nfds,
                          int     timeout) const noexcept
{
    errno = 0;
    return ::poll(fds, nfds, timeout);
}

int filesystem_impl::chown(char  const *file,
                           uid_t        owner,
                           gid_t        group) const noexcept
{
    errno = 0;
    return ::chown(file, owner, group);
}

int filesystem_impl::chown(char const *file,
                           char const *owner,
                           char const *group) const noexcept
{
    uid_t const uid_owner = wago::privileges::get_uid_for_user(owner);
    uid_t const gid_group = wago::privileges::get_gid_for_group(group);
    return chown(file, uid_owner, gid_group);
}

int filesystem_impl::chmod(char const *path,
                           mode_t      mode) const noexcept
{
    errno = 0;
    return ::chmod(path, mode);
}

int filesystem_impl::fcntl(int fd,
                           int cmd,
                           int first_arg) const noexcept
{
    errno = 0;
    return ::fcntl(fd, cmd, first_arg);
}

int filesystem_impl::close(int fd) const noexcept
{
    errno = 0;
    return ::close(fd);
}

int filesystem_impl::rename(char const *old_name, char const *new_name) const noexcept
{
    errno = 0;
    return ::rename(old_name, new_name);
}

int filesystem_impl::unlink(char const *filename) const noexcept
{
    errno = 0;
    return ::unlink(filename);
}


} // Namespace sal
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
