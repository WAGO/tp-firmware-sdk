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
///  \brief    Device extension loader implementation for WAGO parameter service.
///
///  \author   MaHe: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "device_extension_loader.hpp"
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

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
static future<wdd_response> read_description_file(std::string const &file_path);

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
device_extension_loader::device_extension_loader(std::string const &extension_path)
: extension_path_m(extension_path)
{ }

device_extension_loader::~device_extension_loader() noexcept = default;

future<wdd_response> device_extension_loader::get_device_information()
{
    if(linuxos::sal::filesystem::get_instance().is_file_existing(extension_path_m))
    {
        return read_description_file(extension_path_m);
    }
    else
    {
        return resolved_future(wdd_response(wdx::status_codes::no_error_yet));
    }
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
} // Namespace wago

//---- End of source file ------------------------------------------------------
