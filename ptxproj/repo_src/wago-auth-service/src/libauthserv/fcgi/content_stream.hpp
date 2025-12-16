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
///  \brief    FCGI service request.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef SRC_LIBAUTHSERV_FCGI_CONTENT_STREAM_HPP_
#define SRC_LIBAUTHSERV_FCGI_CONTENT_STREAM_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include <wc/structuring.h>
#include <wc/assertion.h>

#include <iostream>
#include <vector>
#include <limits>

struct FCGX_Request;

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace authserv {
namespace fcgi {

class content_buffer : public std::streambuf
{
private:
    FCGX_Request      *request_m;
    std::vector<char>  current_m;
    std::streamsize    current_length_m;
public:

    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(content_buffer)

    explicit content_buffer(FCGX_Request *request, size_t buffer_size = 4096);
    ~content_buffer() noexcept override = default;
    std::streambuf::int_type underflow() override;
    std::streambuf::int_type overflow(std::streambuf::int_type WC_UNUSED_PARAM(banane)) override;
    int sync() override;
};

class content_stream : public std::istream
{
private:
    content_buffer buffer_m;
public:
    explicit content_stream(FCGX_Request *request)
    : std::istream(&buffer_m)
    , buffer_m(request)
    { }
    ~content_stream() noexcept override = default;
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------


} // Namespace fcgi
} // Namespace authserv
} // Namespace wago


#endif // SRC_LIBAUTHSERV_FCGI_CONTENT_STREAM_HPP_
//---- End of source file ------------------------------------------------------
