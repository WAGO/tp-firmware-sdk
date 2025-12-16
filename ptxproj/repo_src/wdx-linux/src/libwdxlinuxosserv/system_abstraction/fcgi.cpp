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
///  \brief    System abstraction implementation for FCGI.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_serv.hpp"

#include <fcgiapp.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace sal {

class fcgi_impl : public fcgi
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(fcgi_impl)

public:
    fcgi_impl() noexcept = default;
    ~fcgi_impl() noexcept override = default;

    int    Init() noexcept override;
    int    OpenSocket(char const *path,
                      int         backlog) noexcept override;
    void   ShutdownPending() noexcept override;
    int    InitRequest(FCGX_Request *request,
                       int           sock,
                       int           flags) noexcept override;
    int    Accept_r(FCGX_Request *request) noexcept override;
    char * GetParam(char const      *name,
                    FCGX_ParamArray  envp) noexcept override;
    char * GetLine(char        *str,
                   int          n,
                   FCGX_Stream *stream) noexcept override;
    int    GetStr(char        *str,
                  int          n,
                  FCGX_Stream *stream) noexcept override;
    int    PutS(char const  *str,
                FCGX_Stream *stream) noexcept override;
    int    PutStr(char const  *str,
                  int          n,
                  FCGX_Stream *stream) noexcept override;
    int    FFlush(FCGX_Stream *stream) noexcept override;
    void   Finish_r(FCGX_Request *request) noexcept override;
    void   Free(FCGX_Request *request,
                int           close) noexcept override;
    int    HasSeenEOF(FCGX_Stream *stream) noexcept override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static fcgi_impl default_fcgi;

fcgi *fcgi::instance = &default_fcgi;

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
int fcgi_impl::Init() noexcept
{
    return ::FCGX_Init();
}

int fcgi_impl::OpenSocket(char const *path,
                          int         backlog) noexcept
{
    return ::FCGX_OpenSocket(path, backlog);
}

void fcgi_impl::ShutdownPending() noexcept
{
    ::FCGX_ShutdownPending();
}

int fcgi_impl::InitRequest(FCGX_Request *request,
                           int           sock,
                           int           flags) noexcept
{
    return ::FCGX_InitRequest(request, sock, flags);
}

int fcgi_impl::Accept_r(FCGX_Request *request) noexcept
{
    return ::FCGX_Accept_r(request);
}

char * fcgi_impl::GetParam(char const      *name,
                           FCGX_ParamArray  envp) noexcept
{
    return ::FCGX_GetParam(name, envp);
}

char * fcgi_impl::GetLine(char        *str,
                          int          n,
                          FCGX_Stream *stream) noexcept
{
    return ::FCGX_GetLine(str, n, stream);
}

int fcgi_impl::GetStr(char        *str,
                      int          n,
                      FCGX_Stream *stream) noexcept
{
    return ::FCGX_GetStr(str, n, stream);
}

int fcgi_impl::PutS(char const  *str,
                    FCGX_Stream *stream) noexcept
{
    return ::FCGX_PutS(str, stream);
}

int fcgi_impl::PutStr(char const  *str,
                      int          n,
                      FCGX_Stream *stream) noexcept
{
    return ::FCGX_PutStr(str, n, stream);
}

int fcgi_impl::FFlush(FCGX_Stream *stream) noexcept
{
    return ::FCGX_FFlush(stream);
}

void fcgi_impl::Finish_r(FCGX_Request *request) noexcept
{
    ::FCGX_Finish_r(request);
}

void fcgi_impl::Free(FCGX_Request *request,
                     int           close) noexcept
{
    ::FCGX_Free(request, close);
}

int fcgi_impl::HasSeenEOF(FCGX_Stream *stream) noexcept
{
    return ::FCGX_HasSeenEOF(stream);
}


} // Namespace sal
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
