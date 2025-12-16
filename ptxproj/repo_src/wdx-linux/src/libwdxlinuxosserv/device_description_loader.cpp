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
///  \brief    Device description repository loader implementation for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "device_description_loader.hpp"
#include "system_abstraction_serv.hpp"

#include <wago/wdx/status_codes.hpp>
#include <wc/structuring.h>
#include <wc/log.h>

#include <ios>
#include <istream>
#include <exception>
#include <set>
#include <algorithm>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
namespace wago {
namespace wdx {
namespace linuxos {
namespace serv {

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
static constexpr char const wdd_extension[] = ".wdd.json";

static constexpr char const replacement     = '_';
static std::set<char> allowed_chars         = { 'a','b','c','d','e','f','g','h','i','j','k','l','m',  // parasoft-suppress CERT_CPP-ERR58-a-2 CERT_CPP-ERR50-f-3 "Set construction does not throw."
                                                'n','o','p','q','r','s','t','u','v','w','x','y','z',
                                                'A','B','C','D','E','F','G','H','I','J','K','L','M',
                                                'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
                                                '1','2','3','4','5','6','7','8','9','0','-','_' };

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static void made_string_file_path_safe(std::string &file_path_part);
static future<wdd_response> read_description_file(std::string const &file_path);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
device_description_loader::device_description_loader(std::string      const &repository_path,
                                                     device_selection const &selected_devices)
: repository_path_m(repository_path)
, selected_devices_m(selected_devices)
{ }

device_description_loader::~device_description_loader() noexcept = default;

device_selector_response device_description_loader::get_provided_devices()
{
    return device_selector_response(selected_devices_m);
}

future<wdd_response> device_description_loader::get_device_information(std::string order_number,
                                                                       std::string firmware_version)
{
    made_string_file_path_safe(order_number);
    made_string_file_path_safe(firmware_version);
    std::string base_file_path = repository_path_m + "/" + order_number +                          wdd_extension;
    std::string full_file_path = repository_path_m + "/" + order_number + "_" + firmware_version + wdd_extension;
    if(linuxos::sal::filesystem::get_instance().is_file_existing(full_file_path))
    {
        return read_description_file(full_file_path);
    }
    // Fallback order number only
    else if(linuxos::sal::filesystem::get_instance().is_file_existing(base_file_path))
    {
        return read_description_file(base_file_path);
    }
    else
    {
        return resolved_future(wdd_response(wdx::status_codes::no_error_yet));
    }
}

void made_string_file_path_safe(std::string &file_path_part)
{
    std::replace_if(file_path_part.begin(), file_path_part.end(), [](char c){
        return (allowed_chars.find(c) == allowed_chars.end());
    }, replacement);
}

future<wdd_response> read_description_file(std::string const &file_path)
{
    std::string error_message = "Failed to read device description from \"" + file_path + "\": ";
    WC_DEBUG_LOG(("Load device description from \"" + file_path + "\"").c_str());
    std::string description_content;
    try
    {
        auto model_stream = linuxos::sal::filesystem::get_instance().open_stream(file_path, std::ios::in);
        if(model_stream.get() == nullptr)
        {
            error_message.append("Failed to open file");
        }
        else
        {
            description_content.assign( (std::istreambuf_iterator<char>(*model_stream) ),
                                        (std::istreambuf_iterator<char>()              ) );
            if(!model_stream->good())
            {
                error_message.append("Failed to read file");
            }
            else
            {
                return resolved_future(wdd_response(description_content));
            }
        }
    }
    catch(std::exception &e)
    {
        error_message.append(e.what());
    }
    catch(...)
    {
        error_message.append("Unknown element thrown");
    }
    wc_log(error, error_message.c_str());
    return resolved_future(wdd_response(wdx::status_codes::internal_error));
}


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
}  // Namespace wago

//---- End of source file ------------------------------------------------------
