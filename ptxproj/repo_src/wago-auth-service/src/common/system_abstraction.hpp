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
///  \brief    System abstraction for libauthserv to be independent
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
#include <string>
#include <vector>

#include <fcgiapp.h>
#include <security/pam_appl.h>
#include <sys/types.h>
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
namespace authserv {
namespace sal {

// sockets & filesystem abstraction
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


    virtual int stat(char        const *file,
                     struct stat       *buf) const noexcept = 0;
    virtual bool is_regular_file(std::string const &file_path) const noexcept = 0;
    virtual std::vector<std::string> list_directory(std::string const &file_path) const = 0;
    virtual int poll(pollfd *fds,
                     nfds_t  nfds,
                     int     timeout) const noexcept = 0;
    virtual int chown(char  const *file,
                      uid_t        owner,
                      gid_t        group) const noexcept = 0;
    virtual int chown(char  const *file,
                      char  const *owner,
                      char  const *group) const noexcept = 0;
    virtual int fcntl(int fd,
                      int cmd,
                      int first_arg) const noexcept = 0;
    virtual int close(int fd) const noexcept = 0;
    virtual std::unique_ptr<std::iostream> open_stream(std::string             const &file_path,
                                                       std::ios_base::openmode        mode) = 0;
    virtual int rename(char const *old_name, char const *new_name) const noexcept = 0;
    virtual int unlink(char const *filename) const noexcept = 0;
};

// clock abstraction (replacement for time.h)
class system_clock
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(system_clock)
protected:
    static system_clock *instance;

protected:
system_clock() noexcept = default;

public:
    static system_clock &get_instance() noexcept { return *instance; }

    virtual ~system_clock() noexcept = default;
    virtual void gettime(clockid_t  clk_id,
                         timespec  *res) const noexcept = 0;
    virtual void sleep_for(time_t milliseconds) const noexcept = 0;
};

// user management abstraction (replacement for grp.h)
class user_management
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(user_management)
protected:
    static user_management *instance;

protected:
    user_management() noexcept = default;

public:
    static user_management &get_instance() noexcept { return *instance; }

    virtual ~user_management() noexcept = default;
    virtual std::string getgroupname(gid_t gid) const noexcept = 0;
    virtual std::vector<std::string> get_groups(std::string const &user) const = 0;
};

// hostname abstraction
class hostname
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(hostname)
protected:
    static hostname *instance;

protected:
    hostname() noexcept = default;

public:
    static hostname &get_instance() noexcept { return *instance; }

    virtual ~hostname() noexcept = default;
    virtual int gethostname(char *out, size_t n) const noexcept = 0;
};

// libfcgi abstraction (replacement for fcgiapp.h)
class fcgi
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(fcgi)
protected:
    static fcgi *instance;

protected:
    fcgi() noexcept = default;

public:
    static fcgi &get_instance() noexcept { return *instance; }

    virtual        ~fcgi() noexcept = default;
    virtual int    Init() noexcept = 0;
    virtual int    OpenSocket(char const *path,
                              int         backlog) noexcept = 0;
    virtual void   ShutdownPending() noexcept = 0;
    virtual int    InitRequest(FCGX_Request *request,
                               int           sock,
                               int           flags) noexcept = 0;
    virtual int    Accept_r(FCGX_Request *request) noexcept = 0;
    virtual char * GetParam(char const      *name,
                            FCGX_ParamArray  envp) noexcept = 0;
    virtual char * GetLine(char        *str,
                           int          n,
                           FCGX_Stream *stream) noexcept = 0;
    virtual int    GetStr(char        *str,
                          int          n,
                          FCGX_Stream *stream) noexcept = 0;
    virtual int    PutS(char const  *str,
                        FCGX_Stream *stream) noexcept = 0;
    virtual int    PutStr(char const  *str,
                          int          n,
                          FCGX_Stream *stream) noexcept = 0;
    virtual int    FFlush(FCGX_Stream *stream) noexcept = 0;
    virtual void   Finish_r(FCGX_Request *request) noexcept = 0;
    virtual void   Free(FCGX_Request *request,
                        int           close) noexcept = 0;
    virtual int    HasSeenEOF(FCGX_Stream *stream) noexcept = 0;
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

// libpam abstraction
class pam
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(pam)
protected:
    static pam *instance;

protected:
pam() noexcept = default;

public:
    static pam &get_instance() noexcept { return *instance; }

    virtual ~pam() noexcept = default;
    
    virtual int start(char         const  *service_name,
                      char         const  *user,
                      pam_conv     const  *pam_conversation,
                      pam_handle_t       **pamh) const noexcept = 0;
    virtual int end(pam_handle_t *pamh,
                    int           pam_status) const noexcept = 0;
    virtual int authenticate(pam_handle_t *pamh,
                             int           pam_status) const noexcept = 0;
    virtual int acct_mgmt(pam_handle_t *pamh,
                          int           pam_status) const noexcept = 0;
    virtual const char * strerror(pam_handle_t *pamh,
                                  int           errnum) const noexcept = 0;
};

} // Namespace sal
} // Namespace authserv
} // Namespace wago


#endif // SRC_COMMON_SYSTEM_ABSTRACTION_HPP_
//---- End of source file ------------------------------------------------------
