//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2025 WAGO GmbH & Co. KG
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
#ifndef SRC_LIBWDXLINUXOSSERV_SYSTEM_ABSTRACTION_SERV_HPP_
#define SRC_LIBWDXLINUXOSSERV_SYSTEM_ABSTRACTION_SERV_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction.hpp"

#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <memory>
#include <istream>
#include <ios>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include <fcgiapp.h>
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
namespace serv {
namespace sal {

// clock abstraction (replacement for time.h)
class system_clock
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(system_clock)

protected:
    static system_clock *instance;

protected:
    system_clock() = default;

public:
    static system_clock &get_instance() noexcept { return *instance; }

    virtual ~system_clock() noexcept = default;
    virtual void gettime(clockid_t  clk_id,
                        timespec   *res) const noexcept = 0;
    virtual void sleep_for(time_t milliseconds) const noexcept = 0;
};

// libcurl abstraction (replacement for curl/curl.h)
class curl
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(curl)

protected:
    static curl *instance;

protected:
    curl() = default;

public:
    static curl &get_instance() noexcept { return *instance; }

    virtual ~curl() noexcept = default;

    class exception : public std::runtime_error
    {
    public:
        explicit exception(std::string const &message) noexcept;
        ~exception() noexcept override = default;
        virtual int get_curl_code() const = 0;
    };
    struct request_result
    {
        long        http_code;
        std::string content_type;
        std::string data;
    };
    virtual request_result post_data(std::string const url,
                                     std::string const query,
                                     std::string const content_type,
                                     std::string const content = "",
                                     std::map<std::string, std::string> const &additional_headers
                                                 = std::map<std::string, std::string>()
                                     ) = 0;

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

// WDx permission abstraction (replacement for grp.h)
class wdx_permission
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(wdx_permission)

protected:
    static wdx_permission *instance;

protected:
    wdx_permission() = default;

public:
    static wdx_permission &get_instance() noexcept { return *instance; }

    virtual ~wdx_permission() noexcept = default;
    virtual std::vector<std::string> get_user_groups(std::string const &user_name) const = 0;
};


} // Namespace sal
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


#endif // SRC_LIBWDXLINUXOSSERV_SYSTEM_ABSTRACTION_SERV_HPP_
//---- End of source file ------------------------------------------------------
