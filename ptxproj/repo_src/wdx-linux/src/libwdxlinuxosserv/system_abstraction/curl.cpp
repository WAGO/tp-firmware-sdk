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
///  \brief    System abstraction implementation for curl.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "system_abstraction_serv.hpp"

#include <wc/structuring.h>
#include <wc/assertion.h>
#include <wc/log.h>
#include <curl/curl.h>

#include <vector>
#include <cstring>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {
namespace sal {

class code_exception : public curl::exception
{
public:
    CURLcode const curl_code;

    explicit code_exception(CURLcode code) noexcept;
    int get_curl_code() const override;
};

class curl_impl : public curl
{
    WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(curl_impl);

public:
    curl_impl() noexcept;
    ~curl_impl() noexcept override;

    request_result post_data(std::string const url,
                             std::string const query,
                             std::string const content_type,
                             std::string const content = "",
                             std::map<std::string, std::string> const &additional_headers
                                         = std::map<std::string, std::string>()
                            ) override;
};

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static curl_impl default_curl;

curl *curl::instance = &default_curl;

constexpr size_t const max_result_data_size = 100 * 1024; // 100 KB

//------------------------------------------------------------------------------
// internal function implementation
//------------------------------------------------------------------------------
namespace {
// See also: https://curl.se/libcurl/c/CURLOPT_DEBUGFUNCTION.html
int curl_debug_log(CURL          const * const WC_UNUSED_PARAM(handle),
                   curl_infotype const         type,
                   char          const * const data,
                   size_t        const         size,
                   void          const * const WC_UNUSED_PARAM(clientp))
{
    std::string message;
    switch(type)
    {
#ifdef NDEBUG
        // get warned in debug builds if not all types are handled in this switch
        default:
            wc_log(log_level_t::warning, "Unknown curl type.");
            return -1;
#endif
        case CURLINFO_END: // end marker for enum should never occur
            return 0;

        case CURLINFO_TEXT:
            message = "CURL    Info: " + std::string(data, size);
            break;
        case CURLINFO_HEADER_OUT:
            message = "CURL => Send header: " + std::string(data, size);
            break;
        case CURLINFO_DATA_OUT:
            message = "CURL => Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            message = "CURL => Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            message = "CURL <= Receive header: " + std::string(data, size);
            break;
        case CURLINFO_DATA_IN:
            message = "CURL <= Receive data";
            break;
        case CURLINFO_SSL_DATA_IN:
            message = "CURL <= Receive SSL data";
            break;
    }
    wc_log(log_level_t::debug, message.c_str());

    return 0;
}

size_t curl_data_callback(char * const ptr,
                          size_t const size,
                          size_t const nmemb,
                          void * const userdata)
{
    WC_ASSERT(size == 1); // This should be always 1: https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    std::string  * const received_data = reinterpret_cast<std::string *>(userdata);
    char const   * const provided_data = ptr;
    size_t const         data_length   = nmemb;

    // Protect implementation not to consume big data responses
    size_t const result_size = received_data->length() + data_length;
    if((result_size < data_length) || (result_size > max_result_data_size))
    {
        return CURL_WRITEFUNC_ERROR;
    }

    // Read data chunk
    try
    {
        *received_data = *received_data + std::string(provided_data, data_length);
    }
    catch(...)
    {
        return CURL_WRITEFUNC_ERROR;
    }

    return data_length;
}

template<typename T>
void curl_throwing_setopt(CURL *curl, CURLoption option, T value)
{
    auto curl_code = curl_easy_setopt(curl, option, value);
    if(curl_code != CURLE_OK)
    {
        throw curl_code;
    }
}

void curl_throwing_perform(CURL *curl)
{
    auto curl_code = curl_easy_perform(curl);
    if(curl_code != CURLE_OK)
    {
        throw curl_code;
    }
}

template<typename T>
void curl_throwing_getinfo(CURL *curl, CURLINFO info, T value)
{
    auto curl_code = curl_easy_getinfo(curl, info, value);
    if(curl_code != CURLE_OK)
    {
        throw curl_code;
    }
}
}
//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
curl::exception::exception(std::string const &message) noexcept
:std::runtime_error(message)
{}

code_exception::code_exception(CURLcode code) noexcept
: curl::exception(curl_easy_strerror(code))
, curl_code(code)
{
    WC_ASSERT(curl_code >= 0);
}

int code_exception::get_curl_code() const
{
    return static_cast<int>(curl_code);
}


curl_impl::curl_impl() noexcept
{
    auto curl_code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(curl_code != CURLE_OK)
    {
        wc_log(log_level_t::fatal, ("Failed to initialize cURL library: " + std::to_string(curl_code)).c_str());
        exit(1);
    }
}

curl_impl::~curl_impl() noexcept
{
    curl_global_cleanup();
}

curl::request_result curl_impl::post_data(std::string const url,
                                          std::string const query,
                                          std::string const content_type,
                                          std::string const content,
                                          std::map<std::string, std::string> const &additional_headers)
{
    request_result  result;
    CURLcode        curl_code   = CURLE_OK;
    curl_slist     *header_list = nullptr;

    auto native_handle = curl_easy_init();
    try
    {
        if(native_handle == nullptr)
        {
            throw static_cast<int>(CURLE_FAILED_INIT);
        }

        // General options
        if(wc_get_log_level() == log_level_t::debug)
        {
            curl_throwing_setopt(native_handle, CURLOPT_DEBUGFUNCTION, curl_debug_log);
        }
        curl_throwing_setopt(native_handle, CURLOPT_NOPROGRESS, 1);

        // URL and method
        curl_throwing_setopt(native_handle, CURLOPT_URL, (url + (query.empty() ? "" : "?" + query)).c_str());
        curl_throwing_setopt(native_handle, CURLOPT_POST, 1);

        // Build headers
        header_list = curl_slist_append(header_list, ("Content-Type: " + content_type).c_str());
        for(auto it = additional_headers.begin(); it != additional_headers.end(); ++it)
        {
            std::string header = it->first + ": " + it->second;
            header_list = curl_slist_append(header_list, header.c_str());
        }
        curl_throwing_setopt(native_handle, CURLOPT_HTTPHEADER, header_list);

        // Add content pointer
        curl_throwing_setopt(native_handle, CURLOPT_POSTFIELDSIZE, content.length());
        curl_throwing_setopt(native_handle, CURLOPT_POSTFIELDS, content.c_str());

        // Set write function for returned data and fire request
        curl_throwing_setopt(native_handle, CURLOPT_WRITEDATA, &(result.data));
        curl_throwing_setopt(native_handle, CURLOPT_WRITEFUNCTION, curl_data_callback);
        WC_DEBUG_LOG("Perform POST request with libcurl");
        curl_throwing_perform(native_handle);

        // Extract return HTTP code and headers
        curl_throwing_getinfo(native_handle, CURLINFO_RESPONSE_CODE, &(result.http_code));
        char *response_content_type = nullptr;
        curl_throwing_getinfo(native_handle, CURLINFO_CONTENT_TYPE, &response_content_type);
        if(response_content_type != nullptr)
        {
            result.content_type = response_content_type;
        }
    }
    catch(CURLcode const &code)
    {
        curl_code = code;
    }

    // Cleanup
    if(header_list != nullptr)
    {
        curl_slist_free_all(header_list);
    }
    if(native_handle != nullptr)
    {
        curl_easy_cleanup(native_handle);
    }

    if(curl_code != CURLE_OK)
    {
        throw code_exception(curl_code);
    }

    return result;
}


} // Namespace sal
} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago


//---- End of source file ------------------------------------------------------
