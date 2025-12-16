//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2019-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Model repository loader implementation for WAGO parameter service.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "model_loader.hpp"
#include "system_abstraction_serv.hpp"

#include <wago/wdx/status_codes.hpp>
#include <wc/structuring.h>
#include <wc/log.h>

#include <ios>
#include <istream>
#include <exception>

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

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
model_loader::model_loader(std::string const &model_path)
: model_path_m(model_path)
{ }

model_loader::~model_loader() noexcept = default;

future<wdm_response> model_loader::get_model_information()
{
    std::string error_message = "Failed to read model from \"" + model_path_m + "\": ";
    WC_DEBUG_LOG(("Load model description from \"" + model_path_m + "\"").c_str());
    std::string model_content;
    try
    {
        auto model_stream = linuxos::sal::filesystem::get_instance().open_stream(model_path_m, std::ios::in);
        if(model_stream.get() == nullptr)
        {
            error_message.append("Failed to open file");
        }
        else
        {
            model_content.assign( (std::istreambuf_iterator<char>(*model_stream) ),
                                  (std::istreambuf_iterator<char>()              ) );
            if(!model_stream->good())
            {
                error_message.append("Failed to read file");
            }
            else
            {
                return resolved_future(wdm_response(model_content));
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
    return resolved_future(wdm_response(wdx::status_codes::internal_error));
}


} // Namespace serv
} // Namespace linuxos
} // Namespace wdx
} // Namespace wago

//---- End of source file ------------------------------------------------------
