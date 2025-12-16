//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project auth-service.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
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
#include <wc/structuring.h>

#include <fstream>
#include <cstring>

#include <sys/poll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace sal {

class filesystem_impl : public filesystem
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(filesystem_impl)
public:
    filesystem_impl() noexcept = default;
    ~filesystem_impl() noexcept override = default;

    int stat(char const  *file,
             struct stat *buf) const noexcept override;
    bool is_regular_file(std::string const &file_path) const noexcept override;
    std::vector<std::string> list_directory(std::string const &file_path) const override;
    int poll(pollfd *fds,
             nfds_t  nfds,
             int     timeout) const noexcept override;
    int chown(char  const *file,
              uid_t        owner,
              gid_t        group) const noexcept override;
    int chown(char const *file,
              char const *owner,
              char const *group) const noexcept override;
    int fcntl(int fd,
              int cmd,
              int first_arg) const noexcept override;
    int close(int fd) const noexcept override;
    std::unique_ptr<std::iostream> open_stream(std::string             const &file_path,
                                               std::ios_base::openmode        mode) override;
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

int filesystem_impl::stat(char const  *file,
                          struct stat *buf) const noexcept
{
    errno = 0;
    return ::stat(file, buf);
}

bool filesystem_impl::is_regular_file(std::string const &file_path) const noexcept
{
    struct stat buf;
    const int error = filesystem_impl::stat(file_path.c_str(), &buf);
    if (error != 0)
    {
        return false;
    }
    return S_ISREG(buf.st_mode);
}

std::vector<std::string> filesystem_impl::list_directory(std::string const &directory_path) const
{
    errno = 0;
    DIR *directory = opendir(directory_path.c_str());
    if (directory == nullptr)
    {
        throw std::runtime_error("Couldn't open directory: " + directory_path + ": " + errno_to_string(errno));
    }
    std::vector<std::string> result;
    dirent dir_content;
    dirent *dir_content_result = nullptr;
    while (   (readdir_r(directory, &dir_content, &dir_content_result) == 0)
           && (dir_content_result != nullptr))
    {
        if (strcmp(dir_content.d_name, ".")  == 0) { continue; }
        if (strcmp(dir_content.d_name, "..") == 0) { continue; }
        result.emplace_back(dir_content.d_name);
    }
    closedir(directory);
    return result;
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

std::unique_ptr<std::iostream> filesystem_impl::open_stream(std::string             const &file_path,
                                                            std::ios_base::openmode        mode)
{
    auto file_stream = std::make_unique<std::fstream>(file_path,  mode);
    if (!file_stream->is_open()) {
        throw std::runtime_error("failed to open \"" + std::string(file_path) + "\"");
    }
    return file_stream;
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
} // Namespace authserv
} // Namespace wago


//---- End of source file ------------------------------------------------------
