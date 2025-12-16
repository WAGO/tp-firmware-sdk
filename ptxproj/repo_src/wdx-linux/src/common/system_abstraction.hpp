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
///  \brief    System abstraction for libwdxlinuxosserv to be independent
///            from real system functions (e. g. helpful for tests)
///
///  \author   PEn:  WAGO GmbH & Co. KG
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_COMMON_SYSTEM_ABSTRACTION_HPP_
#define SRC_COMMON_SYSTEM_ABSTRACTION_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <memory>
#include <istream>
#include <ios>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include <sys/poll.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
struct stat;

namespace wago {
namespace wdx {
namespace linuxos {
namespace sal {

// sockets abstraction (replacement for sys/poll.h)
class filesystem
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(filesystem)

protected:
    static filesystem *instance;

protected:
    filesystem() noexcept = default;

public:
    static filesystem &get_instance() noexcept { return *instance; }

    virtual ~filesystem() noexcept = default;

    virtual bool is_directory_existing(std::string const &file_path) const noexcept = 0;
    virtual bool is_socket_existing(std::string const &file_path) const noexcept = 0;
    virtual bool is_file_existing(std::string const &file_path) const noexcept = 0;
    virtual std::vector<std::string> glob(std::string const &glob_pattern) const = 0;

    virtual std::unique_ptr<std::iostream> open_stream(std::string             const &file_path,
                                                       std::ios_base::openmode        mode) const = 0;

    virtual int stat(char        const *file,
                     struct stat       *buf) const noexcept = 0;
    virtual int poll(pollfd *fds,
                     nfds_t  nfds,
                     int     timeout) const noexcept = 0;
    virtual int chown(char  const *file,
                      uid_t        owner,
                      gid_t        group) const noexcept = 0;
    virtual int chown(char  const *file,
                      char  const *owner,
                      char  const *group) const noexcept = 0;
    virtual int chmod(char   const *path,
                      mode_t        mode) const noexcept = 0;
    virtual int fcntl(int fd,
                      int cmd,
                      int first_arg) const noexcept = 0;
    virtual int close(int fd) const noexcept = 0;
    virtual int rename(char const *old_name, char const *new_name) const noexcept = 0;
    virtual int unlink(char const *filename) const noexcept = 0;
};

// systemd abstraction
class systemd
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(systemd)

protected:
    static systemd *instance;

protected:
    systemd() noexcept = default;

public:
    static systemd &get_instance() noexcept { return *instance; }

    virtual ~systemd() noexcept = default;
    
    // File descriptor functionality
    virtual int get_listen_fds_start() const = 0;
    virtual int get_listen_fds_with_names(char ***fd_names) const = 0;
    virtual int check_for_socket_unix(int fd) const = 0;
    virtual int find_systemd_socket(std::string const &socket_name) const = 0;

    // Notify functionality
    virtual void notify_ready() const = 0;
    virtual void notify_stopping() const = 0;
};


} // Namespace sal
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_COMMON_SYSTEM_ABSTRACTION_HPP_
//---- End of source file ------------------------------------------------------
